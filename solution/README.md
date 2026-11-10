# solution/ — 검증 코드 정답

**먼저 `tests/` 와 `tools/` 에서 직접 써 본 뒤에 연다.**
막혔을 때는 파일 전체를 보지 말고, 막힌 함수 하나만 본다.

| 파일 | 대응하는 내 파일 | 단계 |
|---|---|---|
| `plot.py` | `tools/plot.py` | M3부터 |
| `eig.py` | `tools/eig.py` | M5 (고유값 그림) |
| `test_filter.c` | `tests/test_filter.c` | M6, M7 |
| `test_controller.c` | `tests/test_controller.c` | M6 |
| `check_spec.py` | `tests/check_spec.py` | M6 |
| `make_figures.py` | — | `figures/` 의 참고 그래프를 다시 만든다 |
| `figures/` | — | 내 그래프와 비교할 참고 그래프 |

실행: `python run.py test --solution`
