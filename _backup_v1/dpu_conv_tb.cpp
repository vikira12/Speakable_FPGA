#include "dpu_conv.h"
#include <cstdlib>
#include <iostream>

// static으로 선언해 스택 오버플로 방지 (배열이 크기 때문)
static pixel_t   ifmap[MAX_IN_CH][MAX_IMG_H][MAX_IMG_W];
static weight_t  weight[MAX_OUT_CH][MAX_IN_CH][KSIZE][KSIZE];
static acc_t     bias[MAX_OUT_CH];
static pixel_t   conv_scratch[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W];
static pixel_t   ofmap[MAX_OUT_CH][MAX_IMG_H][MAX_IMG_W];

int main() {
    const int H = 16, W = 16, IN_CH = 8, OUT_CH = 4;

    // 간단한 패턴으로 입력/가중치 초기화 (정답 검증보다는 동작 확인용)
    for (int c = 0; c < IN_CH; c++)
        for (int h = 0; h < H; h++)
            for (int w = 0; w < W; w++)
                ifmap[c][h][w] = (pixel_t)((h + w + c) % 7 - 3);

    for (int oc = 0; oc < OUT_CH; oc++) {
        bias[oc] = 0;
        for (int ic = 0; ic < IN_CH; ic++)
            for (int kh = 0; kh < KSIZE; kh++)
                for (int kw = 0; kw < KSIZE; kw++)
                    weight[oc][ic][kh][kw] = (weight_t)((oc + ic + kh + kw) % 5 - 2);
    }

    perf_counters_t perf = {0, 0, 0};

    dpu_conv_top(ifmap, weight, bias, conv_scratch, ofmap,
                 H, W, IN_CH, OUT_CH,
                 /*do_relu=*/1, /*do_pool=*/1,
                 perf);

    std::cout << "cycle_count  = " << perf.cycle_count  << std::endl;
    std::cout << "mac_count    = " << perf.mac_count    << std::endl;
    std::cout << "invoke_count = " << perf.invoke_count << std::endl;

    std::cout << "ofmap[0][0][0..3] = ";
    for (int i = 0; i < 4; i++) std::cout << (int)ofmap[0][0][i] << " ";
    std::cout << std::endl;

    // 최소한의 sanity check: NaN/폭주 없이 값이 -128~127 범위인지
    bool pass = true;
    for (int oc = 0; oc < OUT_CH; oc++)
        for (int h = 0; h < H/2; h++)
            for (int w = 0; w < W/2; w++)
                if (ofmap[oc][h][w] < -128 || ofmap[oc][h][w] > 127) pass = false;

    std::cout << (pass ? "TEST PASSED" : "TEST FAILED") << std::endl;
    return pass ? 0 : 1;
}
