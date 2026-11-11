/* a5_registers.c — 레지스터: 주소에 붙어 있는 하드웨어
 * 실행: python run.py lab a5_registers
 * 설명: advanced/A5_registers.md
 *
 * 실제 칩에서는 정해진 주소(예: 0x40000000)에 타이머와 ADC의 레지스터가 있다.
 * PC에는 그런 주소가 없으므로, 전역 변수 하나를 "가짜 칩"으로 삼고 그 주소를 쓴다.
 * 접근하는 코드의 모양은 실제 펌웨어와 똑같다.
 */
#include <stdio.h>
#include <stdint.h>

/* ---- 레지스터 배치: 구조체의 멤버 순서 = 메모리의 주소 순서 ---- */
typedef struct {
    volatile uint32_t CR;      /* +0x00 제어 레지스터 */
    volatile uint32_t ARR;     /* +0x04 주기 (자동 재장전 값) */
    volatile uint32_t CCR;     /* +0x08 비교 값 = 듀티 */
    volatile uint32_t SR;      /* +0x0C 상태 */
} TimerRegs;

typedef struct {
    volatile uint32_t CR;      /* +0x00 */
    volatile uint32_t DR;      /* +0x04 변환 결과 */
} AdcRegs;

/* CR 레지스터의 비트 뜻 */
#define TIM_CR_ENABLE   (1u << 0)      /* 0번 비트: 타이머 켜기 */
#define TIM_CR_PWM_MODE (1u << 3)      /* 3번 비트: PWM 모드 */
#define TIM_CR_PRESC_POS 8             /* 8~10번 비트(3비트): 분주비 */
#define TIM_CR_PRESC_MASK (0x7u << TIM_CR_PRESC_POS)

/* ---- 가짜 칩 ---- */
static TimerRegs fake_timer;
static AdcRegs   fake_adc;

/* 실제 칩이라면:  #define TIM1 ((TimerRegs *)0x40012C00)  처럼 숫자 주소를 쓴다 */
#define TIM1 (&fake_timer)
#define ADC1 (&fake_adc)

/* ---- 프로젝트의 hal.h 와 같은 두 함수를, 레지스터로 구현한다 ---- */
#define PWM_DUTY_MAX 1000

static void hal_init(void)
{
    TIM1->ARR = PWM_DUTY_MAX;                        /* 한 주기 = 1000 카운트 */
    TIM1->CR |= TIM_CR_PWM_MODE;                     /* 비트 하나 켜기: OR */
    TIM1->CR = (TIM1->CR & ~TIM_CR_PRESC_MASK)       /* 여러 비트짜리 칸 바꾸기: 지우고 */
             | (5u << TIM_CR_PRESC_POS);             /*                           넣는다 */
    TIM1->CR |= TIM_CR_ENABLE;
}

static void pwm_set(int duty)
{
    TIM1->CCR = (uint32_t)duty;                      /* 레지스터에 쓰는 것이 곧 하드웨어 명령 */
}

static int sensor_read(void)
{
    return (int)ADC1->DR;                            /* 레지스터를 읽는 것이 곧 측정 */
}

static void show_bits(const char *name, uint32_t v)
{
    int k;
    printf("  %-8s = 0x%08X  ...", name, (unsigned)v);
    for (k = 11; k >= 0; k--) printf("%d%s", (int)((v >> k) & 1u), (k % 4 == 0) ? " " : "");
    printf("(아래 12비트)\n");
}

int main(void)
{
    printf("[1] 구조체 멤버의 주소 = 레지스터의 주소\n");
    printf("  TIM1       = %p\n", (void *)TIM1);
    printf("  &TIM1->CR  = %p\n  &TIM1->ARR = %p\n  &TIM1->CCR = %p\n",
           (void *)&TIM1->CR, (void *)&TIM1->ARR, (void *)&TIM1->CCR);

    printf("[2] 비트 조작\n");
    show_bits("CR 처음", TIM1->CR);
    hal_init();
    show_bits("CR 설정", TIM1->CR);
    printf("  켜져 있나? %s\n", (TIM1->CR & TIM_CR_ENABLE) ? "yes" : "no");
    printf("  분주비 칸 = %u\n", (unsigned)((TIM1->CR & TIM_CR_PRESC_MASK) >> TIM_CR_PRESC_POS));
    TIM1->CR &= ~TIM_CR_ENABLE;                      /* 비트 하나 끄기: AND NOT */
    show_bits("CR 끔", TIM1->CR);

    printf("[3] HAL 두 함수\n");
    pwm_set(337);
    printf("  pwm_set(337) 뒤 CCR = %u\n", (unsigned)TIM1->CCR);
    fake_adc.DR = 398;                               /* "하드웨어"가 값을 바꿨다고 치자 */
    printf("  sensor_read() = %d\n", sensor_read());

    printf("[4] 클램프가 없으면: pwm_set(-200)\n");
    pwm_set(-200);
    printf("  CCR = %u  (부호 없는 32비트로 바뀌었다)\n", (unsigned)TIM1->CCR);

    return 0;
}
