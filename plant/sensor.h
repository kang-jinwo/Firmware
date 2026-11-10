/* plant/sensor.h — 속도 센서 모델 (잡음 + 정수 반올림) */
#ifndef SENSOR_H
#define SENSOR_H

typedef struct {
    double       noise_std;  /* 잡음의 표준편차 [rad/s]. 0이면 잡음 없음 */
    unsigned int rng;        /* 난수 발생기 상태 (같은 seed면 항상 같은 잡음) */
} Sensor;

void sensor_init(Sensor *s, double noise_std, unsigned int seed);

/* 실제 속도 w_true를 "측정"한 값을 정수 [rad/s]로 돌려준다. */
int sensor_measure(Sensor *s, double w_true);

#endif
