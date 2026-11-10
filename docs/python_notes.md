# Python 문법 노트

검증 코드와 그래프 코드를 쓸 때 필요한 것만 모았다.

## 1. 실행과 명령줄 인자

```
python tools/plot.py out/m2_open12.csv out/m3_dc6.csv
```

```python
import sys

print(sys.argv)        # ['tools/plot.py', 'out/m2_open12.csv', 'out/m3_dc6.csv']
filename = sys.argv[1] # 첫 번째 인자
files = sys.argv[1:]   # 첫 번째부터 끝까지 (리스트)
```

파일 맨 아래의 이것은 "이 파일을 직접 실행했을 때만 main 을 부른다"는 뜻이다.

```python
if __name__ == "__main__":
    main()
```

## 2. 리스트

```python
w = [0.0, 120.5, 230.1, 310.7]

w[0]          # 첫 값
w[-1]         # 마지막 값
len(w)        # 개수
w.append(5.0) # 끝에 추가

max(w), min(w), sum(w)
sum(w) / len(w)             # 평균

w[1:3]        # 1번부터 3번 앞까지 → [120.5, 230.1]
w[-100:]      # 마지막 100개
```

반복:

```python
for x in w:                 # 값만
    print(x)

for k in range(len(w)):     # 번호로
    print(k, w[k])

for tt, ww in zip(t, w):    # 두 리스트를 나란히
    print(tt, ww)
```

조건에 맞는 것만 골라 새 리스트 만들기:

```python
tail = []
for k in range(len(t)):
    if t[k] >= 0.8:
        tail.append(w[k])

# 같은 일을 한 줄로 (리스트 내포)
tail = [w[k] for k in range(len(t)) if t[k] >= 0.8]
```

## 3. CSV 읽기

CSV의 첫 줄은 열 이름이다: `t,v,i,w,w_meas,w_filt,duty,target`

```python
import csv

cols = {}                                   # 빈 사전
with open("out/m5_pi.csv", newline="") as f:
    for row in csv.DictReader(f):           # row = {"t": "0.000000", "w": "0.0000", ...}
        for name, text in row.items():
            if name not in cols:
                cols[name] = []
            cols[name].append(float(text))  # 글자를 숫자로

t = cols["t"]       # 시각 리스트
w = cols["w"]       # 속도 리스트
```

- 사전(dict)은 이름으로 값을 찾는 묶음이다: `cols["w"]`.
- `with open(...) as f:` 는 블록이 끝나면 파일을 자동으로 닫는다.
- 파일에서 읽은 것은 모두 **문자열**이다. `float()` 로 바꾸지 않으면 `"10" < "9"` 같은 엉뚱한 비교가 된다.

## 4. 함수, 출력, 종료 코드

```python
def overshoot(w, target):
    """목표를 넘어선 최대량 / 목표."""
    return (max(w) - target) / target

ov = overshoot(w, 400)
```

값이 없음을 나타낼 때는 `None` 을 돌려주고, 받는 쪽에서 `if x is None:` 으로 확인한다.

출력 형식:

```python
print("overshoot %.2f %% (limit %.0f %%)" % (ov * 100, 10))
#      %.2f = 소수 2자리,  %d = 정수,  %s = 문자열,  %% = 퍼센트 글자
print("%-4s %-20s" % ("PASS", "overshoot"))    # -20 = 20칸에 왼쪽 맞춤
```

종료 코드 (`run.py` 가 이 값을 보고 통과/실패를 판단한다):

```python
sys.exit(0)    # 성공
sys.exit(1)    # 실패
```

조건을 모아 한 번에 판단:

```python
results = [True, True, False]
all(results)   # 전부 True 인가 → False
```

## 5. 그래프 (matplotlib)

```python
import matplotlib.pyplot as plt

plt.plot(t, w, label="speed")      # 가로, 세로
plt.plot(t, target, "k--")         # 검은 점선
plt.xlabel("t [s]")
plt.ylabel("w [rad/s]")
plt.grid(True)
plt.legend()                       # label 을 표시
plt.xlim(0.0, 0.3)                 # 가로축 범위 (확대)
plt.show()                         # 창 띄우기
# plt.savefig("m5.png")            # 파일로 저장 (show 보다 먼저)
```

위아래 두 칸:

```python
fig, axes = plt.subplots(2, 1, sharex=True)   # 2행 1열, 가로축 공유
axes[0].plot(t, w)
axes[0].set_ylabel("w")
axes[1].plot(t, duty)
axes[1].set_ylabel("duty")
axes[1].set_xlabel("t [s]")
plt.show()
```

한글 글꼴이 깨질 수 있으므로 축 이름은 영어로 쓰는 것이 편하다.

## 6. 행렬과 고유값 (numpy)

```python
import numpy as np

A = np.array([[-2000.0, -20.0],
              [ 1000.0,  -0.1]])

eigs = np.linalg.eigvals(A)     # 고유값들 (복소수일 수 있다)
for e in eigs:
    print(e.real, e.imag)       # 실수부, 허수부
```

- 실수부 < 0 → 사라진다. 시정수 = −1 / 실수부
- 허수부 ≠ 0 → 출렁인다. 주파수 [Hz] = |허수부| / (2π) → `abs(e.imag) / (2 * np.pi)`

## 7. 자주 만나는 오류

| 메시지 | 대개의 원인 |
|---|---|
| `FileNotFoundError` | 파일 이름이 틀렸거나, 프로젝트 폴더가 아닌 곳에서 실행했다. 먼저 `python run.py` 로 CSV를 만든다 |
| `IndexError: list index out of range` | 리스트 길이보다 큰 번호. `sys.argv[1]` 인데 인자를 안 줬을 때도 |
| `KeyError: 'w'` | 사전에 그 이름이 없다. 열 이름 철자를 확인 |
| `TypeError: ... 'str' and 'float'` | `float()` 로 바꾸지 않았다 |
| `IndentationError` | 들여쓰기가 맞지 않는다. 탭과 공백을 섞지 않는다 |
| `ZeroDivisionError` | 빈 리스트의 평균. 조건에 맞는 값이 하나도 없었다 |

오류 메시지는 **맨 아래 줄**부터 읽는다. 그 위 줄에 몇 번째 줄에서 났는지가 있다.
