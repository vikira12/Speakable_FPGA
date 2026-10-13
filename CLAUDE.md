# Speakable FPGA — 작업 안내 (세션 인수인계)

스피커블(구음장애 사용자용 온디바이스 음성 의사소통 보조기기, 2026 빛가람 AI·ICT 경진대회)의 FPGA·임베디드 파트.
보드: Digilent Zybo Z7-20 (xc7z020clg400-1). 도구: Vitis/Vivado 2025.2 (`C:/AMDDesignTools/2025.2`).
담당: 손서원(팀장, FPGA·임베디드). 팀: 웹(장윤호), 서버(남우현, Java/Spring), AI(이시우), 디자인(정일웅).

## 처리 구조

```
wav(PCM16 16kHz) → [ARM] 전처리 → 입력 INT8 1×40×64 → [FPGA] CNN L1~L9 → [ARM] GAP·FC·L2 → [ARM] 매칭·3단계 판정
```

## 폴더

| 폴더 | 내용 |
|---|---|
| `speakable_hls_src/` | HLS 가속기 `speakable_cnn_top` (Conv3×3/DW3×3/PW1×1, stride 1·2, 채널별 재양자화, ReLU, 2×2 pool) + 테스트벤치 |
| `speakable_cnn_ip/` | 패키징된 IP (`xilinx.com:hls:speakable_cnn_top:1.0`), 드라이버 `XSpeakable_cnn_top_*` |
| `Speakable_vitis/speakable_cnn_hls/` | HLS 컴포넌트 (`hls_config.cfg`, work_dir `speakable_cnn_top`) |
| `Speakable_vitis/speakable_platform/`, `app_component/` | Vitis 플랫폼, 보드 앱 (app_component의 main.c는 **구 IP용 Conv 테스트라 현재 IP와 맞지 않음**) |
| `Speakable_vivado/speakable_vivado/` | Vivado 프로젝트, BD `speakable_bd`, 셀 `speakable_cnn_0`, `speakable_bd_wrapper.xsa` |
| `speakable_arm_ref/` | ARM C 구현(전처리·CNN·매칭·FPGA 런타임), Python 참조 구현, 헤더 생성 도구. 상세는 그 안의 README |
| `_backup_v1/` | 재설계 전 원본 (이름에 dpu가 남아 있는 것은 여기만) |

AI 팀 산출물: `D:/Speakable_artifacts/model_pilot_v01` (파일럿, 약한 라벨 20개, **정확도 미검증**), `inference_smoke_v01`.

## 현재 상태 (2026-10-13)

검증 완료
- HLS csim 14/14: 모드별 13개 + 파일럿 모델 L1~L9를 하드웨어 함수로 연결 실행, AI 팀 정수 PyTorch 출력과 비트 일치. 버그 주입 3종 모두 검출
- Vivado 100MHz: WNS +0.537ns, LUT 27%, DSP 53%(117), BRAM 48%, 1.71W
- ARM C(PC): 전처리 20개 샘플 입력 INT8 완전 일치, CNN 20×11레이어 일치, 재양자화 경계값 86개, 매칭 단위 테스트 37개
- 보드 실측(구 Conv 전용 IP, 10-06): 64×64×64→64 Conv FPGA 29.2ms, ARM 단순 C(-O2) 대비 ×187.2

미완료
- **현재 IP(DW/PW 포함)로 보드 실측 안 함**: 플랫폼 하드웨어 갱신 → 새 앱에 `speakable_model.c/.h`, `speakable_fpga.c/.h`, `speakable_fpga_test.c`, `generated/speakable_weights.h`, `generated/speakable_testvec.h` → 실행
- ARM 전처리+CNN 보드 시간 측정 안 함 (`speakable_frontend_test.c`, `-DSPK_ON_BOARD`)
- 매칭 파라미터(Threshold·Margin·즉시 출력 기준)는 임시값 `SPK_MATCH_PARAMS_PLACEHOLDER`. AI 팀 보정 대기
- WAV 파싱, 서버 동기화, 오디오 입출력(코덱), GPIO 버튼, LCD/OLED, SD 부팅(BOOT.BIN) 미구현

## 결정 사항

- **전처리는 기기(ARM)에서만 한다.** 서버는 웹이 품질 검사한 원본 WAV(RIFF, PCM16 LE, mono, 16kHz, 8,000~48,000 샘플, 가공 없음)를 그대로 저장·전달한다. 서버가 소리를 일부만 가공(정규화·필터·무음 제거)하는 것은 금지.
- 기기는 등록 WAV(표현당 최대 5개)로 직접 임베딩을 계산한다. 매칭이 샘플별 max cos를 쓰므로 평균 벡터 하나로는 안 된다.
- 동기화는 JSON 매니페스트(모델 버전, 매칭 파라미터, 표현별 녹음·TTS URL과 sha256). 기기 → 서버는 인식 로그만(원본 음성·갱신 임베딩 제외).
- 이름에 "DPU"를 쓰지 않는다(Xilinx DPU와 혼동). 명칭은 speakable_*.
- 모델은 고치지 않고 파이프라인을 먼저 완성한 뒤, 재학습 가중치를 교체한다(`tools/import_ai_artifacts.py` → `tools/export_c.py`).

## 정수 규약 (하드웨어·ARM·Python·AI 팀 공통)

- 대칭 INT8, zero-point 0, bias·누산 INT32
- 재양자화: `sat8(round_away(acc × mult / 2^shift))`, acc×mult는 INT64, 출력 채널별 mult(양의 INT32)·shift(36~40)
- GAP: `sat8(round_away(합/(H×W)))`, FC 출력도 INT8 재양자화(활성화 없음), L2 정규화는 float
- 입력 양자화: 정규화 값을 float32로 한 번 거친 뒤 `/input_scale`, round_away

## 빌드·검증 명령 (소요 시간은 이 노트북 기준)

```
# HLS csim (약 2분), 합성+IP 패키징 (약 30분) — Speakable_vitis/speakable_cnn_hls에서
vitis-run --mode hls --csim --config hls_config.cfg --work_dir speakable_cnn_top
v++ -c --mode hls --config hls_config.cfg --work_dir speakable_cnn_top
# 새 IP 설치: speakable_cnn_top/hls/impl/ip → speakable_cnn_ip/ (pack.bat, run_ippack.tcl, zip, .Xil, vivado.* 제외)

# Vivado 빌드 (약 55~70분): Tcl 배치. 백그라운드 셸 시간 제한에 걸리므로 PowerShell Start-Process로 독립 실행 후 로그 폴링
vivado -mode batch -nojournal -source <script>.tcl -log <log>
#   open_project → update_ip_catalog -rebuild → upgrade_ip → validate/save → launch_runs impl_1 -to_step write_bitstream
#   → write_hw_platform -fixed -include_bit -force speakable_bd_wrapper.xsa

# ARM C PC 검증 (speakable_arm_ref에서, gcc = C:/AMDDesignTools/2025.2/Vitis/tps/mingw/10.0.0/win64.o/nt/bin/gcc.exe)
gcc -std=c99 -O2 -Isrc -Igenerated src/speakable_model.c src/speakable_test.c -lm -o spk_test
gcc -std=c99 -O2 -Isrc -Igenerated src/speakable_frontend.c src/speakable_model.c src/speakable_frontend_test.c -lm -o fe_test
gcc -std=c99 -O2 -Isrc src/speakable_match.c src/speakable_match_test.c -lm -o match_test

# 보드 코드 컴파일 확인 (보드 없이): arm-none-eabi-gcc ... -DSDT -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard
#   -specs=<platform>/export/.../standalone_ps7_cortexa9_0/Xilinx.spec -I<그 include> -fsyntax-only
```

## 주의사항 (실제로 겪은 것)

- BSP의 `COUNTS_PER_SECOND`는 괄호 없는 `CPU_FREQ/2` 매크로 → 반드시 `(double)(COUNTS_PER_SECOND)`로 감쌀 것 (안 하면 시간이 1/4)
- JTAG 실행은 글로벌 타이머가 꺼져 있음 → `Xil_Out32(XPAR_GLOBAL_TMR_BASEADDR + 0x08, 1)`. 타이머 헤더는 `xiltimer.h` (`xtime_l.h` 없음)
- 베어메탈 기본 스택 8KB → 큰 배열은 static
- 가속기 DDR 버퍼: CPU가 쓴 뒤 `Xil_DCacheFlushRange`, 가속기 결과 읽기 전 `Xil_DCacheInvalidateRange`, 64바이트 정렬
- 보드 JP5는 JTAG 위치 (아니면 `Could not find ARM device`)
- HLS: 포트 인자에 ARRAY_PARTITION 금지(AXI 마스터가 쪼개짐), 작은 버퍼는 BIND_STORAGE로 LUTRAM 고정, 누산 루프는 EXPRESSION_BALANCE off + 명시적 덧셈 트리
- HLS 타이밍·LUT 추정은 보수적(LUT 82% 추정 → 구현 27%). 판단은 Vivado 구현 리포트로
- ReLU가 음수를 지워 반올림 규칙 오류가 일반 테스트에서 안 드러남 → 경계값 전용 테스트 유지
- Vitis 앱 소스(`Speakable_vitis/app_component/src/main.c` 등)는 CRLF. sed로 고치면 LF가 되므로 python으로 CRLF 유지

## 작업 방식 (사용자 선호)

- 코드에 버전 이력 주석("v1은…")이나 작업 흔적을 남기지 않는다. 주석은 한국어, 기존 스타일을 따른다
- 커밋·push는 사용자가 직접 한다. 끝나면 확인을 요청받는다
- 테스트는 골든 모델과 비트 단위 비교 + 버그 주입으로 테스트 자체를 검증한다
- 보드 실행은 사용자가 하고 UART 출력을 붙여 준다
