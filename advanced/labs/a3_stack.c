/* a3_stack.c — 스택: 함수 호출마다 생겼다 사라지는 메모리
 * 실행: python run.py lab a3_stack
 * 설명: advanced/A3_stack.md
 *
 * 주의: [2]와 [3]은 일부러 "하면 안 되는 일"을 한다 (정의되지 않은 동작).
 *       출력은 컴파일러와 옵션에 따라 다를 수 있다. 그것 자체가 배울 점이다.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static int *g_dangling;     /* 사라진 지역 변수의 주소를 붙잡아 둘 전역 포인터 */

static void depth(int n)
{
    int marker = n;
    printf("  depth(%d)  &marker = %p\n", n, (void *)&marker);
    if (n < 4) depth(n + 1);
}

static void make_local(void)
{
    int secret = 1234;
    g_dangling = &secret;               /* 지역 변수의 주소를 밖으로 빼돌린다 */
    printf("  make_local 안:      &secret = %p, *g_dangling = %d\n", (void *)&secret, *g_dangling);
}

static void overwrite_stack(void)
{
    volatile int other = 9999;          /* make_local 의 secret 이 있던 바로 그 자리에 놓인다 */
    printf("  overwrite_stack 안: &other = %p\n", (void *)&other);
}

struct Packet {
    char     name[8];
    uint32_t checksum;                  /* name 바로 뒤에 놓인다 */
};

static unsigned recurse_sum(unsigned n)
{
    volatile char pad[1024];            /* 호출 한 번에 스택 1 kB 이상 */
    pad[0] = (char)n;
    if (n == 0) return 0;
    return n + recurse_sum(n - 1) + (unsigned)(pad[0] - pad[0]);
}

int main(void)
{
    struct Packet pk;

    printf("[1] 호출이 깊어질수록 주소가 어느 쪽으로 가나\n");
    depth(1);

    printf("[2] 사라진 지역 변수를 가리키는 포인터 (dangling pointer)\n");
    make_local();
    overwrite_stack();
    printf("  두 함수가 끝난 뒤:  *g_dangling = %d   (1234 가 아니면 다른 함수가 그 자리를 덮어쓴 것이다)\n",
           *g_dangling);

    printf("[3] 버퍼를 넘겨 쓰면 옆 변수가 망가진다\n");
    pk.checksum = 0xAABBCCDD;
    printf("  전: checksum = 0x%08X\n", (unsigned)pk.checksum);
    memcpy(&pk, "ABCDEFGHIJ", 10);      /* 8칸짜리 name 에 10글자: 2바이트가 checksum 을 덮는다 */
    printf("  후: checksum = 0x%08X   ('I' = 0x49, 'J' = 0x4A)\n", (unsigned)pk.checksum);

    printf("[4] 재귀 한 번에 스택을 얼마나 쓰나\n");
    printf("  recurse_sum(100) = %u  (깊이 100 → 스택 약 100 kB 사용)\n", recurse_sum(100));
    printf("  작은 마이크로컨트롤러의 RAM 전체가 2~20 kB 다.\n");

    return 0;
}
