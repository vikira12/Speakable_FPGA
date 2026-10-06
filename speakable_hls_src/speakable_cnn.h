#ifndef SPEAKABLE_CNN_H
#define SPEAKABLE_CNN_H

#include "speakable_types.h"

// ----------------------------------------------------------------------
// 최상위 함수 (Vitis HLS가 이 함수 하나를 IP로 합성함)
//
// 동작: ofmap = [maxpool2x2]( [relu]( clamp8( (conv3x3(ifmap, weight) + bias) >> 8 ) ) )
//       3x3, stride 1, padding 1
//
// 인터페이스 요약
//  - ifmap, weight, bias, ofmap : DDR 메모리를 AXI4 Master(64비트)로 직접 접근
//                                 배치는 speakable_types.h의 MAX 크기 기준 (패딩 포함)
//                                 ifmap과 ofmap은 서로 다른 버퍼여야 함 (in-place 불가)
//  - in_h/in_w/in_ch/out_ch     : 이번 실행할 레이어의 실제 크기 (1 ~ 64)
//  - do_relu, do_pool           : 활성화/풀링 여부 제어
//  - perf                       : 성능 카운터, PS가 읽어감
//  모든 스칼라와 버퍼 주소는 AXI4-Lite 번들 하나(CTRL)에 모여 있음
// ----------------------------------------------------------------------
void speakable_cnn_top(
    const word_t *ifmap,
    const word_t *weight,
    const acc_t  *bias,
    word_t       *ofmap,
    ap_uint<8>  in_h,
    ap_uint<8>  in_w,
    ap_uint<8>  in_ch,
    ap_uint<8>  out_ch,
    ap_uint<1>  do_relu,
    ap_uint<1>  do_pool,
    perf_counters_t &perf
);

#endif
