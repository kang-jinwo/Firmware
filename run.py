#!/usr/bin/env python3
"""run.py — 컴파일 → 실행 → 테스트를 한 번에.

사용법 (프로젝트 폴더에서):
    python run.py              빌드하고 기본 실험(M2~M5)을 돌린다
    python run.py build        빌드만
    python run.py M5           실험 하나만 (M2 M3 M4 M5 M7 P1 P2 ... P8 spec)
    python run.py all          모든 실험
    python run.py list         실험 목록과 각 실험의 옵션을 보여 준다
    python run.py test         tests/ 에 내가 쓴 검증 코드를 돌린다
    python run.py test --solution   solution/ 의 정답 검증 코드를 돌린다
    python run.py lab a1_memory_map   심화 실습(advanced/labs/) 하나를 컴파일해서 실행

결과 CSV는 out/ 폴더에 생긴다.
"""
import glob
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
EXE = ".exe" if os.name == "nt" else ""
BUILD = os.path.join(ROOT, "build")
OUT = os.path.join(ROOT, "out")
SIM = os.path.join(BUILD, "sim" + EXE)

CFLAGS = ["-std=c99", "-Wall", "-Wextra", "-O2"]

# 기준 이득: 사양서(docs/spec.md)에서 정한 값
BASE = ["--kp", "0.002", "--ki", "0.02", "--target", "400"]
FAST = ["--kp", "0.01", "--ki", "0.1", "--target", "400"]

# 실험 이름 → [(CSV 이름, 시뮬레이터 옵션), ...]
EXPERIMENTS = {
    "M2": [
        ("m2_open12", ["--mode", "open", "--volt", "12", "--tend", "0.6"]),
    ],
    "M3": [
        ("m3_pwm50", ["--mode", "pwm", "--duty", "0.5", "--tend", "0.6"]),
        ("m3_dc6", ["--mode", "open", "--volt", "6", "--tend", "0.6"]),
        ("m3_pwm50_zoom", ["--mode", "pwm", "--duty", "0.5", "--tend", "0.005", "--log_us", "1"]),
    ],
    "M4": [
        ("m4_p_kp0.001", ["--mode", "p", "--kp", "0.001", "--target", "400"]),
        ("m4_p_kp0.002", ["--mode", "p", "--kp", "0.002", "--target", "400"]),
        ("m4_p_kp0.005", ["--mode", "p", "--kp", "0.005", "--target", "400"]),
        ("m4_p_kp0.01", ["--mode", "p", "--kp", "0.01", "--target", "400"]),
    ],
    "M5": [
        ("m5_pi", ["--mode", "pi"] + BASE),
    ],
    "P1": [
        ("p1_ki0.02", ["--mode", "pi", "--kp", "0.002", "--ki", "0.02"]),
        ("p1_ki0.05", ["--mode", "pi", "--kp", "0.002", "--ki", "0.05"]),
        ("p1_ki0.2", ["--mode", "pi", "--kp", "0.002", "--ki", "0.2"]),
        ("p1_ki2", ["--mode", "pi", "--kp", "0.002", "--ki", "2"]),
        ("p1_kp0.3", ["--mode", "pi", "--kp", "0.3", "--ki", "0.2"]),
    ],
    "P2": [
        ("p2_noise0", ["--mode", "pi", "--noise", "0", "--tend", "1.0"] + BASE),
        ("p2_noise20", ["--mode", "pi", "--noise", "20", "--tend", "1.0"] + BASE),
    ],
    "P3": [
        ("p3_ma1", ["--mode", "pi", "--noise", "20", "--ma", "1", "--tend", "1.0"] + BASE),
        ("p3_ma8", ["--mode", "pi", "--noise", "20", "--ma", "8", "--tend", "1.0"] + BASE),
        ("p3_ma32", ["--mode", "pi", "--noise", "20", "--ma", "32", "--tend", "1.0"] + BASE),
        ("p3_ma64", ["--mode", "pi", "--noise", "20", "--ma", "64", "--tend", "1.0"] + BASE),
    ],
    "P4": [
        ("p4_base_1ms", ["--mode", "pi", "--period_ms", "1", "--tend", "1.0"] + BASE),
        ("p4_base_20ms", ["--mode", "pi", "--period_ms", "20", "--tend", "1.0"] + BASE),
        ("p4_fast_1ms", ["--mode", "pi", "--period_ms", "1", "--tend", "1.0"] + FAST),
        ("p4_fast_10ms", ["--mode", "pi", "--period_ms", "10", "--tend", "1.0"] + FAST),
        ("p4_fast_20ms", ["--mode", "pi", "--period_ms", "20", "--tend", "1.0"] + FAST),
    ],
    "P5": [
        ("p5_float", ["--mode", "pi"] + BASE),
        ("p5_int_q8", ["--mode", "pi_int", "--qshift", "8"] + BASE),
        ("p5_int_q6", ["--mode", "pi_int", "--qshift", "6"] + BASE),
        ("p5_int_q4", ["--mode", "pi_int", "--qshift", "4"] + BASE),
        ("p5_int_q8_acc16", ["--mode", "pi_int", "--qshift", "8", "--acc16", "1"] + BASE),
        ("p5_int_q6_acc16", ["--mode", "pi_int", "--qshift", "6", "--acc16", "1"] + BASE),
    ],
    "P6": [
        ("p6_windup", ["--mode", "pi", "--kp", "0.002", "--ki", "0.02", "--target", "1500",
                       "--target2", "400", "--t2", "1.0", "--tend", "2.5", "--aw", "0"]),
        ("p6_antiwindup", ["--mode", "pi", "--kp", "0.002", "--ki", "0.02", "--target", "1500",
                           "--target2", "400", "--t2", "1.0", "--tend", "2.5", "--aw", "1"]),
    ],
    "P7": [
        ("p7_hyst0", ["--mode", "onoff", "--hyst", "0", "--noise", "5", "--target", "400"]),
        ("p7_hyst5", ["--mode", "onoff", "--hyst", "5", "--noise", "5", "--target", "400"]),
        ("p7_hyst20", ["--mode", "onoff", "--hyst", "20", "--noise", "5", "--target", "400"]),
    ],
    "P8": [
        ("p8_exec0", ["--mode", "pi", "--exec_us", "0", "--tend", "1.0"] + BASE),
        ("p8_exec900", ["--mode", "pi", "--exec_us", "900", "--tend", "1.0"] + BASE),
        ("p8_exec1500", ["--mode", "pi", "--exec_us", "1500", "--tend", "1.0"] + BASE),
        ("p8_exec5000", ["--mode", "pi", "--exec_us", "5000", "--tend", "1.0"] + BASE),
        ("p8_fast_exec10000", ["--mode", "pi", "--exec_us", "10000", "--tend", "1.0"] + FAST),
    ],
    # 검증(M6)에 쓰는 실행 두 개
    "spec": [
        ("spec_nominal", ["--mode", "pi", "--tend", "1.0"] + BASE),
        ("spec_noise", ["--mode", "pi", "--noise", "20", "--ma", "8", "--tend", "1.0"] + BASE),
    ],
}
DEFAULT = ["M2", "M3", "M4", "M5"]


def sh(cmd):
    """명령 하나를 실행한다. 실패하면 False."""
    return subprocess.call(cmd, cwd=ROOT) == 0


def build():
    os.makedirs(BUILD, exist_ok=True)
    os.makedirs(OUT, exist_ok=True)
    sources = (
        ["sim/main.c"]
        + sorted(glob.glob(os.path.join(ROOT, "plant", "*.c")))
        + sorted(glob.glob(os.path.join(ROOT, "firmware", "*.c")))
    )
    cmd = ["gcc"] + CFLAGS + ["-o", SIM] + sources + ["-lm"]
    print("[build] gcc ... -o build/sim")
    if not sh(cmd):
        sys.exit("build failed")


def run_experiment(name):
    for csv_name, opts in EXPERIMENTS[name]:
        out = os.path.join("out", csv_name + ".csv")
        print("[%s] sim %s" % (name, " ".join(opts)))
        if not sh([SIM] + opts + ["--out", out]):
            sys.exit("simulation failed")


def run_tests(use_solution):
    folder = "solution" if use_solution else "tests"
    failed = 0

    # 1) C 단위 테스트: test_*.c 를 하나씩 컴파일해서 실행. 종료 코드 0이면 통과.
    for src in sorted(glob.glob(os.path.join(ROOT, folder, "test_*.c"))):
        name = os.path.splitext(os.path.basename(src))[0]
        exe = os.path.join(BUILD, name + EXE)
        cmd = ["gcc"] + CFLAGS + ["-o", exe, src, "firmware/filter.c", "firmware/controller.c", "-lm"]
        print("\n[test] %s/%s.c" % (folder, name))
        if not sh(cmd) or not sh([exe]):
            failed += 1

    # 2) 사양 검증: check_spec.py 가 CSV를 읽어 PASS/FAIL을 출력. 종료 코드 0이면 통과.
    check = os.path.join(ROOT, folder, "check_spec.py")
    if os.path.exists(check):
        run_experiment("spec")
        print("\n[test] %s/check_spec.py" % folder)
        cmd = [sys.executable, check, "out/spec_nominal.csv", "out/spec_noise.csv"]
        if not sh(cmd):
            failed += 1

    print("\n==== %s ====" % ("ALL TESTS PASSED" if failed == 0 else "%d TEST PROGRAM(S) FAILED" % failed))
    return failed == 0


def run_lab(name):
    """advanced/labs/<name>.c 를 최적화 없이(-O0) 컴파일해서 실행한다."""
    src = os.path.join("advanced", "labs", name + ".c")
    if not os.path.exists(os.path.join(ROOT, src)):
        labs = sorted(os.path.basename(p)[:-2] for p in glob.glob(os.path.join(ROOT, "advanced", "labs", "*.c")))
        sys.exit("no such lab: %s\navailable: %s" % (name, ", ".join(labs)))
    os.makedirs(BUILD, exist_ok=True)
    exe = os.path.join(BUILD, name + EXE)
    if not sh(["gcc", "-std=c11", "-Wall", "-Wextra", "-O0", "-pthread", "-o", exe, src]):
        sys.exit("build failed")
    sh([exe])


def main():
    args = sys.argv[1:]
    what = args[0] if args else "default"

    if what == "lab":
        run_lab(args[1] if len(args) > 1 else "")
        return

    if what == "list":
        for name, runs in EXPERIMENTS.items():
            print(name)
            for csv_name, opts in runs:
                print("   out/%s.csv   <-  %s" % (csv_name, " ".join(opts)))
        return

    build()
    if what == "build":
        return
    if what == "test":
        ok = run_tests("--solution" in args)
        sys.exit(0 if ok else 1)

    if what == "default":
        names = DEFAULT
    elif what == "all":
        names = list(EXPERIMENTS)
    elif what in EXPERIMENTS:
        names = [what]
    else:
        sys.exit("unknown command: %s  (try: python run.py list)" % what)

    for name in names:
        run_experiment(name)


if __name__ == "__main__":
    main()
