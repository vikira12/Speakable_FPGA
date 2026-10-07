# speakable_arm_ref

스피커블 음성 전처리(PCM → Mel 입력 INT8)와 임베딩 CNN INT8 정수 추론의 ARM 기준 구현, 검증 도구.
FPGA 이식 시 이 구현이 기준값(골든 모델)이 된다.

```
src/        C 코드 (PC·보드 공용)
            speakable_frontend.c  전처리 (HPF, 이득, STFT, Mel, 64프레임 축소, log, 정규화, 양자화)
            speakable_model.c     CNN 정수 추론, L2 정규화
            speakable_match.c     대표 벡터 매칭, 3단계 판정, 혼동 표현 지정, 확인 기반 적응
            *_test.c              테스트 벡터 비교 (SPK_ON_BOARD 정의 시 시간 측정)
tools/      import_ai_artifacts.py AI 팀 산출물 → 아래 파일 규약으로 변환
            ref_model.py / ref_frontend.py  정수 추론·전처리 참조 구현 (numpy)
            export_c.py / export_frontend_c.py  C 헤더 생성 + 참조 구현 일치 검사
            make_dummy.py          검증용 더미 모델 생성
generated/  생성된 헤더 (현재 model_pilot_v01)
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

## 정수 규약 (model_pilot_v01 테스트 벡터 20개로 확인)

- 재양자화: `out = sat8( round_away( acc × mult / 2^shift ) )`, acc×mult는 INT64
- GAP: `sat8( round_away( 합 / (H×W) ) )`
- FC 출력: INT8 재양자화 (활성화 없음), L2 정규화는 float
- 입력 양자화: 정규화 값을 float32로 거친 뒤 `/ input_scale`, 0에서 먼 방향 반올림

## 사용법

새 모델 반영 (AI 팀 디렉터리 형식 그대로)
```bash
python tools/import_ai_artifacts.py <AI 모델 디렉터리> imported/<이름>
```
```bash
python tools/export_c.py imported/<이름> imported/<이름>/tv generated 5
```
```bash
python tools/export_frontend_c.py <AI 모델 디렉터리> generated 5
```

PC 검증
```bash
gcc -std=c99 -O2 -Isrc -Igenerated src/speakable_model.c src/speakable_test.c -lm -o spk_test
```
```bash
gcc -std=c99 -O2 -Isrc -Igenerated src/speakable_frontend.c src/speakable_model.c src/speakable_frontend_test.c -lm -o fe_test
```

```bash
gcc -std=c99 -O2 -Isrc src/speakable_match.c src/speakable_match_test.c -lm -o match_test
```

매칭 파라미터(`SPK_MATCH_PARAMS_PLACEHOLDER`)는 AI 팀 보정 전 임시값이다. Threshold·Margin·즉시 출력 기준이 확정되면 교체한다.

보드: Vitis 앱에 `src/`의 `.c/.h`(테스트 main은 하나만)와 `generated/*.h`를 넣고 `-DSPK_ON_BOARD`를 추가하면 단계별 시간도 출력된다.
