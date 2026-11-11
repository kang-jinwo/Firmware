# A1. 메모리 지도: 변수는 어디에 사는가

실습: `python run.py lab a1_memory_map` (코드: `advanced/labs/a1_memory_map.c`)

## 1. 먼저 예상한다

코드를 열고, 실행하기 전에 답을 적는다.

1. `g_init` 과 `local_main` 의 주소는 가까울까, 멀까?
2. `func` 가 자기 자신을 세 번 부른다. `&local` 은 세 번 다 같은 주소일까?
3. 같은 함수 안의 `&s_local` 은 세 번 다 같은 주소일까?
4. `heap1` 이라는 포인터 변수 **자체**는 어디에 있을까? 그것이 **가리키는 곳**은?

## 2. 실행 결과 (예)

```
[code / read-only]
  main (code)                  0x55b8a7213289
  func (code)                  0x55b8a72131ff
  c_const                      0x55b8a7214008
  "hello" string               0x55b8a721400c
[data / bss]  프로그램 내내 산다
  g_init                       0x55b8a7216010
  s_file                       0x55b8a7216014
  g_zero (bss)                 0x55b8a7216024
[heap]  malloc ~ free 사이에만 산다
  *heap1 (block)               0x55b8acb322a0
  *heap2 (block)               0x55b8acb322c0
[stack]  함수가 도는 동안만 산다
  local_main                   0x7fffb7136b14
  heap1 (pointer itself)       0x7fffb7136b18
  func(depth=1)  &local = 0x7fffb7136af4   &s_local = 0x55b8a7216028 (값 1)
  func(depth=2)  &local = 0x7fffb7136ac4   &s_local = 0x55b8a7216028 (값 2)
  func(depth=3)  &local = 0x7fffb7136a94   &s_local = 0x55b8a7216028 (값 3)
```

## 3. 설명

프로그램이 쓰는 메모리는 용도별로 구역이 나뉘어 있다.

```
높은 주소
┌──────────────────┐
│ 스택 (stack)     │  지역 변수, 함수 인자, 돌아갈 주소.  함수를 부르면 자라고 끝나면 줄어든다  ↓
│        ↓         │
│                  │
│        ↑         │
│ 힙 (heap)        │  malloc 으로 빌린 메모리.  free 할 때까지 산다  ↑
├──────────────────┤
│ bss              │  초기값이 없는 전역·static.  시작할 때 0으로 채워진다
│ data             │  초기값이 있는 전역·static
├──────────────────┤
│ rodata           │  const, 문자열 상수.  읽기 전용
│ text (code)      │  기계어 명령어
└──────────────────┘
낮은 주소
```

| 구역 | 무엇이 | 수명 | 크기가 정해지는 때 |
|---|---|---|---|
| text / rodata | 함수, 상수 | 프로그램 내내 | 컴파일할 때 |
| data / bss | 전역, static | 프로그램 내내 | 컴파일할 때 |
| 스택 | 지역 변수 | 그 함수가 도는 동안 | 실행 중 (호출 깊이에 따라) |
| 힙 | malloc 한 것 | free 할 때까지 | 실행 중 |

예상 문제의 답:

1. **멀다.** `g_init` 은 data(0x55b8...), `local_main` 은 스택(0x7fff...). 완전히 다른 구역이다.
2. **다르다.** 호출할 때마다 스택에 새 자리가 생긴다. 0x30(48바이트)씩 낮아진다. 같은 이름의 변수가 동시에 세 개 살아 있다.
3. **같다.** `static` 지역 변수는 이름만 함수 안에 있고, 사는 곳은 data/bss 다. 그래서 값이 1, 2, 3 으로 이어진다.
4. 포인터 변수 `heap1` 은 **스택**에 있고(0x7fff...), 그 안에 든 값(주소)은 **힙**을 가리킨다(0x55b8ac...). "포인터가 사는 곳"과 "포인터가 가리키는 곳"은 별개다.

### 마이크로컨트롤러에서는

같은 구역이 두 종류의 메모리에 나뉘어 들어간다.

| 메모리 | 성질 | 들어가는 것 |
|---|---|---|
| 플래시 (ROM) | 전원이 꺼져도 남는다. 실행 중에 못 바꾼다. 크다 (수십 kB ~ MB) | text, rodata, data 의 초기값 |
| RAM | 전원이 꺼지면 사라진다. 작다 (수 kB ~ 수백 kB) | data, bss, 힙, 스택 |

RAM이 2 kB인 칩이라면 전역 변수 + 스택 + 힙이 전부 2 kB 안에 들어가야 한다. 그래서 펌웨어 개발자는 변수 하나가 어느 구역에 몇 바이트를 차지하는지 안다. `const` 를 붙이면 RAM이 아니라 플래시에 놓인다는 것이 실제로 중요한 최적화다.

## 4. 프로젝트와 연결

`firmware/firmware.c` 위쪽:

```c
static Controller ctrl;
static MovingAvg  filt;
static int        target_speed;
```

- 초기값이 없으므로 **bss**. 크기는 컴파일할 때 정해진다. `MovingAvg` 안에 `int buf[64]` 가 있으므로 이것만 256바이트가 넘는다.
- 프로그램 내내 살기 때문에 `fw_tick()` 이 끝나도 적분 값이 남는다.
- `fw_tick` 안의 `raw`, `meas`, `duty` 는 **스택**. 매 주기 생겼다 사라진다. 다음 주기에 필요 없는 값이니 맞는 자리다.

M5 Q4("적분을 지역 변수로 두면")의 버그는 "스택에 둬야 할 것과 bss에 둬야 할 것을 바꾼" 것이다.

## 5. 질문

**Q1.** `const int c_const = 100;` 을 실행 중에 `*(int *)&c_const = 5;` 로 억지로 바꾸려 하면 어떻게 될까?

<details><summary>▶ 답과 설명 보기</summary>

PC에서는 프로그램이 죽는다 (Segmentation fault). 운영체제가 rodata 구역을 "읽기만 가능"으로 표시해 두었고, CPU가 쓰기를 막는다.

마이크로컨트롤러에서는 그 주소가 플래시다. 쓰기 명령이 그냥 무시되거나 오류 인터럽트가 난다. 어느 쪽이든 값은 바뀌지 않는다.

`const` 는 컴파일러와의 약속일 뿐 아니라, 값이 놓이는 **물리적 위치**를 바꾼다.
</details>

**Q2.** 초기값이 0인 큰 배열 `static int table[10000];` 과 초기값이 있는 `static int table[10000] = {1, 2, 3};` 은 실행 파일 크기에 어떤 차이를 만들까?

<details><summary>▶ 답과 설명 보기</summary>

앞의 것은 bss 라서 실행 파일에 "여기 40,000바이트를 0으로 채워라"는 **크기 정보만** 들어간다. 뒤의 것은 data 라서 초기값 40,000바이트가 **통째로** 파일(플래시)에 들어간다.

확인: 두 버전을 컴파일해 `size` 명령으로 본다 (`size build/프로그램`). `data` 와 `bss` 열이 달라진다.

마이크로컨트롤러에서는 시작 코드(startup code)가 `main` 을 부르기 전에 (1) 플래시의 data 초기값을 RAM으로 복사하고 (2) bss 를 0으로 채운다. "전역 변수가 0으로 시작한다"는 것은 누군가 그 일을 해 주기 때문이다. A7에서 다시 나온다.
</details>

**Q3.** 주소가 실행할 때마다 달라지는 이유는 무엇일까?

<details><summary>▶ 답과 설명 보기</summary>

운영체제가 일부러 매번 다른 위치에 올린다 (ASLR, 주소 공간 배치 난수화). 공격자가 "스택은 항상 이 주소"라고 가정하지 못하게 하는 보안 장치다. A3의 버퍼 넘침과 관련이 있다.

마이크로컨트롤러에는 운영체제가 없어서 주소가 항상 같다. 링커 스크립트라는 파일에 "플래시는 0x08000000 부터, RAM은 0x20000000 부터"라고 적혀 있고, 그대로 놓인다.
</details>

## 6. 직접 해 볼 것

- [ ] 프로젝트의 아래 변수들이 어느 구역에 사는지 표로 적는다: `sim/main.c` 의 `motor`, `main` 안의 `o`(Options), `firmware.c` 의 `ctrl`, `control_pi` 안의 `e`, `controller.c` 의 문자열이 아닌 상수 `PWM_DUTY_MAX`(이것은 메모리에 있기는 한가?).
- [ ] `size build/sim` 을 실행해 text, data, bss 크기를 본다. `filter.h` 의 `MA_MAX_LEN` 을 64에서 1024로 바꾸고 다시 빌드하면 어느 열이 얼마나 늘어나는지 **먼저 계산**하고 확인한다. (끝나면 되돌린다.)
- [ ] `a1_memory_map.c` 에 `static int big[1000];` 과 `int big2[1000] = {1};` 을 추가하고 `size` 로 차이를 본다.

<details><summary>▶ 첫 번째 항목의 답</summary>

| 변수 | 구역 | 이유 |
|---|---|---|
| `motor` (sim/main.c) | bss | static 전역, 초기값 없음 |
| `o` (main 안) | 스택 | 지역 변수 |
| `ctrl` (firmware.c) | bss | static 전역 |
| `e` (control_pi 안) | 스택 (또는 CPU 레지스터) | 지역 변수. 최적화하면 메모리에 놓이지 않고 레지스터에만 있을 수 있다 |
| `PWM_DUTY_MAX` | 어디에도 없다 | `#define` 은 컴파일 전에 숫자 1000 으로 바뀐다. 명령어 안에 숫자로 박힌다 (text) |
</details>

다음: [A2](A2_bytes_and_pointers.md)
