# Speakable_vitis

- `speakable_cnn_hls/`: HLS 컴포넌트 (csim·합성)
- `speakable_platform/`: Vitis 플랫폼 (Vivado xsa)
- `app_component/`: 보드 검증 앱. `speakable_arm_ref/src`의 FPGA 가속 추론 검증 프로그램(`speakable_fpga_test.c`)과
  `speakable_arm_ref/generated/` 헤더를 직접 빌드한다 (`src/UserConfig.cmake`)

Vivado에서 xsa가 바뀌었거나 모델 헤더를 교체한 뒤:

```
vitis -s rebuild_board_test.py
```

플랫폼 하드웨어 갱신 → 플랫폼 빌드 → 앱 실행용 비트스트림·ps7_init 갱신 → 앱 빌드 (`app_component/build/app_component.elf`).
