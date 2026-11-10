/* plant/motor.c — DC 모터 식을 수치적으로 푼다 (오일러 방법)
 *
 *   전기:  L · di/dt = v − R·i − Ke·w
 *   기계:  J · dw/dt = Kt·i − b·w
 */
#include "motor.h"

void motor_default_params(MotorParams *p)
{
    p->Vs = 12.0;
    p->R  = 1.0;
    p->L  = 0.5e-3;
    p->Kt = 0.01;
    p->Ke = 0.01;
    p->J  = 1e-5;
    p->b  = 1e-6;
}

void motor_init(MotorState *m)
{
    m->i = 0.0;
    m->w = 0.0;
}

void motor_step(MotorState *m, const MotorParams *p, double v, double dt)
{
    /* 1) 지금 상태에서의 기울기(변화율)를 구한다 */
    double di_dt = (v - p->R * m->i - p->Ke * m->w) / p->L;
    double dw_dt = (p->Kt * m->i - p->b * m->w) / p->J;

    /* 2) 오일러 방법: 새 값 = 지금 값 + 기울기 × 시간 간격 */
    m->i += di_dt * dt;
    m->w += dw_dt * dt;
}
