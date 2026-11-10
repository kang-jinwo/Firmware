# 센서-모터 제어 펌웨어 구현과 검증

가상의 DC 모터와 센서를 코드로 만들고, 그 위에서 도는 속도 제어 펌웨어를 C로 구현한 뒤,
사양대로 동작하는지 자동 테스트로 검증하는 프로젝트. 실물은 만들지 않는다 (시뮬레이션까지).

전체 계획은 [`PROJECT_PLAN.md`](PROJECT_PLAN.md) 에 있다.

## 처음 시작할 때

1. [`docs/M0_setup.md`](docs/M0_setup.md) 를 따라 내 컴퓨터에 gcc 와 Python 을 준비한다.
2. 프로젝트 폴더에서 `python run.py` 를 실행한다. `out/` 폴더에 CSV가 생기면 준비 끝.
3. 아래 "공부 순서"의 문서를 위에서부터 하나씩 읽는다.

## 문서를 읽는 방법

모든 단계 문서는 같은 순서로 되어 있다.

1. **목표와 끝났다는 기준**
2. **읽을 파일** — 그 단계에서 뜯어볼 코드
3. **설명** — 개념과 코드가 어떻게 이어지는지
4. **실행과 결과** — 명령과, 나와야 하는 숫자
5. **생각해 볼 질문** — 먼저 스스로 답해 보고, `▶ 답과 설명 보기` 를 눌러 펼친다
6. **할 일** — 내가 직접 하는 것 (손 계산, 검증 코드, 실험, 학습 기록)

질문은 "답을 맞히는 것"이 목적이 아니다. 1~2분 생각해서 내 답을 한 줄 적고, 설명을 읽으며 어디가 달랐는지 보는 것이 목적이다.

## 공부 순서 (2주 계획)

| 날 | 단계 | 문서 | 내가 직접 하는 것 |
|---|---|---|---|
| 1 | M0, M1 | [`M0_setup.md`](docs/M0_setup.md), [`spec.md`](docs/spec.md) | 환경 준비, 손 계산 3개 |
| 2 | M2 | [`M2_plant.md`](docs/M2_plant.md) | 모터 모델 읽기, CSV 열어 보기 |
| 3 | M3 | [`M3_pwm.md`](docs/M3_pwm.md) | **`tools/plot.py` 작성** |
| 4 | M4 | [`M4_p_control.md`](docs/M4_p_control.md) | 비례 제어 실험, 남는 오차 손 계산 |
| 5 | M5 | [`M5_pi_control.md`](docs/M5_pi_control.md) | PI 실험, **`tools/eig.py` 작성**, 학습 기록 P1 |
| 6~7 | M6 | [`M6_verification.md`](docs/M6_verification.md) | **`tests/` 의 검증 코드 3개 작성** |
| 8~9 | M7 | [`M7_noise_filter.md`](docs/M7_noise_filter.md) | 학습 기록 P2, P3 |
| 10~11 | M8 | [`M8_integer_timing.md`](docs/M8_integer_timing.md) | 타이밍 표, 학습 기록 P4, P5, P8 |
| 12 | M9 | [`M9_windup_onoff.md`](docs/M9_windup_onoff.md) | 학습 기록 P6, P7 |
| 13 | M10 | [`M10_report.md`](docs/M10_report.md) | 보고서 한 장, 학업계획서용 요약 |
| 14 | 여유 | — | 밀린 것, 다시 보고 싶은 것 |

문법이 막히면: [`docs/c_notes.md`](docs/c_notes.md) (C), [`docs/python_notes.md`](docs/python_notes.md) (Python)

## 폴더

```
PROJECT_PLAN.md      계획서
docs/                단계별 사양서, 설명서 (질문과 답 포함)
plant/               모터·PWM·센서 모델 (C)          "세상" 역할
firmware/            제어 펌웨어 (C)                 프로젝트의 중심
sim/                 plant 와 firmware 를 묶어 돌리고 CSV로 기록 (C)
tests/               검증 코드 뼈대 (내가 직접 채운다)
tools/               그래프 코드 뼈대 (내가 직접 채운다)
solution/            검증·그래프 코드 정답, 참고 그래프 (막혔을 때만 연다)
log/                 학습 기록 (내가 쓴다)
run.py               컴파일 → 실행 → 테스트를 한 번에
```

`build/` (실행 파일)와 `out/` (CSV)은 실행하면 생기고, GitHub에는 올라가지 않는다.

## 실행 방법

```
python run.py              빌드 + 기본 실험 (M2~M5)
python run.py M5           실험 하나만
python run.py P1           문제 실험 하나만 (P1 ~ P8)
python run.py all          전부
python run.py list         실험 목록과 옵션 보기
python run.py test         내가 쓴 검증 코드 실행
python run.py test --solution    정답 검증 코드 실행
```

값을 바꿔 가며 직접 돌릴 때는 시뮬레이터를 바로 부른다 (Windows는 `build\sim.exe`):

```
build/sim --mode pi --kp 0.002 --ki 0.05 --target 400 --out out/my_test.csv
```

### 시뮬레이터 옵션

| 옵션 | 뜻 | 기본값 |
|---|---|---|
| `--mode` | `open`(전압 직접), `pwm`(고정 듀티), `p`, `pi`, `pi_int`(정수 PI), `onoff` | `pi` |
| `--volt` | `open` 모드에서 걸 전압 [V] | 12 |
| `--duty` | `pwm` 모드의 듀티 (0~1) | 0.5 |
| `--kp`, `--ki` | 비례 이득, 적분 이득 | 0.002, 0.02 |
| `--target` | 목표 속도 [rad/s] | 400 |
| `--target2`, `--t2` | `t2`초부터 목표를 `target2`로 바꾼다 | 쓰지 않음 |
| `--noise` | 센서 잡음의 표준편차 [rad/s] | 0 |
| `--ma` | 이동평균 길이 (1 = 필터 없음, 최대 64) | 1 |
| `--period_ms` | 제어 주기 [ms] | 1 |
| `--tend` | 시뮬레이션 길이 [s] | 0.6 |
| `--log_us` | CSV 기록 간격 [µs] | 100 |
| `--hyst` | 켜고 끄기 제어의 히스테리시스 폭 [rad/s] | 0 |
| `--aw` | 1이면 anti-windup 켬 | 0 |
| `--qshift` | 정수 제어의 소수부 비트 수 | 8 |
| `--acc16` | 1이면 적분기를 16비트로 (오버플로 실험) | 0 |
| `--exec_us` | 루프 한 바퀴의 실행 시간 [µs] (P8 실험) | 0 |
| `--seed` | 잡음 난수의 시작값 | 1 |
| `--out` | 결과 CSV 파일 이름 | `out/run.csv` |

### CSV의 열

| 열 | 뜻 |
|---|---|
| `t` | 시각 [s] |
| `v` | 모터에 걸린 전압 [V] |
| `i` | 전류 [A] |
| `w` | **실제** 속도 [rad/s] (펌웨어는 볼 수 없는 값) |
| `w_meas` | 센서가 준 값 (잡음 포함, 정수) |
| `w_filt` | 펌웨어 안에서 필터를 거친 값 |
| `duty` | 펌웨어가 요청한 듀티 (0~1) |
| `target` | 목표 속도 |
