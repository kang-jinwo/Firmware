# M0. 환경 준비

**목표**: 내 컴퓨터에서 C 프로그램을 컴파일하고, 이 프로젝트를 한 번 돌려 본다.
**끝났다는 기준**: `hello` 가 출력되고, `python run.py` 가 끝까지 돈다.

(클라우드에서는 이미 확인했다: gcc 13.3, Python 3.13. 여기서는 내 컴퓨터를 맞춘다.)

## 1. 필요한 것 세 가지

| 도구 | 확인 명령 | 없으면 |
|---|---|---|
| git | `git --version` | https://git-scm.com 에서 설치 |
| gcc | `gcc --version` | 아래 2절 |
| Python 3 | `python --version` | https://www.python.org 에서 설치 (Windows는 설치 화면에서 "Add python.exe to PATH" 체크) |

## 2. gcc 설치

- **Windows**: MSYS2 (https://www.msys2.org) 를 설치하고, "MSYS2 UCRT64" 창에서
  `pacman -S mingw-w64-ucrt-x86_64-gcc` 를 실행한다.
  그다음 Windows 환경 변수 PATH 에 `C:\msys64\ucrt64\bin` 을 추가한다.
  새 명령 프롬프트를 열어 `gcc --version` 이 나오면 된다.
- **macOS**: 터미널에서 `xcode-select --install`
- **Linux**: `sudo apt install gcc`

## 3. 프로젝트 내려받기와 Python 패키지

```
git clone https://github.com/kang-jinwo/Implementation-and-Verification-of-Sensor-Motor-Control-Firmware-.git
cd Implementation-and-Verification-of-Sensor-Motor-Control-Firmware-
pip install numpy matplotlib
```

## 4. 확인

```
gcc -Wall -Wextra -o hello docs/m0/hello.c
./hello                 (Windows: hello.exe)
python run.py
```

마지막 명령 뒤에 `mode=open  final w=1185.42 rad/s ...` 같은 줄이 여러 개 나오고 `out/` 폴더에 CSV 파일이 생기면 성공이다.

`python` 명령이 없다고 나오면 `python3` 또는 `py` 로 바꿔 본다.

## 5. 생각해 볼 질문

**Q1.** `hello.c` 에서 `#include <stdio.h>` 를 지우면 왜 문제가 생길까?

<details><summary>▶ 답과 설명 보기</summary>

컴파일러는 파일을 위에서 아래로 읽는다. `printf` 를 만났을 때 "이 함수는 어떤 인자를 받고 무엇을 돌려주는가"를 알고 있어야 올바른 기계어를 만들 수 있다. `stdio.h` 안에 그 정보(함수 **선언**)가 들어 있다.

- **선언**: "이런 이름과 모양의 함수가 어딘가에 있다" (헤더 파일 `.h`)
- **정의**: 함수의 실제 몸통 (`.c` 파일, 또는 이미 컴파일된 라이브러리)

`printf` 의 몸통은 C 표준 라이브러리에 있고, 컴파일의 마지막 단계(링크)에서 내 프로그램에 연결된다.

이 구분이 이 프로젝트의 핵심 구조와 똑같다. `firmware/hal.h` 에는 `sensor_read()` 의 **선언**만 있고, 몸통은 `sim/main.c` 에 있다. 펌웨어는 선언만 보고 컴파일되고, 링크할 때 몸통과 이어진다. 그래서 몸통을 "가상 모터용"에서 "실제 보드용"으로 바꿔 끼울 수 있다 (M4에서 다시 나온다).
</details>

**Q2.** `main` 의 `return 0;` 에서 0은 누구에게 무엇을 알려 주는 값일까?

<details><summary>▶ 답과 설명 보기</summary>

프로그램을 실행시킨 쪽(운영체제, 또는 그 프로그램을 부른 다른 프로그램)에게 "정상적으로 끝났다"고 알려 주는 **종료 코드**다. 0이면 성공, 0이 아니면 실패라는 것이 약속이다.

이 프로젝트에서 실제로 쓰인다. `run.py` 는 테스트 프로그램을 실행한 뒤 종료 코드를 보고 통과/실패를 판단한다. 그래서 `tests/test_filter.c` 의 마지막 줄이 `return fail_count == 0 ? 0 : 1;` 이다. 화면에 PASS 라고 찍는 것은 사람을 위한 것이고, 종료 코드는 프로그램(자동화)을 위한 것이다.
</details>

**Q3.** `gcc -Wall -Wextra` 의 두 옵션은 왜 붙일까?

<details><summary>▶ 답과 설명 보기</summary>

경고(warning)를 더 많이 켜는 옵션이다. C는 "문법은 맞지만 아마 실수일 것"인 코드를 그냥 통과시킨다 (예: 값을 넣지 않은 변수를 사용, `if (x = 0)`). 경고는 그런 곳을 알려 준다.

펌웨어에서는 버그 하나가 기계를 망가뜨릴 수 있어서, 경고를 0개로 유지하는 것이 기본 습관이다. 이 프로젝트의 코드는 두 옵션을 켜고 경고 없이 컴파일된다. 내가 검증 코드를 쓸 때 경고가 나오면 무시하지 말고 읽어 본다.
</details>

## 다음

[`spec.md`](spec.md) (M1) 로 간다.
