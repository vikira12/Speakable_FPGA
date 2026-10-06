#ifndef DPU_TYPES_H
#define DPU_TYPES_H

#include <ap_int.h>
#include <hls_stream.h>

// ----------------------------------------------------------------------
// 데이터 타입 정의 (INT8 양자화 기준)
// ----------------------------------------------------------------------
typedef ap_int<8>   pixel_t;    // 입력/출력 특징맵 (activation), INT8
typedef ap_int<8>   weight_t;   // 가중치, INT8
typedef ap_int<32>  acc_t;      // 누산기 (MAC 결과), overflow 방지를 위해 32비트

// ----------------------------------------------------------------------
// 레이어 최대 크기 (자원 예산에 맞춰 조절. Zynq-7020 기준 보수적으로 설정)
// 실제 모델의 레이어가 이보다 작으면 CTRL 레지스터로 실제 크기를 넘겨서 사용
// ----------------------------------------------------------------------
#define MAX_IMG_H     64
#define MAX_IMG_W     64
#define MAX_IN_CH     64
#define MAX_OUT_CH    64
#define KSIZE         3      // 3x3 컨볼루션 고정 (다른 커널 크기는 별도 유닛으로 확장)

// 채널 방향 병렬화 정도. DSP 예산(약 200개)에 맞춰 조절.
// UNROLL_FACTOR * KSIZE * KSIZE 개의 MAC이 매 사이클 동시 동작한다고 보면 됨
#define UNROLL_FACTOR 8

// ----------------------------------------------------------------------
// 성능 카운터를 담는 구조체 (AXI4-Lite로 PS에 노출됨)
// ----------------------------------------------------------------------
struct perf_counters_t {
    ap_uint<32> cycle_count;   // 이번 추론(레이어 1회 실행)에 소요된 클럭 사이클 수
    ap_uint<32> mac_count;     // 실제 수행된 MAC(곱셈-누산) 연산 횟수
    ap_uint<32> invoke_count;  // 누적 호출 횟수 (재시작 전까지 유지, FPS 계산용)
};

#endif
