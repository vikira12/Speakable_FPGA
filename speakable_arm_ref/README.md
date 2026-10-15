# speakable_arm_ref

스피커블 음성 전처리(PCM → Mel 입력 INT8)와 임베딩 CNN INT8 정수 추론의 ARM 기준 구현, 검증 도구.
FPGA 이식 시 이 구현이 기준값(골든 모델)이 된다.

```
src/        C 코드 (PC·보드 공용)
            speakable_frontend.c  전처리 (HPF, 이득, STFT, Mel, 64프레임 축소, log, 정규화, 양자화)
            speakable_model.c     CNN 정수 추론, L2 정규화
            speakable_match.c     대표 벡터 매칭, 3단계 판정, 혼동 표현 지정, 확인 기반 적응
            speakable_fpga.c      FPGA 가속 추론 (L1~L9 FPGA, GAP·FC ARM). 베어메탈(정적 버퍼 + 캐시 flush)과
                                  Linux(UIO: map0 레지스터, map1 예약 DDR 버퍼, 캐시 없는 매핑)
            *_test.c              테스트 벡터 비교 (SPK_ON_BOARD 정의 시 시간 측정)
sim/        PC 시뮬레이션용 드라이버·Xilinx 헤더 (speakable_fpga.c를 HLS C 모델과 연결해 PC에서 실행)
tools/      import_ai_artifacts.py AI 팀 산출물 → 아래 파일 규약으로 변환
            ref_model.py / ref_frontend.py  정수 추론·전처리 참조 구현 (numpy)
            export_c.py / export_frontend_c.py  C 헤더 생성 + 참조 구현 일치 검사
            make_dummy.py          검증용 더미 모델 생성
generated/  생성된 헤더 (현재 speakable_v02. 테스트 벡터 헤더는 AI Hub 음성 유래라 git 제외)
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

매칭 파라미터 `SPK_MATCH_PARAMS`(`src/speakable_match.c`)는 학습 산출물의 `matching_params.json`(검증셋 보정값)으로 정한다.
모델을 바꾸면 같이 교체한다. 단위 테스트(`speakable_match_test.c`)는 판정 규칙 시험용 고정값 `TEST_PARAMS`를 쓴다.

FPGA 런타임 PC 시뮬레이션 (보드 없이 `speakable_fpga.c`의 버퍼 배치·파라미터 변환을 확인): `sim/`의 가짜 드라이버가
레지스터 값을 받아 HLS C 모델 `speakable_cnn_top()`을 호출한다. 보드용 `speakable_fpga_test.c`를 그대로 빌드한다
(g++ = 같은 mingw 폴더, `<Vitis>/include` = ap_int 헤더)

```bash
g++ -O2 -Isim -I../speakable_hls_src -I<Vitis>/include -c sim/speakable_cnn_sim.cpp ../speakable_hls_src/speakable_cnn.cpp
```
```bash
gcc -std=c99 -O2 -Isim -Isrc -Igenerated -c src/speakable_fpga.c src/speakable_fpga_test.c src/speakable_model.c
```
```bash
g++ speakable_fpga_test.o speakable_fpga.o speakable_model.o speakable_cnn_sim.o speakable_cnn.o -o fpga_sim
```

버그 주입으로 확인한 것: shift(int8)를 int32 배열로 변환하지 않고 복사 / 결과 꺼내기(unpad)의 채널 간격 오류 → 모두 FAIL.
mult에 1을 더하는 정도의 미세한 오차는 반올림 경계에 걸릴 때만 드러나 테스트 벡터 5개로는 잡히지 않는다 (재양자화 경계값 테스트가 담당).

보드(베어메탈): `Speakable_vitis/rebuild_board_test.py [fpga|frontend]` (Speakable_vitis/README 참고).

보드(PetaLinux): 같은 소스를 arm-linux-gnueabihf-gcc로 정적 링크 (`-static`, 드라이버 `xspeakable_cnn_top.c`,
`xspeakable_cnn_top_linux.c` 포함). UIO 장치 이름은 커널에 따라 `speakable_cnn_top@40000000`처럼 주소가 붙으므로
런타임이 `speakable_cnn_top`으로 시작하는 장치를 찾아 그 이름을 드라이버에 넘긴다. UIO 접근은 root 권한 필요.
