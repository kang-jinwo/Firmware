/* tests/test_filter.c — 이동평균 필터 단위 테스트 (내가 직접 쓴다)
 *
 * 실행: python run.py test
 * 설명: docs/M6_verification.md 2절
 *
 * 단위 테스트란: 함수 하나에 "답을 아는 입력"을 넣고, 나온 값이 답과 같은지 확인하는 작은 프로그램.
 */
#include <stdio.h>
#include "../firmware/filter.h"

static int fail_count = 0;

/* CHECK(조건): 조건이 거짓이면 줄 번호와 조건식을 출력하고 실패 횟수를 1 올린다.
 *   __LINE__ : 이 줄의 번호로 바뀐다.
 *   #cond    : 조건식을 글자 그대로 문자열로 바꾼다.
 *   do { } while (0) : 매크로를 "한 문장"처럼 쓰게 해 주는 관용 표현. */
#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("FAIL  line %d: %s\n", __LINE__, #cond); \
            fail_count++; \
        } \
    } while (0)

/* 예시 하나: 길이 1이면 넣은 값이 그대로 나와야 한다 */
static void test_len1_passthrough(void)
{
    MovingAvg f;            /* 필터 상태를 담을 구조체 변수 */
    ma_init(&f, 1);         /* &f : f의 주소를 넘긴다 → 함수가 f의 내용을 바꿀 수 있다 */
    CHECK(ma_update(&f, 7) == 7);
    CHECK(ma_update(&f, -3) == -3);
}

/* TODO 1: 같은 값(예: 400)을 20번 넣으면 매번 400이 나오는지 (len = 8) */

/* TODO 2: len = 4 에 10, 20, 30, 40, 50 을 차례로 넣을 때 나올 값을
 *         먼저 종이에 계산하고, 그 값과 같은지 CHECK 한다.
 *         (버퍼가 다 차기 전에는 "들어온 개수"로 나눈다는 점에 주의) */

/* TODO 3: 버퍼를 여러 바퀴 돈 뒤에도 옛 값이 남아 있지 않은지
 *         (예: 1000을 103번 넣고, 8을 4번 넣으면 마지막 출력은?) */

/* TODO 4: 0을 8번 넣은 뒤 80을 계속 넣으면 출력이 몇 번 만에 80이 되는지 (len = 8)
 *         → 이것이 필터의 "지연"이다 (M7에서 다시 쓴다) */

/* TODO 5: ma_init 에 길이 0, 1000 같은 이상한 값을 주면 f.len 이 어떻게 되어야 하는지 */

int main(void)
{
    test_len1_passthrough();
    /* TODO: 위에서 만든 테스트 함수들을 여기서 부른다 */

    if (fail_count == 0) printf("PASS  test_filter (all checks)\n");
    else                 printf("test_filter: %d check(s) failed\n", fail_count);
    return fail_count == 0 ? 0 : 1;     /* 종료 코드 0 = 성공. run.py 가 이 값을 본다 */
}
