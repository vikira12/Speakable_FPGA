#ifndef SPEAKABLE_MODEL_H
#define SPEAKABLE_MODEL_H

#include <stdint.h>

// ----------------------------------------------------------------------
// 스피커블 음성 임베딩 CNN (INT8 정수 추론, ARM 기준 구현)
//
// 입력 1×40×64 (C × Mel × 시간) → 64차원 임베딩
//
//  L1  Conv3×3 s2   1→16   16×20×32
//  L2  DW3×3   s1   16     16×20×32
//  L3  PW1×1        16→32  32×20×32
//  L4  DW3×3   s2   32     32×10×16
//  L5  PW1×1        32→64  64×10×16
//  L6  DW3×3   s1   64     64×10×16
//  L7  PW1×1        64→64  64×10×16
//  L8  DW3×3   s2   64     64×5×8
//  L9  PW1×1        64→64  64×5×8
//  L10 GAP                 64
//  L11 FC           64→64  64      (활성화 없음)
//
// 정수 산술 규약
//  - 가중치·활성값: 대칭 INT8 (zero-point = 0), bias·누산: INT32
//  - 재양자화: 출력 채널별 정수 multiplier·shift, 반올림은 0에서 먼 방향, 포화 −128~127
//  - 3×3 padding = 1, 1×1 padding = 0, L1~L9 뒤 ReLU
//
// 특징맵 메모리 배치: int8 [C][H][W] (패딩 없이 실제 크기로 연속 배치)
// ----------------------------------------------------------------------

#define SPK_IN_C        1
#define SPK_IN_H        40
#define SPK_IN_W        64
#define SPK_EMB_DIM     64
#define SPK_NUM_LAYERS  11

typedef enum {
    SPK_CONV3X3,
    SPK_DW3X3,
    SPK_PW1X1,
    SPK_GAP,
    SPK_FC
} spk_layer_type_t;

typedef struct {
    spk_layer_type_t type;
    int in_c, in_h, in_w;
    int out_c, out_h, out_w;
    int stride;
    int relu;
    const int8_t  *weight;   // CONV3X3 [OC][IC][3][3], DW3X3 [C][3][3], PW1X1/FC [OC][IC]
    const int32_t *bias;     // [OC]
    const int32_t *mult;     // [OC] 재양자화 multiplier
    const int8_t  *shift;    // [OC] 재양자화 오른쪽 시프트
} spk_layer_t;

// 레이어 하나가 끝날 때마다 호출 (검증·디버깅용, 필요 없으면 NULL)
//   layer_idx: 1 ~ SPK_NUM_LAYERS, out: [C][H][W] int8, size: 바이트 수
typedef void (*spk_layer_hook_t)(int layer_idx, const int8_t *out, int size, void *ctx);

extern const spk_layer_t spk_layers[SPK_NUM_LAYERS];

// 정수 추론: input [1][40][64] int8 → embedding [64] int8
void spk_cnn_run(const int8_t *input, int8_t *embedding, spk_layer_hook_t hook, void *ctx);

// 재양자화 (정수 규약의 단일 구현 지점)
int8_t spk_requant(int32_t acc, int32_t mult, int shift);

// INT8 임베딩 → L2 정규화된 float 벡터 (코사인 유사도 = 내적)
void spk_l2_normalize(const int8_t *embedding, float *out);

#endif
