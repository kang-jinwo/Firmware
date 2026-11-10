#!/usr/bin/env python3
"""plot.py (정답) — CSV 여러 개를 겹쳐 그린다.

사용법:
    python solution/plot.py out/m5_pi.csv
    python solution/plot.py out/m3_pwm50.csv out/m3_dc6.csv --cols w i
    python solution/plot.py out/m3_pwm50_zoom.csv --cols v i --tmin 0.004 --tmax 0.0042
    python solution/plot.py out/p1_ki0.02.csv out/p1_ki0.2.csv --cols w duty --save p1.png

--cols 뒤에 그릴 열 이름을 적는다 (t,v,i,w,w_meas,w_filt,duty,target 중에서). 열 하나당 그래프 한 칸.
"""
import csv
import os
import sys

import matplotlib
if "--save" in sys.argv:
    matplotlib.use("Agg")          # 화면 없이 파일로만 저장
import matplotlib.pyplot as plt


def load(filename):
    """CSV를 읽어 {열 이름: [값, ...]} 사전으로 돌려준다."""
    cols = {}
    with open(filename, newline="") as f:
        for row in csv.DictReader(f):
            for name, text in row.items():
                cols.setdefault(name, []).append(float(text))
    return cols


def plot_files(files, columns, tmin=None, tmax=None, save=None, title=None):
    fig, axes = plt.subplots(len(columns), 1, sharex=True, figsize=(8, 2.6 * len(columns)))
    if len(columns) == 1:
        axes = [axes]                       # 한 칸일 때도 리스트처럼 다루기 위해

    for filename in files:
        data = load(filename)
        t = data["t"]
        label = os.path.splitext(os.path.basename(filename))[0]
        for ax, col in zip(axes, columns):
            ax.plot(t, data[col], label=label, linewidth=1.0)
            if col == "w" and max(data["target"]) > 0:
                ax.plot(t, data["target"], "k--", linewidth=0.6)

    for ax, col in zip(axes, columns):
        ax.set_ylabel(col)
        ax.grid(True, alpha=0.3)
        if tmin is not None or tmax is not None:
            ax.set_xlim(tmin, tmax)
    axes[0].legend(fontsize=8)
    axes[-1].set_xlabel("t [s]")
    if title:
        axes[0].set_title(title)
    fig.tight_layout()

    if save:
        fig.savefig(save, dpi=80)
        plt.close(fig)
        print("saved", save)
    else:
        plt.show()


def main():
    files, columns = [], []
    tmin = tmax = save = None
    args = sys.argv[1:]
    k = 0
    while k < len(args):
        a = args[k]
        if a == "--cols":
            k += 1
            while k < len(args) and not args[k].startswith("--"):
                columns.append(args[k])
                k += 1
            continue
        if a == "--tmin":
            tmin = float(args[k + 1]); k += 2; continue
        if a == "--tmax":
            tmax = float(args[k + 1]); k += 2; continue
        if a == "--save":
            save = args[k + 1]; k += 2; continue
        files.append(a)
        k += 1

    if not files:
        sys.exit(__doc__)
    plot_files(files, columns or ["w", "duty"], tmin, tmax, save)


if __name__ == "__main__":
    main()
