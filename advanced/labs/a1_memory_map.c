/* a1_memory_map.c — 변수는 메모리의 어디에 사는가
 * 실행: python run.py lab a1_memory_map
 * 설명: advanced/A1_memory_map.md
 */
#include <stdio.h>
#include <stdlib.h>

int        g_init = 42;          /* 초기값 있는 전역      → data 영역 */
int        g_zero;               /* 초기값 없는 전역      → bss 영역 (0으로 채워짐) */
static int s_file = 7;           /* 파일 안 static        → data 영역 */
const int  c_const = 100;        /* const 전역            → 읽기 전용 영역 (rodata) */
const char *msg = "hello";       /* 문자열 상수 "hello"   → 읽기 전용 영역 */

static void show(const char *name, const void *addr)
{
    printf("  %-28s %p\n", name, addr);
}

static void func(int depth)
{
    int        local = depth;    /* 지역 변수             → 스택 */
    static int s_local = 0;      /* 함수 안 static        → data/bss (스택이 아니다!) */

    s_local++;
    printf("  func(depth=%d)  &local = %p   &s_local = %p (값 %d)\n",
           depth, (void *)&local, (void *)&s_local, s_local);
    if (depth < 3) func(depth + 1);
}

int main(void)
{
    int  local_main = 1;
    int *heap1 = malloc(sizeof(int));
    int *heap2 = malloc(sizeof(int));

    printf("[code / read-only]\n");
    show("main (code)", (const void *)main);
    show("func (code)", (const void *)func);
    show("c_const", &c_const);
    show("\"hello\" string", msg);

    printf("[data / bss]  프로그램 내내 산다\n");
    show("g_init", &g_init);
    show("s_file", &s_file);
    show("g_zero (bss)", &g_zero);

    printf("[heap]  malloc ~ free 사이에만 산다\n");
    show("*heap1 (block)", heap1);
    show("*heap2 (block)", heap2);

    printf("[stack]  함수가 도는 동안만 산다\n");
    show("local_main", &local_main);
    show("heap1 (pointer itself)", &heap1);
    func(1);

    free(heap1);
    free(heap2);
    return 0;
}
