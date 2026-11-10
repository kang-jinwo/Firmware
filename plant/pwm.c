/* plant/pwm.c */
#include "pwm.h"

void pwm_init(Pwm *p, int period_steps)
{
    p->period_steps = period_steps;
    p->duty = 0.0;
}

double pwm_voltage(const Pwm *p, double Vs, long n)
{
    int    k        = (int)(n % p->period_steps);        /* 이번 걸음이 주기 안에서 몇 번째 칸인지 */
    double on_steps = p->duty * p->period_steps;         /* 켜져 있는 칸 수 (소수 가능: 예 25.3칸) */

    /* k번째 칸 [k, k+1) 중에서 켜진 구간 [0, on_steps)와 겹치는 비율.
     *   완전히 켜진 칸 → 1,  완전히 꺼진 칸 → 0,  경계에 걸친 칸 → 0~1 사이.
     * 경계 칸을 이렇게 처리하지 않으면 듀티를 1/50 = 2% 단위로밖에 표현하지 못한다. */
    double frac = on_steps - k;
    if (frac > 1.0) frac = 1.0;
    if (frac < 0.0) frac = 0.0;

    return Vs * frac;
}
