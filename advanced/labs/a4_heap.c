/* a4_heap.c — 힙(malloc)과 고정 블록 풀
 * 실행: python run.py lab a4_heap
 * 설명: advanced/A4_heap_and_pool.md
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ---------- 펌웨어식 대안: 고정 블록 풀 ----------
 * 같은 크기의 블록 N개를 미리 잡아 두고 빌려준다.
 * 빈 블록들은 "빈 블록의 첫 칸에 다음 빈 블록의 주소를 적어" 줄줄이 잇는다 (연결 리스트).
 */
#define BLOCK_SIZE  32
#define BLOCK_COUNT 4

typedef struct {
    uint8_t  storage[BLOCK_COUNT][BLOCK_SIZE];   /* 실제 메모리. 크기가 컴파일 때 정해진다 */
    void    *free_list;                          /* 첫 번째 빈 블록 */
    int      used;
} Pool;

static void pool_init(Pool *p)
{
    int k;
    p->free_list = NULL;
    p->used = 0;
    for (k = BLOCK_COUNT - 1; k >= 0; k--) {
        void *block = p->storage[k];
        *(void **)block = p->free_list;     /* 블록의 첫 칸에 "다음 빈 블록" 주소를 쓴다 */
        p->free_list = block;
    }
}

static void *pool_alloc(Pool *p)
{
    void *block = p->free_list;
    if (block == NULL) return NULL;         /* 다 썼다. 호출한 쪽이 반드시 확인해야 한다 */
    p->free_list = *(void **)block;         /* 빈 목록의 머리를 다음 블록으로 옮긴다 */
    p->used++;
    return block;
}

static void pool_free(Pool *p, void *block)
{
    *(void **)block = p->free_list;         /* 돌려받은 블록을 빈 목록의 맨 앞에 끼운다 */
    p->free_list = block;
    p->used--;
}

static Pool pool;       /* static: bss 영역. 스택도 힙도 쓰지 않는다 */

int main(void)
{
    char *a, *b, *c;
    void *blk[BLOCK_COUNT + 1];
    int k;

    printf("[1] malloc: 요청한 크기와 실제 간격\n");
    a = malloc(10);
    b = malloc(10);
    c = malloc(100);
    printf("  a = %p\n  b = %p   (a 와의 간격 %d 바이트: 10을 달라 했는데?)\n  c = %p\n",
           (void *)a, (void *)b, (int)(b - a), (void *)c);

    printf("[2] free 한 자리는 다시 쓰인다\n");
    strcpy(a, "motor");
    free(a);
    a = malloc(10);                         /* 방금 돌려준 자리를 다시 받을 가능성이 높다 */
    printf("  다시 malloc(10) = %p\n", (void *)a);

    printf("[3] 고정 블록 풀: 주소가 미리 정해져 있다\n");
    pool_init(&pool);
    for (k = 0; k < BLOCK_COUNT + 1; k++) {
        blk[k] = pool_alloc(&pool);
        printf("  alloc #%d -> %p   (used = %d)\n", k, blk[k], pool.used);
    }
    pool_free(&pool, blk[1]);
    printf("  blk[1] 을 돌려준 뒤 alloc -> %p  (같은 자리)\n", pool_alloc(&pool));

    free(a); free(b); free(c);
    return 0;
}
