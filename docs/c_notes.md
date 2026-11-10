# C 문법 노트

프로젝트에 나오는 C 문법을 "어디에 왜 쓰였는지"와 함께 모았다. 막힐 때 찾아본다.

## 1. 구조체 (struct)

관련된 값을 한 묶음으로 만든다.

```c
typedef struct {
    double i;   /* 전류 */
    double w;   /* 속도 */
} MotorState;

MotorState motor;      /* 변수 하나 */
motor.i = 0.0;         /* 점(.)으로 멤버에 접근 */
```

`typedef` 를 붙이면 `struct MotorState` 대신 `MotorState` 라고만 써도 된다.

쓰인 곳: `MotorState`, `MotorParams` (plant/motor.h), `MovingAvg` (firmware/filter.h), `Controller` (firmware/controller.h)

## 2. 포인터

포인터는 **주소를 담는 변수**다.

```c
int x = 5;
int *p = &x;     /* p 에 x 의 주소를 넣는다.  & = "주소를 구한다" */
*p = 7;          /* p 가 가리키는 곳에 7 을 쓴다.  * = "따라간다" */
                 /* 이제 x 는 7 이다 */
```

```
 변수   주소     값
 x     0x1000   7        ← *p 로 여기를 고쳤다
 p     0x1008   0x1000   ← x 의 주소가 들어 있다
```

### 왜 쓰는가: 함수가 호출한 쪽의 값을 바꾸게 하려고

C는 함수에 인자를 넘길 때 **복사**한다.

```c
void bad(int a)   { a = 99; }     /* 복사본을 고친다. 원본은 그대로 */
void good(int *a) { *a = 99; }    /* 주소를 따라가 원본을 고친다 */

int x = 5;
bad(x);     /* x 는 여전히 5 */
good(&x);   /* x 는 99 */
```

### 구조체 포인터와 `->`

```c
void motor_step(MotorState *m, ...)
{
    m->i += ...;      /* (*m).i 를 줄여 쓴 것. "m 이 가리키는 구조체의 i" */
}

motor_step(&motor, ...);
```

구조체를 포인터로 넘기는 이유 두 가지: (1) 함수가 내용을 바꿀 수 있다, (2) 큰 구조체를 통째로 복사하지 않아도 된다 (`MovingAvg` 는 64칸 배열이 들어 있다).

### `const` 포인터

```c
void motor_step(MotorState *m, const MotorParams *p, ...)
```

`const MotorParams *p` 는 "p 가 가리키는 값을 이 함수 안에서 바꾸지 않겠다"는 약속이다. `p->R = 0;` 이라고 쓰면 컴파일 오류가 난다. 읽기만 하는 인자에는 붙이는 것이 좋은 습관이다.

## 3. 배열과 포인터, 원형 버퍼

```c
int buf[4] = {10, 20, 30, 40};
buf[0]          /* 첫 칸 */
buf[3]          /* 마지막 칸 */
buf[4]          /* 범위 밖! C 는 막아 주지 않는다. 엉뚱한 메모리를 읽거나 망가뜨린다 */
```

배열 이름은 첫 칸의 주소처럼 쓰인다. `buf[k]` 는 `*(buf + k)` 와 같다 ("첫 칸에서 k칸 떨어진 곳").

원형 버퍼 (firmware/filter.c): 번호가 끝에 닿으면 0으로 돌아간다.

```c
f->buf[f->idx] = x;
f->idx = f->idx + 1;
if (f->idx >= f->len) f->idx = 0;
```

`f->buf[f->idx]` 읽는 법: f 가 가리키는 구조체 → 그 안의 배열 buf → 그 배열의 idx번째 칸.

## 4. 헤더 파일과 여러 파일 컴파일

| | 헤더 (`.h`) | 소스 (`.c`) |
|---|---|---|
| 담는 것 | 선언 (함수의 이름과 모양, 구조체 정의) | 정의 (함수의 몸통) |
| 비유 | 차림표 | 주방 |

```
컴파일: 각 .c → .o   (서로의 .h 만 본다)
링크:   모든 .o → 실행 파일   (선언과 몸통을 이어 붙인다)
```

`gcc a.c b.c -o prog` 는 이 두 단계를 한 번에 한다.

헤더 맨 위의 이것은 같은 헤더가 두 번 포함되어도 한 번만 읽히게 한다 (include guard).

```c
#ifndef MOTOR_H
#define MOTOR_H
...
#endif
```

`#include "motor.h"` (큰따옴표)는 내 프로젝트의 파일, `#include <stdio.h>` (꺾쇠)는 시스템 라이브러리.

## 5. `static`

두 가지 뜻이 있다. 이 프로젝트에서는 둘 다 "이 파일 안에서만 쓴다"는 뜻으로 쓰였다.

```c
static Controller ctrl;                 /* 파일 맨 위: 이 파일에서만 보이는 전역 변수 */
static double uniform01(...) { ... }    /* 이 파일에서만 부를 수 있는 함수 */
```

함수 밖에 있는 변수는 프로그램이 도는 동안 계속 살아 있다. `fw_tick` 이 끝나도 `ctrl` 안의 적분 값이 남는 이유다.

## 6. 함수 포인터

함수도 메모리 어딘가에 있고 주소가 있다. 그 주소를 변수에 담아 두었다가 부를 수 있다.

```c
/* "Controller*, int, int 를 받아 int 를 돌려주는 함수"를 가리키는 자료형에 이름을 붙인다 */
typedef int (*ControlFn)(Controller *c, int target, int measured);

ControlFn step;            /* 함수 포인터 변수 */
step = control_pi;         /* 함수 이름이 곧 주소다. 괄호를 붙이지 않는다 */
duty = step(&ctrl, 400, 398);   /* control_pi(&ctrl, 400, 398) 과 같다 */

step = control_onoff;      /* 가리키는 함수를 바꾼다 */
duty = step(&ctrl, 400, 398);   /* 이번에는 control_onoff 가 불린다 */
```

선언 읽는 법 `int (*ControlFn)(...)`: 가운데 `(*ControlFn)` 이 "포인터다", 뒤의 `(...)` 가 "함수를 가리킨다", 앞의 `int` 가 "그 함수는 int 를 돌려준다".

쓰인 곳: `firmware/controller.h` 의 `ControlFn`, `firmware/firmware.c` 의 `ctrl.step`

## 7. 정수 자료형과 오버플로

`#include <stdint.h>` 를 하면 크기가 분명한 자료형을 쓸 수 있다.

| 자료형 | 비트 | 범위 |
|---|---|---|
| `int8_t` | 8 | −128 ~ 127 |
| `int16_t` | 16 | −32,768 ~ 32,767 |
| `int32_t` | 32 | −2,147,483,648 ~ 2,147,483,647 |
| `uint16_t` | 16 | 0 ~ 65,535 |

`int` 의 크기는 컴퓨터마다 다르다 (PC에서 32비트, 작은 마이크로컨트롤러에서 16비트). 펌웨어에서는 크기를 분명히 적는 쪽을 쓴다.

범위를 넘으면 값이 한 바퀴 돈다: `int16_t` 에서 32,767 + 1 → −32,768. (M8의 P5)

정수 나눗셈은 소수점 아래를 버린다: `7 / 2` 는 3.

비트 밀기:

```c
x << 3      /* 왼쪽으로 3비트 = 8을 곱한다 */
x >> 3      /* 오른쪽으로 3비트 = 8로 나눈다 (나머지 버림) */
1L << q     /* 2 의 q 제곱 */
```

형 변환: `(float)x`, `(int32_t)y` 처럼 괄호 안에 자료형을 쓴다. `(int)3.9` 는 3 (버림).

## 8. 이중 포인터: `char **argv`

이 프로젝트에서 이중 포인터가 나오는 유일한 곳이다 (`sim/main.c` 의 `main`).

```c
int main(int argc, char **argv)
```

- 문자열 하나는 `char *` (첫 글자의 주소)다.
- 명령줄 인자는 문자열 여러 개다 → `char *` 의 배열.
- 배열을 함수에 넘기면 첫 칸의 주소가 넘어간다 → `char *` 를 가리키는 포인터 → `char **`.

```
build/sim --mode pi 로 실행하면

argv ──▶ ┌────────┐
         │ argv[0]│──▶ "build/sim"
         │ argv[1]│──▶ "--mode"
         │ argv[2]│──▶ "pi"
         └────────┘
argc = 3
```

`argv[1]` 은 `char *` (문자열), `argv[1][0]` 은 `char` (글자 '-').

문자열 비교는 `==` 가 아니라 `strcmp(a, b) == 0` 으로 한다. `==` 는 주소를 비교한다.

## 9. 파일 쓰기

```c
FILE *fp = fopen("out/run.csv", "w");     /* "w" = 쓰기. 실패하면 NULL */
if (fp == NULL) { ... }                   /* 반드시 확인한다 (폴더가 없으면 실패) */
fprintf(fp, "%.6f,%d\n", t, value);       /* printf 와 같고, 맨 앞에 fp 가 붙는다 */
fclose(fp);                               /* 닫아야 내용이 확실히 저장된다 */
```

자주 쓰는 형식: `%d` 정수, `%ld` long, `%f` 실수, `%.4f` 소수 4자리, `%s` 문자열.

## 10. 매크로

```c
#define PWM_DUTY_MAX 1000
```

컴파일 전에 글자를 그대로 바꿔 치운다. 상수에 이름을 붙이는 데 쓴다. 숫자 1000 을 코드 여기저기에 직접 쓰면, 바꿀 때 하나를 빠뜨린다.

`tests/` 의 `CHECK` 매크로처럼 여러 줄짜리는 `\` 로 줄을 잇고 `do { ... } while (0)` 으로 감싼다.
