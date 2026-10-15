# Speakable_vitis

- `speakable_cnn_hls/`: HLS 컴포넌트 (csim·합성)
- `speakable_platform/`: Vitis 플랫폼 (Vivado xsa)
- `app_component/`: 보드 검증 앱. `speakable_arm_ref/src`의 검증 프로그램과 `speakable_arm_ref/generated/` 헤더를
  직접 빌드한다 (`src/UserConfig.cmake`)

Vivado에서 xsa가 바뀌었거나 모델 헤더를 교체한 뒤 (Vitis IDE가 이 작업 공간을 열고 있으면 실패하므로 IDE를 닫고):

```
vitis -s rebuild_board_test.py [fpga|frontend]
```

- `fpga` (기본): FPGA 가속 추론 레이어별 비트 비교 + ARM 단독 대비 시간 (`speakable_fpga_test.c`)
- `frontend`: PCM → 전처리 → CNN(ARM·FPGA), 단계별 시간 (`speakable_frontend_test.c`, `SPK_ON_BOARD`)

플랫폼 하드웨어 갱신 → 플랫폼 빌드 → 앱 실행용 비트스트림·ps7_init 갱신 → 앱 빌드 (`app_component/build/app_component.elf`).
하드웨어는 그대로이고 검증 프로그램만 바꿀 때는 IDE에서 앱만 빌드해도 된다 (스크립트를 한 번 실행해 UserConfig.cmake를 바꾼 뒤).
