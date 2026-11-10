/* plant/pwm.h — PWM 출력단 모델
 *
 * PWM 한 주기를 period_steps 칸(시뮬레이션 걸음)으로 나눈다.
 * 듀티가 d이면 앞쪽 d 비율 동안 Vs, 나머지는 0 V가 나간다.
 */
#ifndef PWM_H
#define PWM_H

typedef struct {
    int    period_steps;  /* PWM 한 주기가 시뮬레이션 몇 걸음인지 (50 µs / 1 µs = 50) */
    double duty;          /* 0.0 ~ 1.0 */
} Pwm;

void pwm_init(Pwm *p, int period_steps);

/* 시뮬레이션 n번째 걸음 동안의 출력 전압(그 걸음 안에서의 평균)을 돌려준다. */
double pwm_voltage(const Pwm *p, double Vs, long n);

#endif
