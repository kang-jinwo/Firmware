/* advanced/solution/pool.c — A4 "직접 해 볼 것"의 정답 예: 이중 해제를 잡는 고정 블록 풀 + 테스트
 * 컴파일: gcc -std=c11 -Wall -Wextra -o build/pool advanced/solution/pool.c && build/pool
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define BLOCK_SIZE  32
#define BLOCK_COUNT 4

typedef struct {
    uint8_t  storage[BLOCK_COUNT][BLOCK_SIZE];
    uint8_t  in_use[BLOCK_COUNT];       /* 1 = 빌려준 상태 */
    void    *free_list;
    int      used;
} Pool;

void pool_init(Pool *p)
{
    int k;
    p->free_list = NULL;
    p->used = 0;
    for (k = BLOCK_COUNT - 1; k >= 0; k--) {
        void *block = p->storage[k];
        *(void **)block = p->free_list;
        p->free_list = block;
        p->in_use[k] = 0;
    }
}

void *pool_alloc(Pool *p)
{
    uint8_t *block = p->free_list;
    if (block == NULL) return NULL;
    p->free_list = *(void **)block;
    p->in_use[(block - p->storage[0]) / BLOCK_SIZE] = 1;
    p->used++;
    return block;
}

/* 블록 주소 → 번호. 풀의 블록이 아니면 -1.
 * 포인터 뺄셈: 같은 배열 안의 두 주소를 빼면 "몇 칸 떨어졌는지"가 나온다 (여기서는 바이트 단위). */
static int block_index(const Pool *p, const void *block)
{
    const uint8_t *b = block;
    const uint8_t *start = p->storage[0];
    ptrdiff_t off;

    if (b < start || b >= start + sizeof p->storage) return -1;    /* 풀 밖 */
    off = b - start;
    if (off % BLOCK_SIZE != 0) return -1;                           /* 블록의 시작이 아님 */
    return (int)(off / BLOCK_SIZE);
}

/* 성공 0, 실패 -1 */
int pool_free(Pool *p, void *block)
{
    int k = block_index(p, block);
    if (k < 0) return -1;
    if (!p->in_use[k]) return -1;       /* 이미 빈 블록: 이중 해제 */

    p->in_use[k] = 0;
    *(void **)block = p->free_list;
    p->free_list = block;
    p->used--;
    return 0;
}

/* ---------------- 테스트 ---------------- */
static int fail_count = 0;
#define CHECK(cond) \
    do { if (!(cond)) { printf("FAIL  line %d: %s\n", __LINE__, #cond); fail_count++; } } while (0)

static Pool pool;

int main(void)
{
    void *b[BLOCK_COUNT];
    int outside;
    int k, j;

    pool_init(&pool);

    for (k = 0; k < BLOCK_COUNT; k++) {             /* 전부 빌린다: 서로 다른 주소여야 한다 */
        b[k] = pool_alloc(&pool);
        CHECK(b[k] != NULL);
        for (j = 0; j < k; j++) CHECK(b[k] != b[j]);
    }
    CHECK(pool.used == BLOCK_COUNT);
    CHECK(pool_alloc(&pool) == NULL);               /* 다 썼으면 NULL */

    CHECK(pool_free(&pool, b[1]) == 0);             /* 돌려주면 */
    CHECK(pool.used == BLOCK_COUNT - 1);
    CHECK(pool_free(&pool, b[1]) == -1);            /* 이중 해제는 거절 */
    CHECK(pool.used == BLOCK_COUNT - 1);            /* 장부가 망가지지 않았다 */
    CHECK(pool_alloc(&pool) == b[1]);               /* 같은 자리를 다시 받는다 */
    CHECK(pool_alloc(&pool) == NULL);

    CHECK(pool_free(&pool, &outside) == -1);        /* 풀 밖의 주소는 거절 */
    CHECK(pool_free(&pool, (uint8_t *)b[0] + 1) == -1);   /* 블록 중간의 주소는 거절 */

    for (k = 0; k < BLOCK_COUNT; k++) CHECK(pool_free(&pool, b[k]) == 0);
    CHECK(pool.used == 0);

    if (fail_count == 0) printf("PASS  pool (all checks)\n");
    return fail_count == 0 ? 0 : 1;
}
