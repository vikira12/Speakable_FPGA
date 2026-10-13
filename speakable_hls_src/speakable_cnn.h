#ifndef SPEAKABLE_CNN_H
#define SPEAKABLE_CNN_H

#include "speakable_types.h"

// ----------------------------------------------------------------------
// 최상위 함수 (Vitis HLS가 이 함수 하나를 IP로 합성함). 한 번 호출 = 레이어 하나
//
// 동작: ofmap = [maxpool2x2]( [relu]( requant( layer(ifmap, weight) + bias ) ) )
//   layer   : CONV3X3 / DW3X3 (padding 1) 또는 PW1X1 (padding 0), stride 1 또는 2
//   requant : 출력 채널별 sat8( round_away( acc × mult / 2^shift ) ), acc × mult는 INT64
//   출력 크기: out_h = (in_h − 1) / stride + 1, out_w도 같음
//
// 인터페이스 요약
//  - ifmap, weight, bias, mult, shift, ofmap : DDR을 AXI4 Master(64비트)로 직접 접근
//                 배치는 speakable_types.h 참고. ifmap과 ofmap은 서로 다른 버퍼여야 함
//  - in_h/in_w/in_ch/out_ch : 입력 크기와 출력 채널 수 (1 ~ 64)
//  - layer_type, stride     : LAYER_* (speakable_types.h), 1 또는 2
//  - do_relu, do_pool       : 활성화/풀링 여부 (풀링은 stride 1 CONV3X3 용도)
//  - perf                   : 성능 카운터
//  모든 스칼라와 버퍼 주소는 AXI4-Lite 번들 하나(CTRL)에 모여 있음
// ----------------------------------------------------------------------
void speakable_cnn_top(
    const word_t *ifmap,
    const word_t *weight,
    const acc_t  *bias,
    const acc_t  *mult,
    const acc_t  *shift,
    word_t       *ofmap,
    ap_uint<8>  in_h,
    ap_uint<8>  in_w,
    ap_uint<8>  in_ch,
    ap_uint<8>  out_ch,
    ap_uint<2>  layer_type,
    ap_uint<2>  stride,
    ap_uint<1>  do_relu,
    ap_uint<1>  do_pool,
    perf_counters_t &perf
);

#endif
