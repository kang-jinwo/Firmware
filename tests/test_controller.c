/* tests/test_controller.c — 제어기 단위 테스트 (내가 직접 쓴다)
 *
 * 실행: python run.py test
 * 설명: docs/M6_verification.md 3절
 */
#include <stdio.h>
#include "../firmware/controller.h"
#include "../firmware/hal.h"        /* PWM_DUTY_MAX 를 쓰기 위해 */

static int fail_count = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("FAIL  line %d: %s\n", __LINE__, #cond); \
            fail_count++; \
        } \
    } while (0)

/* 예시 하나: P 제어에서 오차가 100 rad/s 이고 kp = 0.002 이면
 *            듀티 = 0.002 × 100 = 0.2 → 정수로 200 */
static void test_p_example(void)
{
    Controller c;
    controller_init(&c, 0.002f, 0.0f, 0.001f);   /* kp, ki, 제어 주기[s] */
    CHECK(control_p(&c, 400, 300) == 200);       /* (제어기, 목표, 측정값) */
}

/* TODO 1: clamp_duty 의 경계값 — −1, 0, 500, 1000, 1001, 아주 큰 수 */

/* TODO 2: control_p 에서 오차가 0, 음수, 아주 클 때의 듀티 */

/* TODO 3: 어떤 (목표, 측정값) 조합에서도 듀티가 0 ~ PWM_DUTY_MAX 안에 있는지.
 *         for 문 두 개로 여러 조합을 넣어 본다. 네 가지 제어 함수 모두.
 *         (선택) 함수 포인터 배열을 쓰면 한 번에 돌릴 수 있다:
 *             ControlFn fns[4] = { control_p, control_pi, control_pi_int, control_onoff }; */

/* TODO 4: control_pi 에 같은 오차를 계속 넣으면 듀티가 커지는지 (적분이 쌓이는지).
 *         첫 주기의 값은 손으로 계산할 수 있다: kp·e + ki·e·dt */

/* TODO 5: 오차가 0이 되면 듀티가 변하지 않고 유지되는지 */

/* TODO 6 (M8 이후): control_pi_int 와 control_pi 가 거의 같은 값을 내는지 */

/* TODO 7 (M9 이후): control_onoff 의 히스테리시스 — 띠 안에서는 이전 상태를 유지하는지 */

int main(void)
{
    test_p_example();
    /* TODO: 위에서 만든 테스트 함수들을 여기서 부른다 */

    if (fail_count == 0) printf("PASS  test_controller (all checks)\n");
    else                 printf("test_controller: %d check(s) failed\n", fail_count);
    return fail_count == 0 ? 0 : 1;
}
