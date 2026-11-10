/* firmware/hal.h — 펌웨어와 바깥 세상의 경계 (HAL: Hardware Abstraction Layer)
 *
 * 펌웨어는 모터 내부(전류, 실제 속도)를 볼 수 없다. 오직 이 두 함수로만 바깥과 통한다.
 * 이 두 함수의 "몸통"은 펌웨어 안에 없다.
 *   - 시뮬레이션에서는 sim/main.c 가 구현한다 (가상 모터에 연결).
 *   - 실제 보드로 옮기면 타이머/ADC 레지스터를 만지는 코드로 이 두 함수만 다시 쓰면 된다.
 */
#ifndef HAL_H
#define HAL_H

#define PWM_DUTY_MAX 1000   /* 듀티 100%에 해당하는 정수 값. 듀티 0.37 → 370 */

int  sensor_read(void);     /* 속도 센서 값 [rad/s], 정수, 잡음 포함 */
void pwm_set(int duty);     /* PWM 듀티 설정. 0 ~ PWM_DUTY_MAX */

#endif
