# speakable_arm_ref

스피커블 음성 임베딩 CNN의 INT8 정수 추론 ARM 기준 구현과 검증 도구.
FPGA 이식 시 이 구현이 기준값(골든 모델)이 된다.

```
src/        추론 C 코드 (PC·보드 공용)
tools/      ref_model.py  정수 추론 참조 구현 (numpy)
            export_c.py   npy → C 헤더 변환 + 참조 구현 일치 검사
            make_dummy.py 실제 가중치 전 검증용 더미 모델 생성
generated/  변환된 헤더 (현재는 더미 모델)
```

## AI 팀 전달 파일 규약

모델 디렉터리 (n = 레이어 번호 1~11, L10 GAP은 파라미터 없음)

| 파일 | dtype | shape |
|---|---|---|
| `L{n}_weight.npy` | int8 | L1 `(16,1,3,3)`, DW `(C,1,3,3)`, PW `(OC,IC,1,1)`, FC `(64,64)` |
| `L{n}_bias.npy` | int32 | `(OC,)` |
| `L{n}_mult.npy` | int32 | `(OC,)` |
| `L{n}_shift.npy` | int | `(OC,)` |

테스트 벡터 디렉터리 (k = 0, 1, ...)

| 파일 | dtype | shape |
|---|---|---|
| `tv{k}_input.npy` | int8 | `(1,40,64)` (양자화된 Mel 입력) |
| `tv{k}_L{n}.npy` | int8 | 레이어 n 출력 `(C,H,W)`, L10·L11은 `(64,)` |

## 확정이 필요한 정수 규약 (현재 가정)

- 재양자화: `out = sat8( round_away( acc × mult / 2^shift ) )` — `src/speakable_model.c`의 `spk_requant`, `tools/ref_model.py`의 `requant`
- GAP: `sat8( round_away( 합 / (H×W) ) )`
- FC 출력: 다른 레이어와 같은 방식으로 INT8 재양자화 (활성화 없음), L2 정규화는 float
- Mel(실수) → INT8 입력 양자화 scale과 반올림

## 사용법

```bash
python tools/export_c.py <모델 디렉터리> <테스트 벡터 디렉터리> generated
```
```bash
gcc -std=c99 -O2 -Isrc -Igenerated src/speakable_model.c src/speakable_test.c -lm -o spk_test
```

보드: Vitis 앱 `src/`에 `src/*.c, *.h`와 `generated/*.h`를 넣고 컴파일 옵션에 `-DSPK_ON_BOARD`를 추가하면 추론 시간도 출력된다.
