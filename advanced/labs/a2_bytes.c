/* a2_bytes.c — 메모리는 바이트의 줄이다: 엔디안, 포인터 연산, 구조체 패딩
 * 실행: python run.py lab a2_bytes
 * 설명: advanced/A2_bytes_and_pointers.md
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* 어떤 변수든 "바이트 배열"로 들여다본다.
 * unsigned char* 로는 어떤 메모리든 한 바이트씩 읽어도 된다 (C가 허락한 유일한 방법). */
static void dump(const char *name, const void *addr, size_t n)
{
    const unsigned char *p = addr;
    size_t k;
    printf("  %-10s @ %p :", name, addr);
    for (k = 0; k < n; k++) printf(" %02x", p[k]);
    printf("\n");
}

struct Loose {          /* 멤버 순서가 나쁜 구조체 */
    uint8_t  a;
    uint32_t b;
    uint8_t  c;
    uint16_t d;
};

struct Tight {          /* 큰 것부터 놓은 구조체 */
    uint32_t b;
    uint16_t d;
    uint8_t  a;
    uint8_t  c;
};

int main(void)
{
    uint32_t x = 0x12345678;
    int16_t  neg = -2;
    float    f = 1.0f;
    int      arr[4] = {10, 20, 30, 40};
    int     *p = arr;
    struct Loose lo;

    printf("[1] 엔디안: 0x12345678 은 메모리에 어떤 순서로 놓이나\n");
    dump("x", &x, sizeof x);

    printf("[2] 같은 바이트, 다른 해석\n");
    dump("neg(-2)", &neg, sizeof neg);
    printf("  int16_t 로 읽으면 %d, uint16_t 로 읽으면 %u\n", neg, (unsigned)(uint16_t)neg);
    dump("f(1.0f)", &f, sizeof f);

    printf("[3] 포인터 연산: p + 1 은 주소를 몇 늘리나\n");
    printf("  p     = %p  (*p = %d)\n", (void *)p, *p);
    printf("  p + 1 = %p  (*(p+1) = %d)\n", (void *)(p + 1), *(p + 1));
    printf("  주소 차이 = %d 바이트 = sizeof(int)\n", (int)((char *)(p + 1) - (char *)p));
    printf("  arr[2] = %d,  *(arr + 2) = %d,  2[arr] = %d\n", arr[2], *(arr + 2), 2[arr]);
    dump("arr", arr, sizeof arr);

    printf("[4] 구조체 패딩\n");
    printf("  멤버 크기 합 = %d\n", (int)(sizeof(uint8_t) * 2 + sizeof(uint32_t) + sizeof(uint16_t)));
    printf("  sizeof(struct Loose) = %d   offsets: a=%d b=%d c=%d d=%d\n",
           (int)sizeof(struct Loose),
           (int)offsetof(struct Loose, a), (int)offsetof(struct Loose, b),
           (int)offsetof(struct Loose, c), (int)offsetof(struct Loose, d));
    printf("  sizeof(struct Tight) = %d   offsets: b=%d d=%d a=%d c=%d\n",
           (int)sizeof(struct Tight),
           (int)offsetof(struct Tight, b), (int)offsetof(struct Tight, d),
           (int)offsetof(struct Tight, a), (int)offsetof(struct Tight, c));

    memset(&lo, 0xEE, sizeof lo);       /* 전체를 0xEE 로 칠한 뒤 멤버만 채운다 */
    lo.a = 0x11; lo.b = 0x22222222; lo.c = 0x33; lo.d = 0x4444;
    dump("Loose", &lo, sizeof lo);      /* ee 로 남은 칸이 패딩이다 */

    return 0;
}
