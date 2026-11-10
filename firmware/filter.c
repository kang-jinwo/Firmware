/* firmware/filter.c — 이동평균 필터
 *
 * 원형 버퍼: 배열 끝까지 쓰면 다시 0번 칸으로 돌아가 가장 오래된 값을 덮어쓴다.
 *
 *   len = 4 일 때, 값이 10, 20, 30, 40, 50 순서로 들어오면
 *
 *   buf: [10][20][30][40]   idx → 0 (다음에 쓸 칸, 지금은 가장 오래된 10이 있다)
 *   50이 들어오면:
 *   buf: [50][20][30][40]   idx → 1      sum = 이전 합 − 10 + 50
 *
 * 매번 N개를 다 더하지 않고 "나가는 값은 빼고, 들어오는 값은 더한다".
 * N이 커져도 계산량이 같다 (덧셈 1번, 뺄셈 1번, 나눗셈 1번).
 */
#include "filter.h"

void ma_init(MovingAvg *f, int len)
{
    int k;
    if (len < 1) len = 1;
    if (len > MA_MAX_LEN) len = MA_MAX_LEN;

    f->len = len;
    f->idx = 0;
    f->count = 0;
    f->sum = 0;
    for (k = 0; k < MA_MAX_LEN; k++) f->buf[k] = 0;
}

int ma_update(MovingAvg *f, int x)
{
    f->sum -= f->buf[f->idx];   /* 가장 오래된 값을 합에서 뺀다 (처음엔 0이라 영향 없음) */
    f->buf[f->idx] = x;         /* 그 칸에 새 값을 쓴다 */
    f->sum += x;                /* 새 값을 합에 더한다 */

    f->idx = f->idx + 1;
    if (f->idx >= f->len) f->idx = 0;      /* 끝에 닿으면 처음으로 (원형) */

    if (f->count < f->len) f->count++;     /* 버퍼가 다 차기 전에는 들어온 개수로 나눈다 */

    return (int)(f->sum / f->count);
}
