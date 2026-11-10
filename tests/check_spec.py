#!/usr/bin/env python3
"""check_spec.py — 사양 검증 (내가 직접 쓴다)

docs/spec.md 4절의 기준을 CSV에서 재서 PASS/FAIL로 출력한다.

실행:  python run.py test
       (run.py 가 out/spec_nominal.csv, out/spec_noise.csv 를 만들고 이 파일을 부른다)
설명:  docs/M6_verification.md 4절
"""
import csv
import sys

# TODO 0: docs/spec.md 4절의 기준 숫자를 여기 상수로 적는다. (코드를 짜기 전에 먼저!)
# SETTLE_MAX_S = ...


def load(filename):
    """CSV를 읽어 {열 이름: [값, 값, ...]} 사전으로 돌려준다. (이 함수는 완성돼 있다)

    쓰는 법:  data = load("out/spec_nominal.csv")
              data["t"][0]   → 첫 시각
              data["w"][-1]  → 마지막 속도
    """
    cols = {}
    with open(filename, newline="") as f:
        for row in csv.DictReader(f):          # 한 줄씩, {"t": "0.000000", "w": "0.0000", ...}
            for name, text in row.items():
                cols.setdefault(name, []).append(float(text))
    return cols


def settling_time(t, w, target, band):
    """TODO 1: 속도가 목표의 ±band 안에 들어가서 다시 나오지 않게 되는 시각."""
    return None


def overshoot(w, target):
    """TODO 2: 목표를 넘어선 최대량 / 목표."""
    return None


def steady_state_error(t, w, target):
    """TODO 3: 마지막 0.2초의 평균 속도와 목표의 차이 / 목표."""
    return None


def ripple(t, w):
    """TODO 4: 마지막 0.5초 동안 속도의 (최대 − 최소)."""
    return None


def main():
    nom = load(sys.argv[1])     # 잡음 없는 실행
    noi = load(sys.argv[2])     # 잡음 있는 실행

    # TODO 5: 위 함수들을 불러 값을 구하고, 기준과 비교해 한 줄씩 출력한다.
    #         예)  PASS  overshoot   0.09 % (limit 10 %)
    # TODO 6: 듀티가 항상 0~1 인지 (두 실행 모두)
    # TODO 7 (M8 이후): 루프 실행 시간 추정값 < 제어 주기
    # TODO 8: 하나라도 FAIL이면 sys.exit(1), 전부 PASS면 sys.exit(0)
    print("check_spec.py: not written yet")
    sys.exit(1)


if __name__ == "__main__":
    main()
