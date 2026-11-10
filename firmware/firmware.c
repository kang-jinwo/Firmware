/* firmware/firmware.c — 제어 루프 한 바퀴
 *
 *   센서 읽기 → 필터 → 제어 계산 → PWM 출력
 *
 * 이 파일은 plant/ 의 어떤 것도 include하지 않는다. hal.h 의 두 함수만 쓴다.
 */
#include "firmware.h"
#include "controller.h"
#include "filter.h"
#include "hal.h"

/* static 전역 변수: 이 파일 안에서만 보이고, 함수가 끝나도 값이 남는다.
 * 펌웨어는 "지난 주기의 값"(적분, 필터 버퍼)을 기억해야 해서 이런 상태가 필요하다. */
static Controller ctrl;
static MovingAvg  filt;
static int        target_speed;
static int        last_filtered;

void fw_init(const FwConfig *cfg)
{
    controller_init(&ctrl, cfg->kp, cfg->ki, cfg->period_s);
    controller_set_fixed_point(&ctrl, cfg->q_shift, cfg->acc16);
    ctrl.anti_windup = cfg->anti_windup;
    ctrl.hyst = cfg->hyst;

    /* 제어 방식 고르기: 함수 포인터에 함수 "이름"을 넣는다 (이름이 곧 주소) */
    switch (cfg->mode) {
    case FW_MODE_P:      ctrl.step = control_p;      break;
    case FW_MODE_PI:     ctrl.step = control_pi;     break;
    case FW_MODE_PI_INT: ctrl.step = control_pi_int; break;
    case FW_MODE_ONOFF:  ctrl.step = control_onoff;  break;
    }

    ma_init(&filt, cfg->ma_len);
    target_speed = 0;
    last_filtered = 0;
}

void fw_set_target(int target)
{
    target_speed = target;
}

void fw_tick(void)
{
    int raw  = sensor_read();                          /* 1. 센서 읽기 */
    int meas = ma_update(&filt, raw);                  /* 2. 잡음 거르기 */
    int duty = ctrl.step(&ctrl, target_speed, meas);   /* 3. 제어 계산 (함수 포인터로 호출) */
    pwm_set(duty);                                     /* 4. 출력 */

    last_filtered = meas;
}

int fw_debug_filtered(void)
{
    return last_filtered;
}
