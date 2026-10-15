#ifndef SPEAKABLE_FPGA_H
#define SPEAKABLE_FPGA_H

#include <stdint.h>
#include "speakable_model.h"

// ----------------------------------------------------------------------
// FPGA 가속 추론 (보드 전용, speakable_cnn_top IP 사용)
//
//  L1~L9 (Conv/DW/PW)  : FPGA. 레이어마다 레지스터 설정 → 시작 → 완료 대기
//                        특징맵은 DDR 버퍼 두 개를 번갈아 사용 (CPU를 거치지 않음)
//  L10~L11 (GAP, FC)   : ARM (speakable_model.c의 spk_run_layer)
//
// 가중치는 spk_fpga_init에서 한 번만 IP의 DDR 배치(출력 채널마다 576바이트 칸)로 옮겨 둠
// 결과는 spk_cnn_run과 비트 단위로 같아야 함
// ----------------------------------------------------------------------

#define SPK_FPGA_LAYERS 9

// 0: 성공, −1: IP(레지스터) 초기화 실패 (Linux: 이름이 speakable_cnn_top인 UIO 장치 없음)
// Linux 전용: −2: DDR 버퍼(UIO map1) 매핑 실패, −3: 버퍼 영역이 필요한 크기보다 작음
int spk_fpga_init(uintptr_t base_addr);

// hook: 레이어마다 출력([C][H][W], 패딩 없이)을 받아 검증할 때 사용 (필요 없으면 NULL)
//       hook을 주면 레이어마다 결과를 캐시 무효화 후 꺼내므로 시간이 늘어남
void spk_fpga_cnn_run(const int8_t *input, int8_t *embedding, spk_layer_hook_t hook, void *ctx);

#endif
