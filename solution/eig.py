#!/usr/bin/env python3
"""eig.py (정답) — 닫힌 루프의 상태행렬 A를 만들고 고유값을 구한다 (3단계: 선형대수).

사용법:
    python solution/eig.py                 표를 출력
    python solution/eig.py --save eig.png  고유값이 움직이는 그림도 저장

상태 x = [i, w, z]  (z = 적분 항, 듀티 단위)
    v  = Vs · (kp·(목표 − w) + z)
    i' = (v − R·i − Ke·w) / L
    w' = (Kt·i − b·w) / J
    z' = ki · (목표 − w)
목표는 상수라서 안정성에는 영향이 없다 → x' = A x 의 A만 본다.
"""
import sys

import numpy as np

Vs, R, L, Kt, Ke, J, b = 12.0, 1.0, 0.5e-3, 0.01, 0.01, 1e-5, 1e-6


def open_loop_A():
    return np.array([[-R / L, -Ke / L],
                     [Kt / J, -b / J]])


def closed_loop_A(kp, ki):
    return np.array([[-R / L, -(Ke + Vs * kp) / L, Vs / L],
                     [Kt / J, -b / J,              0.0],
                     [0.0,    -ki,                 0.0]])


def describe(eigs):
    """고유값 목록을 사람이 읽을 문장으로."""
    worst = max(e.real for e in eigs)
    osc = [e for e in eigs if abs(e.imag) > 1e-9]
    if worst >= 0:
        return "UNSTABLE (real part >= 0)"
    if osc:
        e = max(osc, key=lambda x: x.real)
        zeta = -e.real / abs(e)
        return "oscillates: %.1f Hz, damping ratio %.2f" % (abs(e.imag) / (2 * np.pi), zeta)
    return "no oscillation, slowest time constant %.1f ms" % (-1000.0 / worst)


def main():
    print("open loop (no control):")
    for e in np.linalg.eigvals(open_loop_A()):
        print("   lambda = %10.1f   -> time constant %.3f ms" % (e.real, -1000.0 / e.real))

    print("\nclosed loop, kp = 0.002:")
    kis = [0.02, 0.05, 0.2, 2.0]
    for ki in kis:
        eigs = np.linalg.eigvals(closed_loop_A(0.002, ki))
        text = ", ".join("%.1f%+.1fj" % (e.real, e.imag) for e in eigs)
        print("   ki = %-5g  [%s]\n              %s" % (ki, text, describe(eigs)))

    if "--save" in sys.argv:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt

        fig, ax = plt.subplots(figsize=(6, 5))
        sweep = np.logspace(-3, 0.5, 200)
        pts = np.array([np.linalg.eigvals(closed_loop_A(0.002, ki)) for ki in sweep]).ravel()
        ax.plot(pts.real, pts.imag, ".", markersize=2, color="0.6")
        for ki in kis:
            e = np.linalg.eigvals(closed_loop_A(0.002, ki))
            ax.plot(e.real, e.imag, "o", label="ki = %g" % ki)
        ax.axvline(0, color="k", linewidth=0.8)
        ax.set_xlim(-60, 10)
        ax.set_ylim(-160, 160)
        ax.set_xlabel("real part [1/s]  (left = decays faster)")
        ax.set_ylabel("imaginary part [rad/s]  (away from 0 = oscillates)")
        ax.set_title("closed-loop eigenvalues, kp = 0.002 (fast electrical one is off-screen)")
        ax.grid(True, alpha=0.3)
        ax.legend()
        fig.tight_layout()
        out = sys.argv[sys.argv.index("--save") + 1]
        fig.savefig(out, dpi=80)
        print("saved", out)


if __name__ == "__main__":
    main()
