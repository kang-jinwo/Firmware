/* firmware/controller.c — 제어 계산
 *
 * 모든 제어 함수는 같은 모양이다:  (제어기 상태, 목표, 측정값) → 듀티 정수
 * 듀티 정수는 0 ~ PWM_DUTY_MAX(1000). 즉 1000이 듀티 1.0이다.
 */
#include "controller.h"
#include "hal.h"

int clamp_duty(long duty)
{
    if (duty < 0) return 0;
    if (duty > PWM_DUTY_MAX) return PWM_DUTY_MAX;
    return (int)duty;
}

void controller_init(Controller *c, float kp, float ki, float dt)
{
    c->step = control_pi;
    c->kp = kp;
    c->ki = ki;
    c->dt = dt;
    c->integ = 0.0f;
    c->anti_windup = 0;

    c->hyst = 0;
    c->on = 0;

    controller_set_fixed_point(c, 8, 0);
}

/* 실수 이득을 정수(고정소수점)로 바꿔 둔다.
 * 이 계산은 시작할 때 한 번만 한다. 매 주기 도는 control_pi_int 안에는 실수 연산이 없다. */
void controller_set_fixed_point(Controller *c, int q_shift, int acc16)
{
    float scale = (float)PWM_DUTY_MAX * (float)(1L << q_shift);

    c->q_shift = q_shift;
    c->kp_q = (int32_t)(c->kp * scale + 0.5f);
    c->ki_q = (int32_t)(c->ki * c->dt * scale + 0.5f);
    c->integ_q = 0;
    c->integ_q16 = 0;
    c->acc16 = acc16;
}

/* ---------- 1판: 비례(P) 제어, 실수 ---------- */
int control_p(Controller *c, int target, int measured)
{
    float e = (float)(target - measured);   /* 오차 [rad/s] */
    float u = c->kp * e;                    /* 듀티 (0~1이어야 하지만 넘을 수 있다) */

    return clamp_duty((long)(u * PWM_DUTY_MAX + 0.5f));
}

/* ---------- 2판: 비례 + 적분(PI) 제어, 실수 ---------- */
int control_pi(Controller *c, int target, int measured)
{
    float e = (float)(target - measured);
    float add = c->ki * e * c->dt;          /* 이번 주기에 적분에 더할 양 */
    float u;

    c->integ += add;
    u = c->kp * e + c->integ;

    if (c->anti_windup) {
        /* 듀티가 이미 한계인데 적분이 그 방향으로 더 쌓이려 하면, 방금 더한 것을 되돌린다 */
        if ((u > 1.0f && add > 0.0f) || (u < 0.0f && add < 0.0f)) {
            c->integ -= add;
            u = c->kp * e + c->integ;
        }
    }

    return clamp_duty((long)(u * PWM_DUTY_MAX + 0.5f));
}

/* ---------- 3판: PI 제어, 정수(고정소수점) ----------
 *
 * 실수 0.05를 정수로는 그대로 담을 수 없다. 그래서 2^q_shift 를 곱해서 정수로 저장한다.
 *   q_shift = 8 이면  0.05 × 256 = 12.8 → 13 으로 저장 (오차가 생긴다)
 * 계산이 끝난 뒤 오른쪽으로 q_shift 비트 밀어(>>) 다시 2^q_shift 로 나눈다.
 */
int control_pi_int(Controller *c, int target, int measured)
{
    int32_t e = (int32_t)(target - measured);
    int32_t integ;
    int32_t u_q;

    if (c->acc16) {
        /* 16비트 적분기: −32768 ~ 32767 을 넘으면 값이 반대편으로 넘어간다 (오버플로) */
        c->integ_q16 = (int16_t)(c->integ_q16 + c->ki_q * e);
        integ = c->integ_q16;
    } else {
        c->integ_q += c->ki_q * e;
        integ = c->integ_q;
    }

    u_q = c->kp_q * e + integ;

    if (c->anti_windup && !c->acc16) {
        int32_t limit = (int32_t)PWM_DUTY_MAX << c->q_shift;
        if ((u_q > limit && e > 0) || (u_q < 0 && e < 0)) {
            c->integ_q -= c->ki_q * e;
            u_q = c->kp_q * e + c->integ_q;
        }
    }

    /* >> 는 음수에서 "내림"으로 동작한다 (gcc 기준). 어차피 음수는 0으로 잘린다. */
    return clamp_duty((long)(u_q >> c->q_shift));
}

/* ---------- 켜고 끄기 제어 + 히스테리시스 ----------
 *
 *   측정값 < 목표 − hyst  → 켠다
 *   측정값 > 목표 + hyst  → 끈다
 *   그 사이               → 이전 상태를 그대로 둔다  ← 이것이 히스테리시스 (슈미트 트리거와 같은 원리)
 */
int control_onoff(Controller *c, int target, int measured)
{
    if (measured < target - c->hyst) {
        c->on = 1;
    } else if (measured > target + c->hyst) {
        c->on = 0;
    }
    return c->on ? PWM_DUTY_MAX : 0;
}
