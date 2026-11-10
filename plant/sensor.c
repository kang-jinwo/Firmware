/* plant/sensor.c */
#include <math.h>
#include "sensor.h"

void sensor_init(Sensor *s, double noise_std, unsigned int seed)
{
    s->noise_std = noise_std;
    s->rng = seed;
}

/* 0 이상 1 미만의 난수 하나. (선형 합동 생성기: 간단한 곱셈·덧셈으로 만드는 가짜 난수)
 * rand()를 쓰지 않는 이유: 컴퓨터마다 결과가 달라서 실험을 똑같이 되풀이할 수 없다. */
static double uniform01(unsigned int *state)
{
    *state = *state * 1664525u + 1013904223u;
    return (double)(*state >> 8) / 16777216.0;   /* 위쪽 24비트만 사용 */
}

int sensor_measure(Sensor *s, double w_true)
{
    double noise = 0.0;

    if (s->noise_std > 0.0) {
        /* 균일 난수 12개를 더하고 6을 빼면 평균 0, 표준편차 1인 종 모양 분포에 가깝다
         * (중심극한정리). */
        double sum = 0.0;
        int k;
        for (k = 0; k < 12; k++) sum += uniform01(&s->rng);
        noise = (sum - 6.0) * s->noise_std;
    }

    /* 실제 센서(ADC, 엔코더 카운터)는 정수를 준다 → 가장 가까운 정수로 반올림 */
    return (int)floor(w_true + noise + 0.5);
}
