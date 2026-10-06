#ifndef DPU_CONV_H
#define DPU_CONV_H

#include "dpu_types.h"

// ----------------------------------------------------------------------
// DPU 최상위 함수 (Vitis HLS가 이 함수 하나를 IP로 합성함)
//
// 인터페이스 요약
//  - ifmap, weight, bias, ofmap : DDR 메모리를 AXI4 Master로 직접 접근 (m_axi)
//  - in_h/in_w/in_ch/out_ch     : 이번 실행할 레이어의 실제 크기 (AXI4-Lite 레지스터)
//  - do_relu, do_pool           : 활성화/풀링 여부 제어 (AXI4-Lite 레지스터)
//  - perf                       : 성능 카운터, PS가 AXI4-Lite로 읽어감
// ----------------------------------------------------------------------
void dpu_conv_top(
    pixel_t   ifmap[MAX_IN_CH][MAX_IMG_H][MAX_IMG_W],
    weight_t  weight[MAX_OUT_CH][MAX_IN_CH][KSIZE][KSIZE],
    acc_t     bias[MAX_OUT_CH],
    pixel_t   conv_scratch[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W],  // conv 중간 결과용 DDR 스크래치 공간
    pixel_t   ofmap[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W],
    ap_uint<8>  in_h,
    ap_uint<8>  in_w,
    ap_uint<8>  in_ch,
    ap_uint<8>  out_ch,
    ap_uint<1>  do_relu,
    ap_uint<1>  do_pool,
    perf_counters_t &perf
);

#endif
