/* firmware/controller.h — 제어기 (P, PI, 정수 PI, 켜고 끄기) */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

/* 구조체 이름을 먼저 알려 둔다 (아래 함수 포인터 선언에서 쓰기 위해) */
typedef struct Controller Controller;

/* 함수 포인터 자료형.
 * "Controller 포인터, 목표, 측정값을 받아 듀티(0~PWM_DUTY_MAX)를 돌려주는 함수"를 가리킨다.
 * 제어 방식을 바꿔 끼울 때 이 포인터가 가리키는 함수만 바꾸면 된다. */
typedef int (*ControlFn)(Controller *c, int target, int measured);

struct Controller {
    ControlFn step;      /* 지금 쓰는 제어 방식 */

    /* --- 실수(float) 제어기용 --- */
    float kp;            /* 비례 이득 [듀티 / (rad/s)] */
    float ki;            /* 적분 이득 [듀티 / (rad/s · s)] */
    float dt;            /* 제어 주기 [s] */
    float integ;         /* 적분 항 (듀티 단위로 쌓인다) */
    int   anti_windup;   /* 1이면 포화 중 적분을 멈춘다 */

    /* --- 정수(고정소수점) 제어기용 --- */
    int     q_shift;     /* 소수부 비트 수. 실제 값 = 저장 값 / 2^q_shift */
    int32_t kp_q;        /* kp × PWM_DUTY_MAX × 2^q_shift 를 반올림한 정수 */
    int32_t ki_q;        /* ki × dt × PWM_DUTY_MAX × 2^q_shift 를 반올림한 정수 */
    int32_t integ_q;     /* 적분 항 (32비트) */
    int16_t integ_q16;   /* 적분 항 (16비트, 오버플로 실험용) */
    int     acc16;       /* 1이면 16비트 적분기를 쓴다 */

    /* --- 켜고 끄기 제어용 --- */
    int hyst;            /* 히스테리시스 폭 [rad/s] */
    int on;              /* 지금 켜져 있는지 (기억해야 하는 상태) */
};

void controller_init(Controller *c, float kp, float ki, float dt);
void controller_set_fixed_point(Controller *c, int q_shift, int acc16);

int control_p(Controller *c, int target, int measured);
int control_pi(Controller *c, int target, int measured);
int control_pi_int(Controller *c, int target, int measured);
int control_onoff(Controller *c, int target, int measured);

/* 듀티를 0 ~ PWM_DUTY_MAX 범위로 자른다 */
int clamp_duty(long duty);

#endif
