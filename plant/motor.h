/* plant/motor.h — 가상의 DC 모터 모델
 *
 * "plant"는 제어 대상(여기서는 모터)을 부르는 말이다.
 * 이 파일은 "실제 세상" 역할이다. 펌웨어는 이 파일을 include하면 안 된다.
 */
#ifndef MOTOR_H
#define MOTOR_H

/* 모터의 고정된 성질 (사양표의 값) */
typedef struct {
    double Vs;  /* 전원 전압 [V] */
    double R;   /* 권선 저항 [ohm] */
    double L;   /* 권선 인덕턴스 [H] */
    double Kt;  /* 토크 상수 [N·m/A] */
    double Ke;  /* 역기전력 상수 [V·s/rad] */
    double J;   /* 회전 관성 [kg·m^2] */
    double b;   /* 마찰 계수 [N·m·s/rad] */
} MotorParams;

/* 시간에 따라 변하는 값 (상태) */
typedef struct {
    double i;   /* 전류 [A] */
    double w;   /* 각속도 [rad/s] */
} MotorState;

void motor_default_params(MotorParams *p);
void motor_init(MotorState *m);

/* 전압 v를 dt초 동안 걸었을 때 상태 m을 한 걸음 전진시킨다.
 * m은 포인터로 받는다: 함수 안에서 바꾼 값이 호출한 쪽에도 남아야 하기 때문. */
void motor_step(MotorState *m, const MotorParams *p, double v, double dt);

#endif
