/* solution/test_filter.c (정답) — 이동평균 필터 단위 테스트
 *
 * 컴파일: gcc -o build/test_filter solution/test_filter.c firmware/filter.c firmware/controller.c
 */
#include <stdio.h>
#include "../firmware/filter.h"

static int fail_count = 0;

/* 조건이 거짓이면 어느 줄에서 틀렸는지 출력하고 실패 횟수를 센다.
 * #cond 는 조건식을 글자 그대로 문자열로 바꿔 준다. */
#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("FAIL  line %d: %s\n", __LINE__, #cond); \
            fail_count++; \
        } \
    } while (0)

/* 길이 1이면 넣은 값이 그대로 나와야 한다 */
static void test_len1_passthrough(void)
{
    MovingAvg f;
    ma_init(&f, 1);
    CHECK(ma_update(&f, 7) == 7);
    CHECK(ma_update(&f, -3) == -3);
    CHECK(ma_update(&f, 100) == 100);
}

/* 같은 값만 계속 넣으면 평균도 그 값이어야 한다 */
static void test_constant_input(void)
{
    MovingAvg f;
    int k;
    ma_init(&f, 8);
    for (k = 0; k < 20; k++) {
        CHECK(ma_update(&f, 400) == 400);
    }
}

/* 손으로 계산한 값과 비교: len=4 에 10,20,30,40,50 */
static void test_known_sequence(void)
{
    MovingAvg f;
    ma_init(&f, 4);
    CHECK(ma_update(&f, 10) == 10);   /* 10 / 1 */
    CHECK(ma_update(&f, 20) == 15);   /* 30 / 2 */
    CHECK(ma_update(&f, 30) == 20);   /* 60 / 3 */
    CHECK(ma_update(&f, 40) == 25);   /* 100 / 4 */
    CHECK(ma_update(&f, 50) == 35);   /* (20+30+40+50) / 4  ← 10이 빠졌는지 확인 */
    CHECK(ma_update(&f, 50) == 42);   /* (30+40+50+50) / 4 = 42.5 → 정수 나눗셈으로 42 */
}

/* 버퍼를 여러 바퀴 돈 뒤에도 합이 틀어지지 않는지 (원형 버퍼의 idx 처리 확인) */
static void test_wraparound(void)
{
    MovingAvg f;
    int k, out = 0;
    ma_init(&f, 4);
    for (k = 0; k < 103; k++) ma_update(&f, 1000);
    for (k = 0; k < 4; k++) out = ma_update(&f, 8);
    CHECK(out == 8);                  /* 옛 값 1000이 전부 빠져나갔어야 한다 */
}

/* 계단 입력: 0만 넣다가 80을 넣으면 N번에 걸쳐 올라간다 → 필터의 지연 */
static void test_step_delay(void)
{
    MovingAvg f;
    int k;
    ma_init(&f, 8);
    for (k = 0; k < 8; k++) ma_update(&f, 0);
    CHECK(ma_update(&f, 80) == 10);   /* 80 / 8 */
    CHECK(ma_update(&f, 80) == 20);
    CHECK(ma_update(&f, 80) == 30);
    CHECK(ma_update(&f, 80) == 40);   /* 4번째에야 절반 */
    for (k = 0; k < 3; k++) ma_update(&f, 80);
    CHECK(ma_update(&f, 80) == 80);   /* 8번째에 다 올라옴 */
}

/* 범위를 벗어난 길이를 줘도 배열 밖을 건드리지 않아야 한다 */
static void test_len_limits(void)
{
    MovingAvg f;
    ma_init(&f, 0);
    CHECK(f.len == 1);
    ma_init(&f, 1000);
    CHECK(f.len == MA_MAX_LEN);
}

int main(void)
{
    test_len1_passthrough();
    test_constant_input();
    test_known_sequence();
    test_wraparound();
    test_step_delay();
    test_len_limits();

    if (fail_count == 0) printf("PASS  test_filter (all checks)\n");
    else                 printf("test_filter: %d check(s) failed\n", fail_count);
    return fail_count == 0 ? 0 : 1;
}
