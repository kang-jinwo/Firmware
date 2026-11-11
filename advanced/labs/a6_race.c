/* a6_race.c — 두 실행 흐름이 같은 변수를 건드릴 때 (경쟁 상태)
 * 실행: python run.py lab a6_race
 * 설명: advanced/A6_interrupts_and_races.md
 *
 * PC에는 인터럽트가 없으므로 스레드 2개로 흉내 낸다.
 * "메인 루프"와 "인터럽트 처리 함수"가 같은 변수를 쓰는 상황과 구조가 같다.
 */
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

#define N 1000000

static volatile long counter_plain;        /* 보호 없음 */
static long          counter_mutex;        /* 자물쇠로 보호 */
static atomic_long   counter_atomic;       /* 원자적 연산 */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *worker(void *arg)
{
    long k;
    (void)arg;
    for (k = 0; k < N; k++) {
        counter_plain++;                   /* 읽기 → 더하기 → 쓰기, 세 단계다 */

        pthread_mutex_lock(&lock);         /* 한 번에 한 흐름만 들어온다 */
        counter_mutex++;
        pthread_mutex_unlock(&lock);

        atomic_fetch_add(&counter_atomic, 1);   /* 세 단계를 쪼갤 수 없는 한 동작으로 */
    }
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    int run;

    for (run = 1; run <= 3; run++) {
        counter_plain = 0;
        counter_mutex = 0;
        atomic_store(&counter_atomic, 0);

        pthread_create(&t1, NULL, worker, NULL);
        pthread_create(&t2, NULL, worker, NULL);
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);

        printf("run %d  expected %d\n", run, 2 * N);
        printf("  plain  = %ld   (잃어버린 횟수 %ld)\n", counter_plain, 2L * N - counter_plain);
        printf("  mutex  = %ld\n", counter_mutex);
        printf("  atomic = %ld\n", atomic_load(&counter_atomic));
    }
    return 0;
}
