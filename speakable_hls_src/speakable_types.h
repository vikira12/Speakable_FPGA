#ifndef SPEAKABLE_TYPES_H
#define SPEAKABLE_TYPES_H

#include <ap_int.h>

// ----------------------------------------------------------------------
// 데이터 타입 정의 (INT8 양자화 기준)
// ----------------------------------------------------------------------
typedef ap_int<8>   pixel_t;    // 입력/출력 특징맵 (activation), INT8
typedef ap_int<8>   weight_t;   // 가중치, INT8
typedef ap_int<32>  acc_t;      // 누산기 (MAC 결과), overflow 방지를 위해 32비트

// DDR 접근 단위. Zynq HP 포트 폭(64비트)에 맞춰 INT8 8개를 한 워드로 묶어 전송
// 워드 안의 바이트 순서는 리틀엔디언 (byte i = bits[8i+7:8i]) → PS의 int8_t 배열과 그대로 호환
typedef ap_uint<64> word_t;
#define WORD_BYTES    8

// ----------------------------------------------------------------------
// 레이어 최대 크기 (자원 예산에 맞춰 조절. Zynq-7020 기준 보수적으로 설정)
// 실제 모델의 레이어가 이보다 작으면 CTRL 레지스터로 실제 크기를 넘겨서 사용
//
// DDR 메모리 배치는 항상 MAX 크기 기준 (패딩 포함):
//   ifmap/ofmap : int8 [MAX_IN_CH][MAX_IMG_H][MAX_IMG_W]   (행 하나 = 64바이트)
//   weight      : int8 [MAX_OUT_CH][MAX_IN_CH][KSIZE][KSIZE]
//   bias        : int32[MAX_OUT_CH]
// ----------------------------------------------------------------------
#define MAX_IMG_H     64
#define MAX_IMG_W     64
#define MAX_IN_CH     64
#define MAX_OUT_CH    64
#define KSIZE         3      // 3x3 컨볼루션 고정 (다른 커널 크기는 별도 유닛으로 확장)
#define KK            (KSIZE * KSIZE)

#define ROW_WORDS     (MAX_IMG_W / WORD_BYTES)                  // DDR에서 한 행 = 8워드
#define FMAP_WORDS    (MAX_IN_CH * MAX_IMG_H * ROW_WORDS)       // 특징맵 전체 워드 수
#define WEIGHT_WORDS  (MAX_OUT_CH * MAX_IN_CH * KK / WORD_BYTES)
#define OC_WEIGHT_WORDS (MAX_IN_CH * KK / WORD_BYTES)           // 출력 채널 하나의 가중치 = 72워드

// ----------------------------------------------------------------------
// 병렬화 파라미터
//   TIC × TOC 개의 MAC이 매 사이클 동시 동작 (현재 8 × 8 = 64 MAC/cycle)
//   입력 채널 TIC개를 동시에 읽고, 출력 채널 TOC개를 동시에 계산함
// ----------------------------------------------------------------------
#define TIC           8
#define TOC           8
#define IC_GROUPS     (MAX_IN_CH / TIC)     // 입력 채널 묶음 수 (최대 8)
#define OC_TILES      (MAX_OUT_CH / TOC)    // 출력 채널 타일 수 (최대 8)

// 출력 행 타일 높이. 입력은 위아래 패딩 포함 TH+2 행만 온칩에 올림
// (풀링이 타일 안에서 끝나도록 반드시 짝수)
#define TH            8
#define BAND_ROWS     (TH + 2)

// ----------------------------------------------------------------------
// 성능 카운터를 담는 구조체 (AXI4-Lite로 PS에 노출됨)
// ----------------------------------------------------------------------
struct perf_counters_t {
    ap_uint<32> cycle_count;   // 연산 파이프라인 반복 횟수 (II=1이므로 연산 구간 사이클 수와 같음, 로드/저장 제외)
    ap_uint<32> mac_count;     // 패딩을 제외하고 실제 의미 있는 MAC(곱셈-누산) 연산 횟수
    ap_uint<32> invoke_count;  // 누적 호출 횟수 (재시작 전까지 유지, FPS 계산용)
};

#endif
