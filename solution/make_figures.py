#!/usr/bin/env python3
"""make_figures.py — solution/figures/ 의 참고 그래프를 다시 만든다.

먼저 `python run.py all` 로 out/ 의 CSV를 만든 뒤 실행한다:
    python solution/make_figures.py
"""
import os
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from plot import plot_files  # noqa: E402

FIG = os.path.join(HERE, "figures")
os.makedirs(FIG, exist_ok=True)


def fig(name, files, cols, tmin=None, tmax=None):
    paths = [os.path.join("out", f + ".csv") for f in files]
    plot_files(paths, cols, tmin, tmax, save=os.path.join(FIG, name + ".png"), title=name)


fig("m2_open12", ["m2_open12"], ["w", "i"])
fig("m3_pwm_vs_dc", ["m3_pwm50", "m3_dc6"], ["w", "i"])
fig("m3_pwm_zoom", ["m3_pwm50_zoom"], ["v", "i"], 0.004, 0.0043)
fig("m4_p_control", ["m4_p_kp0.001", "m4_p_kp0.002", "m4_p_kp0.005", "m4_p_kp0.01"], ["w", "duty"])
fig("m5_pi", ["m4_p_kp0.002", "m5_pi"], ["w", "duty"])
fig("p1_ki", ["p1_ki0.02", "p1_ki0.05", "p1_ki0.2", "p1_ki2"], ["w", "duty"])
fig("p1_kp", ["p1_ki0.02", "p1_kp0.3"], ["w", "duty", "i"], 0.0, 0.3)
fig("p2_noise", ["p2_noise20", "p2_noise0"], ["w_meas", "w", "duty"])
fig("p3_ma", ["p3_ma1", "p3_ma8", "p3_ma32", "p3_ma64"], ["w_filt", "w", "duty"])
fig("p4_period", ["p4_fast_1ms", "p4_fast_10ms", "p4_fast_20ms", "p4_base_20ms"], ["w", "duty"])
fig("p5_integer", ["p5_float", "p5_int_q8", "p5_int_q6", "p5_int_q4", "p5_int_q8_acc16"], ["w", "duty"])
fig("p6_windup", ["p6_windup", "p6_antiwindup"], ["w", "duty"])
fig("p7_onoff", ["p7_hyst0", "p7_hyst5", "p7_hyst20"], ["w", "duty"], 0.3, 0.4)
fig("p8_overrun", ["p8_exec0", "p8_exec900", "p8_exec1500", "p8_exec5000", "p8_fast_exec10000"], ["w", "duty"])

subprocess.call([sys.executable, os.path.join(HERE, "eig.py"), "--save", os.path.join(FIG, "m5_eigenvalues.png")])
