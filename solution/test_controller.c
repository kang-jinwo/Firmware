/* solution/test_controller.c (정답) — 제어기 단위 테스트
 *
 * 컴파일: gcc -o build/test_controller solution/test_controller.c firmware/filter.c firmware/controller.c
 */
#include <stdio.h>
#include "../firmware/controller.h"
#include "../firmware/hal.h"

static int fail_count = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("FAIL  line %d: %s\n", __LINE__, #cond); \
            fail_count++; \
        } \
    } while (0)

static void test_clamp(void)
{
    CHECK(clamp_duty(-1) == 0);
    CHECK(clamp_duty(0) == 0);
    CHECK(clamp_duty(500) == 500);
    CHECK(clamp_duty(PWM_DUTY_MAX) == PWM_DUTY_MAX);
    CHECK(clamp_duty(PWM_DUTY_MAX + 1) == PWM_DUTY_MAX);
    CHECK(clamp_duty(1000000L) == PWM_DUTY_MAX);
}

/* P 제어: 듀티 = kp × 오차. 손으로 계산한 값과 비교 */
static void test_p_known_values(void)
{
    Controller c;
    controller_init(&c, 0.002f, 0.0f, 0.001f);
    CHECK(control_p(&c, 400, 400) == 0);      /* 오차 0 → 듀티 0 */
    CHECK(control_p(&c, 400, 300) == 200);    /* 0.002 × 100 = 0.2 → 200 */
    CHECK(control_p(&c, 400, 0) == 800);      /* 0.002 × 400 = 0.8 → 800 */
    CHECK(control_p(&c, 400, 500) == 0);      /* 음수 → 0으로 잘림 */
    CHECK(control_p(&c, 2000, 0) == PWM_DUTY_MAX);  /* 4.0 → 1000으로 잘림 */
}

/* 어떤 입력이 와도 듀티는 0 ~ PWM_DUTY_MAX 안에 있어야 한다 (모든 제어 방식) */
static void test_duty_always_in_range(void)
{
    ControlFn fns[4] = { control_p, control_pi, control_pi_int, control_onoff };
    int f, target, meas;

    for (f = 0; f < 4; f++) {
        Controller c;
        controller_init(&c, 0.5f, 5.0f, 0.001f);      /* 일부러 큰 이득 */
        controller_set_fixed_point(&c, 8, 0);
        for (target = 0; target <= 1500; target += 500) {
            for (meas = -200; meas <= 1500; meas += 50) {
                int d = fns[f](&c, target, meas);
                CHECK(d >= 0 && d <= PWM_DUTY_MAX);
            }
        }
    }
}

/* PI: 오차가 일정하면 적분 때문에 듀티가 매 주기 커져야 한다 */
static void test_pi_integrates(void)
{
    Controller c;
    int d1, d2 = 0, k;
    controller_init(&c, 0.002f, 0.02f, 0.001f);
    d1 = control_pi(&c, 400, 300);
    for (k = 0; k < 999; k++) d2 = control_pi(&c, 400, 300);
    /* 1000주기(1초) 뒤: kp·e + ki·e·1s = 0.2 + 0.02×100×1 = 2.2 → 한계에 걸림 */
    CHECK(d1 == 202);                 /* 첫 주기: kp·e + ki·e·dt = 0.2 + 0.002 = 0.202 → 202 */
    CHECK(d2 == PWM_DUTY_MAX);
}

/* PI: 오차가 0이면 적분이 변하지 않아 듀티가 유지된다 */
static void test_pi_holds_when_error_zero(void)
{
    Controller c;
    int k, d_before, d_after = 0;
    controller_init(&c, 0.002f, 0.02f, 0.001f);
    for (k = 0; k < 500; k++) control_pi(&c, 400, 0);     /* 적분을 쌓는다 */
    d_before = control_pi(&c, 400, 400);
    for (k = 0; k < 100; k++) d_after = control_pi(&c, 400, 400);
    CHECK(d_before > 0);
    CHECK(d_before == d_after);
}

/* 정수 PI와 실수 PI는 거의 같은 값을 내야 한다 (q_shift = 12, 차이 2 이하) */
static void test_int_matches_float(void)
{
    Controller cf, ci;
    int k;
    controller_init(&cf, 0.002f, 0.02f, 0.001f);
    controller_init(&ci, 0.002f, 0.02f, 0.001f);
    controller_set_fixed_point(&ci, 12, 0);
    for (k = 0; k < 300; k++) {
        int meas = k;                       /* 측정값이 천천히 올라가는 상황 */
        int a = control_pi(&cf, 400, meas);
        int b = control_pi_int(&ci, 400, meas);
        CHECK(a - b <= 2 && b - a <= 2);
    }
}

/* q_shift 가 너무 작으면 적분 이득이 0으로 반올림된다 (P5 문제를 테스트로 고정) */
static void test_small_qshift_kills_integral(void)
{
    Controller c;
    controller_init(&c, 0.002f, 0.02f, 0.001f);
    controller_set_fixed_point(&c, 4, 0);
    CHECK(c.ki_q == 0);
    controller_set_fixed_point(&c, 8, 0);
    CHECK(c.ki_q == 5);                     /* 0.02 × 0.001 × 1000 × 256 = 5.12 → 5 */
    CHECK(c.kp_q == 512);                   /* 0.002 × 1000 × 256 = 512 */
}

/* anti-windup: 포화 상태로 오래 있어도 적분이 한없이 쌓이지 않는다 */
static void test_anti_windup_limits_integral(void)
{
    Controller a, b;
    int k;
    controller_init(&a, 0.002f, 0.02f, 0.001f);
    controller_init(&b, 0.002f, 0.02f, 0.001f);
    b.anti_windup = 1;
    for (k = 0; k < 2000; k++) {
        control_pi(&a, 1500, 1188);
        control_pi(&b, 1500, 1188);
    }
    CHECK(a.integ > 5.0f);                  /* 꺼 두면 크게 쌓인다 */
    CHECK(b.integ <= 1.0f);                 /* 켜 두면 듀티 한계 안쪽에 머문다 */
}

/* 켜고 끄기 + 히스테리시스: 띠 안에서는 이전 상태를 유지한다 */
static void test_onoff_hysteresis(void)
{
    Controller c;
    controller_init(&c, 0.0f, 0.0f, 0.001f);
    c.hyst = 10;
    CHECK(control_onoff(&c, 400, 380) == PWM_DUTY_MAX);   /* 390 미만 → 켬 */
    CHECK(control_onoff(&c, 400, 395) == PWM_DUTY_MAX);   /* 띠 안 → 유지(켬) */
    CHECK(control_onoff(&c, 400, 405) == PWM_DUTY_MAX);   /* 띠 안 → 유지(켬) */
    CHECK(control_onoff(&c, 400, 411) == 0);              /* 410 초과 → 끔 */
    CHECK(control_onoff(&c, 400, 405) == 0);              /* 띠 안 → 유지(끔) */
    CHECK(control_onoff(&c, 400, 395) == 0);              /* 띠 안 → 유지(끔) */
    CHECK(control_onoff(&c, 400, 389) == PWM_DUTY_MAX);   /* 다시 켬 */
}

int main(void)
{
    test_clamp();
    test_p_known_values();
    test_duty_always_in_range();
    test_pi_integrates();
    test_pi_holds_when_error_zero();
    test_int_matches_float();
    test_small_qshift_kills_integral();
    test_anti_windup_limits_integral();
    test_onoff_hysteresis();

    if (fail_count == 0) printf("PASS  test_controller (all checks)\n");
    else                 printf("test_controller: %d check(s) failed\n", fail_count);
    return fail_count == 0 ? 0 : 1;
}
