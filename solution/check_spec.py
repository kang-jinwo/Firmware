#!/usr/bin/env python3
"""check_spec.py (정답) — 사양서(docs/spec.md 4절)의 검증 기준을 PASS/FAIL로 출력한다.

사용법:  python solution/check_spec.py out/spec_nominal.csv out/spec_noise.csv
전부 PASS면 종료 코드 0, 하나라도 FAIL이면 1.
"""
import csv
import sys

# ---- 사양서의 기준 (docs/spec.md 4절). 코드를 보기 전에 정한 숫자다. ----
SETTLE_BAND = 0.02        # ±2 %
SETTLE_MAX_S = 0.25       # 도달 시간 한계 [s]
OVERSHOOT_MAX = 0.10      # 10 %
SSE_MAX = 0.01            # 1 %
RIPPLE_MAX = 20.0         # 잡음이 있을 때 속도 흔들림 한계 [rad/s]
PERIOD_S = 0.001          # 제어 주기

# ---- 타이밍 추정 (docs/M8_integer_timing.md 의 표) ----
INSTRUCTIONS = 110        # 루프 한 바퀴의 명령어 수 (정수 PI)
CPI = 2.0                 # 명령어 하나당 평균 클록 수 (가정)
CLOCK_HZ = 16e6           # MCU 클록 (가정)


def load(filename):
    """CSV를 읽어 {열 이름: [값, 값, ...]} 사전으로 돌려준다."""
    cols = {}
    with open(filename, newline="") as f:
        for row in csv.DictReader(f):
            for name, text in row.items():
                cols.setdefault(name, []).append(float(text))
    return cols


def settling_time(t, w, target, band):
    """마지막으로 허용 띠 밖에 있었던 시각의 다음 시각. 끝까지 못 들어오면 None."""
    lo = target * (1 - band)
    hi = target * (1 + band)
    last_outside = -1
    for k in range(len(w)):
        if w[k] < lo or w[k] > hi:
            last_outside = k
    if last_outside == len(w) - 1:
        return None
    return t[last_outside + 1]


def overshoot(w, target):
    """목표를 넘어선 최대량 / 목표. 넘지 않았으면 0."""
    return max(0.0, (max(w) - target) / target)


def tail(t, x, seconds):
    """마지막 seconds초 구간의 값들만 골라 낸다."""
    t_start = t[-1] - seconds
    return [x[k] for k in range(len(x)) if t[k] >= t_start]


def steady_state_error(t, w, target):
    last = tail(t, w, 0.2)
    mean = sum(last) / len(last)
    return abs(mean - target) / target


def ripple(t, w):
    last = tail(t, w, 0.5)
    return max(last) - min(last)


def report(name, ok, detail):
    print("%-4s  %-28s %s" % ("PASS" if ok else "FAIL", name, detail))
    return ok


def main():
    if len(sys.argv) != 3:
        sys.exit("usage: check_spec.py <nominal.csv> <noise.csv>")

    nom = load(sys.argv[1])
    noi = load(sys.argv[2])
    target = nom["target"][-1]
    results = []

    ts = settling_time(nom["t"], nom["w"], target, SETTLE_BAND)
    results.append(report("settling time",
                          ts is not None and ts <= SETTLE_MAX_S,
                          "%s s (limit %.2f s)" % ("never" if ts is None else "%.4f" % ts, SETTLE_MAX_S)))

    ov = overshoot(nom["w"], target)
    results.append(report("overshoot", ov <= OVERSHOOT_MAX,
                          "%.2f %% (limit %.0f %%)" % (ov * 100, OVERSHOOT_MAX * 100)))

    sse = steady_state_error(nom["t"], nom["w"], target)
    results.append(report("steady-state error", sse <= SSE_MAX,
                          "%.3f %% (limit %.0f %%)" % (sse * 100, SSE_MAX * 100)))

    rp = ripple(noi["t"], noi["w"])
    results.append(report("speed ripple with noise", rp <= RIPPLE_MAX,
                          "%.2f rad/s (limit %.0f rad/s)" % (rp, RIPPLE_MAX)))

    duty = nom["duty"] + noi["duty"]
    results.append(report("duty range", min(duty) >= 0.0 and max(duty) <= 1.0,
                          "min %.3f, max %.3f (must stay in 0..1)" % (min(duty), max(duty))))

    exec_s = INSTRUCTIONS * CPI / CLOCK_HZ
    results.append(report("loop time estimate", exec_s < PERIOD_S,
                          "%.1f us (period %.0f us)" % (exec_s * 1e6, PERIOD_S * 1e6)))

    sys.exit(0 if all(results) else 1)


if __name__ == "__main__":
    main()
