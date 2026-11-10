/* firmware/firmware.h — 펌웨어의 바깥쪽 얼굴 */
#ifndef FIRMWARE_H
#define FIRMWARE_H

typedef enum {
    FW_MODE_P,        /* 비례 제어 (float) */
    FW_MODE_PI,       /* PI 제어 (float) */
    FW_MODE_PI_INT,   /* PI 제어 (정수) */
    FW_MODE_ONOFF     /* 켜고 끄기 */
} FwMode;

typedef struct {
    FwMode mode;
    float  kp;
    float  ki;
    float  period_s;     /* 펌웨어가 "알고 있는" 제어 주기 [s] */
    int    ma_len;       /* 이동평균 길이 (1 = 필터 없음) */
    int    hyst;         /* 켜고 끄기 제어의 히스테리시스 폭 */
    int    anti_windup;
    int    q_shift;      /* 정수 제어의 소수부 비트 수 */
    int    acc16;        /* 1 = 16비트 적분기 (오버플로 실험) */
} FwConfig;

void fw_init(const FwConfig *cfg);
void fw_set_target(int target);   /* 목표 속도 [rad/s] */

/* 제어 주기마다 한 번 불린다 (실제 보드에서는 타이머 인터럽트가 부른다) */
void fw_tick(void);

/* 디버그용: 필터를 거친 속도 값을 본다. 제어에는 쓰이지 않는다. */
int fw_debug_filtered(void);

#endif
