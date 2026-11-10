/* firmware/filter.h — 이동평균 필터 (원형 버퍼) */
#ifndef FILTER_H
#define FILTER_H

#define MA_MAX_LEN 64

typedef struct {
    int  buf[MA_MAX_LEN];  /* 최근 값들을 담는 배열 */
    int  len;              /* 평균 낼 개수 N (1이면 필터 없음과 같다) */
    int  idx;              /* 다음에 쓸 칸 번호 */
    int  count;            /* 지금까지 들어온 개수 (N까지만 센다) */
    long sum;              /* buf에 들어 있는 값들의 합 */
} MovingAvg;

void ma_init(MovingAvg *f, int len);

/* 새 값 x를 넣고, 최근 N개의 평균을 돌려준다. */
int ma_update(MovingAvg *f, int x);

#endif
