#!/usr/bin/env python3
"""eig.py — 닫힌 루프 행렬의 고유값 (내가 직접 쓴다, M5)

설명: docs/M5_pi_control.md 3절

할 일:
  TODO 1: 모터 값(Vs, R, L, Kt, Ke, J, b)을 변수로 적는다.
  TODO 2: kp, ki 를 받아 3×3 행렬 A 를 돌려주는 함수를 만든다.
          A 의 각 칸은 docs/M5_pi_control.md 3절에서 내가 손으로 유도한 것을 쓴다.
  TODO 3: np.linalg.eigvals(A) 로 고유값을 구해 출력한다.
  TODO 4: ki = 0.02, 0.05, 0.2, 2 에 대해 되풀이하고, P1 실험의 그래프와 비교한다.
  TODO 5 (선택): 고유값을 복소평면(가로 = 실수부, 세로 = 허수부)에 점으로 찍는다.
"""
import numpy as np

# 문법 예:
#   A = np.array([[1.0, 2.0],
#                 [3.0, 4.0]])
#   print(np.linalg.eigvals(A))      # 고유값 2개
#   e.real, e.imag                   # 복소수 e 의 실수부, 허수부
