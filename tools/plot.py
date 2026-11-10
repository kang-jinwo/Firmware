#!/usr/bin/env python3
"""plot.py — CSV를 읽어 그래프를 그린다 (내가 직접 쓴다)

1차 목표 (M3):  python tools/plot.py out/m2_open12.csv
               → 가로축 t, 세로축 w 인 그래프 한 장

그다음 조금씩 늘린다 (docs/M3_pwm.md 의 "할 일" 참고):
  - 파일 두 개를 겹쳐 그리기
  - w 와 duty 를 위아래 두 칸에 그리기
  - 특정 시간 구간만 확대하기
"""
import csv
import sys

import matplotlib.pyplot as plt


def load(filename):
    """TODO 1: CSV를 읽어 {열 이름: [값, ...]} 사전으로 돌려준다.

    순서:
      1. 빈 사전 cols = {} 를 만든다.
      2. with open(filename, newline="") as f: 로 파일을 연다.
      3. csv.DictReader(f) 로 한 줄씩 읽는다. 한 줄은 {"t": "0.000000", "w": "0.0000", ...} 모양.
      4. 각 값은 문자열이므로 float(...) 로 바꿔서 cols[열이름] 리스트에 append 한다.
    """
    return {}


def main():
    # TODO 2: sys.argv[1] 이 파일 이름이다. load 로 읽는다.
    # TODO 3: plt.plot(가로 리스트, 세로 리스트) 로 그린다.
    # TODO 4: plt.xlabel, plt.ylabel, plt.grid(True) 로 축 이름과 격자를 넣는다.
    # TODO 5: plt.show() 로 창을 띄운다. (파일로 저장하려면 plt.savefig("이름.png"))
    pass


if __name__ == "__main__":
    main()
