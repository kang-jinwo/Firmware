/* sim/main.c — 가상 모터(plant)와 펌웨어(firmware)를 묶어서 돌리고 CSV로 기록한다.
 *
 * 이 파일이 하는 일은 세 가지다.
 *   1. hal.h 의 sensor_read(), pwm_set() 을 "가상 모터에 연결되도록" 구현한다.
 *   2. 시간을 1 µs씩 전진시키면서 모터 식을 풀고, 제어 주기마다 fw_tick()을 부른다.
 *   3. 결과를 CSV 파일로 쓴다.
 *
 * 사용 예:
 *   sim --mode open --volt 12 --tend 0.6 --out out/m2.csv
 *   sim --mode pi --kp 0.002 --ki 0.02 --target 400 --out out/m5.csv
 * 옵션 전체는 README.md 의 "시뮬레이터 옵션" 표를 본다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../plant/motor.h"
#include "../plant/pwm.h"
#include "../plant/sensor.h"
#include "../firmware/firmware.h"
#include "../firmware/hal.h"

#define SIM_DT    1e-6   /* 시뮬레이션 간격 1 µs */
#define PWM_STEPS 50     /* PWM 주기 50 µs = 50걸음 (20 kHz) */

/* ---- "세상"의 상태 (시뮬레이터만 안다. 펌웨어는 모른다) ---- */
static MotorParams params;
static MotorState  motor;
static Pwm         pwm;
static Sensor      sensor;

static int last_sensor_value;   /* 마지막으로 센서가 준 값 (기록용) */
static int last_duty_request;   /* 펌웨어가 마지막으로 요청한 듀티 (기록용, 자르기 전 값) */

/* 루프 실행 시간 모형 (P8): 계산 결과가 exec_steps 걸음 뒤에야 PWM에 반영된다 */
static long exec_steps;
static long now_step;
static long pending_at = -1;    /* 대기 중인 듀티가 반영될 시각 (-1 = 없음) */
static int  pending_duty;

/* ================= HAL 구현: 펌웨어가 부르는 두 함수 ================= */

int sensor_read(void)
{
    last_sensor_value = sensor_measure(&sensor, motor.w);
    return last_sensor_value;
}

static void apply_duty(int duty)
{
    /* 하드웨어는 0~100% 밖의 듀티를 낼 수 없다 */
    if (duty < 0) duty = 0;
    if (duty > PWM_DUTY_MAX) duty = PWM_DUTY_MAX;
    pwm.duty = (double)duty / PWM_DUTY_MAX;
}

void pwm_set(int duty)
{
    last_duty_request = duty;          /* 펌웨어가 요청한 값을 그대로 기록 (검증에 쓴다) */

    if (exec_steps <= 0) {
        apply_duty(duty);              /* 계산 시간이 0이라고 볼 때: 즉시 반영 */
    } else {
        pending_duty = duty;           /* 계산이 끝나는 시각에 반영되도록 예약 */
        pending_at = now_step + exec_steps;
    }
}

/* ================= 명령줄 옵션 ================= */

typedef struct {
    const char *mode;      /* open | pwm | p | pi | pi_int | onoff */
    double volt;           /* open 모드: 걸어 줄 전압 */
    double duty;           /* pwm 모드: 고정 듀티 */
    double kp, ki;
    int    target;         /* 목표 속도 */
    int    target2;        /* t2초부터의 목표 속도 (-1 = 쓰지 않음) */
    double t2;
    double noise;          /* 센서 잡음 표준편차 */
    int    ma;             /* 이동평균 길이 */
    double period_ms;      /* 제어 주기 */
    double tend;           /* 시뮬레이션 길이 [s] */
    long   log_us;         /* 기록 간격 [µs] */
    int    hyst;
    int    aw;             /* anti-windup */
    int    qshift;
    int    acc16;
    long   exec_us;        /* 루프 실행 시간 [µs] */
    unsigned int seed;
    const char *out;
} Options;

static void set_defaults(Options *o)
{
    o->mode = "pi";
    o->volt = 12.0;
    o->duty = 0.5;
    o->kp = 0.002;
    o->ki = 0.02;
    o->target = 400;
    o->target2 = -1;
    o->t2 = 0.0;
    o->noise = 0.0;
    o->ma = 1;
    o->period_ms = 1.0;
    o->tend = 0.6;
    o->log_us = 100;
    o->hyst = 0;
    o->aw = 0;
    o->qshift = 8;
    o->acc16 = 0;
    o->exec_us = 0;
    o->seed = 1;
    o->out = "out/run.csv";
}

/* argv 는 "문자열들의 배열"이다. 문자열이 char* 이므로 argv 의 자료형은 char** (이중 포인터).
 *   argv[0] = "sim", argv[1] = "--mode", argv[2] = "pi", ...
 * 옵션은 항상 "--이름 값" 두 칸씩 온다고 약속한다. */
static int parse_args(int argc, char **argv, Options *o)
{
    int k;
    for (k = 1; k + 1 < argc; k += 2) {
        const char *key = argv[k];
        const char *val = argv[k + 1];

        if      (strcmp(key, "--mode") == 0)      o->mode = val;
        else if (strcmp(key, "--volt") == 0)      o->volt = atof(val);
        else if (strcmp(key, "--duty") == 0)      o->duty = atof(val);
        else if (strcmp(key, "--kp") == 0)        o->kp = atof(val);
        else if (strcmp(key, "--ki") == 0)        o->ki = atof(val);
        else if (strcmp(key, "--target") == 0)    o->target = atoi(val);
        else if (strcmp(key, "--target2") == 0)   o->target2 = atoi(val);
        else if (strcmp(key, "--t2") == 0)        o->t2 = atof(val);
        else if (strcmp(key, "--noise") == 0)     o->noise = atof(val);
        else if (strcmp(key, "--ma") == 0)        o->ma = atoi(val);
        else if (strcmp(key, "--period_ms") == 0) o->period_ms = atof(val);
        else if (strcmp(key, "--tend") == 0)      o->tend = atof(val);
        else if (strcmp(key, "--log_us") == 0)    o->log_us = atol(val);
        else if (strcmp(key, "--hyst") == 0)      o->hyst = atoi(val);
        else if (strcmp(key, "--aw") == 0)        o->aw = atoi(val);
        else if (strcmp(key, "--qshift") == 0)    o->qshift = atoi(val);
        else if (strcmp(key, "--acc16") == 0)     o->acc16 = atoi(val);
        else if (strcmp(key, "--exec_us") == 0)   o->exec_us = atol(val);
        else if (strcmp(key, "--seed") == 0)      o->seed = (unsigned int)atol(val);
        else if (strcmp(key, "--out") == 0)       o->out = val;
        else {
            fprintf(stderr, "unknown option: %s\n", key);
            return -1;
        }
    }
    if (k < argc) {
        fprintf(stderr, "option without value: %s\n", argv[k]);
        return -1;
    }
    return 0;
}

/* ================= 메인 ================= */

int main(int argc, char **argv)
{
    Options  o;
    FwConfig cfg;
    FILE    *fp;
    long     n, n_end, ctrl_steps, busy_until = 0;
    long     missed = 0, ticks = 0, switches = 0;
    int      closed_loop;       /* 펌웨어가 도는 모드인가 */
    int      prev_duty = -1;

    set_defaults(&o);
    if (parse_args(argc, argv, &o) != 0) return 1;

    closed_loop = !(strcmp(o.mode, "open") == 0 || strcmp(o.mode, "pwm") == 0);

    /* ---- 세상 준비 ---- */
    motor_default_params(&params);
    motor_init(&motor);
    pwm_init(&pwm, PWM_STEPS);
    sensor_init(&sensor, o.noise, o.seed);

    ctrl_steps = (long)(o.period_ms * 1000.0 + 0.5);   /* 제어 주기가 몇 걸음인지 (1 ms = 1000걸음) */
    exec_steps = o.exec_us;
    n_end = (long)(o.tend / SIM_DT + 0.5);

    /* ---- 펌웨어 준비 ---- */
    if (closed_loop) {
        if      (strcmp(o.mode, "p") == 0)      cfg.mode = FW_MODE_P;
        else if (strcmp(o.mode, "pi") == 0)     cfg.mode = FW_MODE_PI;
        else if (strcmp(o.mode, "pi_int") == 0) cfg.mode = FW_MODE_PI_INT;
        else if (strcmp(o.mode, "onoff") == 0)  cfg.mode = FW_MODE_ONOFF;
        else {
            fprintf(stderr, "unknown mode: %s\n", o.mode);
            return 1;
        }
        cfg.kp = (float)o.kp;
        cfg.ki = (float)o.ki;
        cfg.period_s = (float)(o.period_ms / 1000.0);
        cfg.ma_len = o.ma;
        cfg.hyst = o.hyst;
        cfg.anti_windup = o.aw;
        cfg.q_shift = o.qshift;
        cfg.acc16 = o.acc16;

        fw_init(&cfg);
        fw_set_target(o.target);
    } else if (strcmp(o.mode, "pwm") == 0) {
        pwm.duty = o.duty;
    }

    fp = fopen(o.out, "w");
    if (fp == NULL) {
        fprintf(stderr, "cannot open %s (does the folder exist?)\n", o.out);
        return 1;
    }
    fprintf(fp, "t,v,i,w,w_meas,w_filt,duty,target\n");

    /* ---- 시간 전진 ---- */
    for (n = 0; n < n_end; n++) {
        double v;
        int    target_now = o.target;

        now_step = n;
        if (o.target2 >= 0 && n * SIM_DT >= o.t2) target_now = o.target2;

        /* (a) 예약된 듀티가 반영될 시각이면 반영 */
        if (pending_at >= 0 && n >= pending_at) {
            apply_duty(pending_duty);
            pending_at = -1;
        }

        /* (b) 제어 주기마다: 펌웨어 한 바퀴 */
        if (n % ctrl_steps == 0) {
            if (closed_loop) {
                if (n < busy_until) {
                    missed++;                       /* 이전 계산이 아직 안 끝났다 → 이번 주기를 놓침 */
                } else {
                    fw_set_target(target_now);
                    fw_tick();
                    ticks++;
                    busy_until = n + exec_steps;
                    if (prev_duty >= 0 && last_duty_request != prev_duty) switches++;
                    prev_duty = last_duty_request;
                }
            } else {
                sensor_read();                      /* 제어는 없지만 센서 값은 기록해 둔다 */
            }
        }

        /* (c) 모터에 걸리는 전압 */
        if (strcmp(o.mode, "open") == 0) v = o.volt;
        else                             v = pwm_voltage(&pwm, params.Vs, n);

        /* (d) 기록 (모터를 전진시키기 전, 시각 t의 값) */
        if (n % o.log_us == 0) {
            double duty_log = closed_loop ? (double)last_duty_request / PWM_DUTY_MAX : pwm.duty;
            fprintf(fp, "%.6f,%.4f,%.5f,%.4f,%d,%d,%.4f,%d\n",
                    n * SIM_DT, v, motor.i, motor.w,
                    last_sensor_value,
                    closed_loop ? fw_debug_filtered() : last_sensor_value,
                    duty_log,
                    closed_loop ? target_now : 0);
        }

        /* (e) 모터를 1 µs 전진 */
        motor_step(&motor, &params, v, SIM_DT);
    }

    fclose(fp);

    printf("mode=%s  final w=%.2f rad/s  i=%.4f A  ticks=%ld  missed=%ld  duty_changes=%ld  -> %s\n",
           o.mode, motor.w, motor.i, ticks, missed, switches, o.out);
    return 0;
}
