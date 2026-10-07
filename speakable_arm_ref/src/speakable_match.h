#ifndef SPEAKABLE_MATCH_H
#define SPEAKABLE_MATCH_H

#include <stdint.h>
#include "speakable_model.h"

// ----------------------------------------------------------------------
// 개인 대표 벡터 매칭 + 신뢰도 3단계 판정
//
// 점수   S_k = α·cos(e, c_k) + (1−α)·max_i cos(e, s_k,i)
//        e: 입력 임베딩(L2 정규화), c_k: 표현 k 샘플 평균, s_k,i: 표현 k의 i번째 샘플
// 판정   확실  S1 ≥ T 그리고 S1 − S2 ≥ M      → 결과 1개 (확인 후 출력, 조건 충족 시 즉시 출력)
//        애매  S1 ≥ T 그리고 S1 − S2 < M      → 후보 최대 3개
//        거절  S1 < T                         → 출력 없음, 재발화 안내
//        "항상 후보 확인" 표현은 점수가 높아도 후보로 표시
//
// 등록   웹 녹음 샘플로 사용 샘플을 채우고, 대표 벡터가 기존 표현과
//        conflict_threshold 이상 비슷하면 두 표현 모두 "항상 후보 확인"으로 지정
// 적응   사용자가 확인한 발화만 반영. 대표 벡터와 adapt_min_sim 미만이면 이상치로 제외,
//        사용 샘플은 가장 오래된 것부터 교체 → 웹 마이크 샘플이 점차 기기 마이크 샘플로 바뀜
//        최초 등록본은 점수에 쓰지 않고 따로 보존 (spk_match_restore로 되돌리기)
// ----------------------------------------------------------------------

#define SPK_MAX_EXPR          32     // 기본 20개 + 사용자 추가
#define SPK_MAX_SAMPLES       5      // 점수 계산에 쓰는 샘플 수 (= 웹 등록 개수)
#define SPK_MAX_CANDIDATES    3

typedef struct {
    float alpha;
    float threshold;              // T
    float margin;                 // M
    float immediate_threshold;    // 즉시 출력용 엄격한 T (음수면 즉시 출력 비활성)
    float immediate_margin;       // 즉시 출력용 엄격한 M
    float conflict_threshold;     // 등록 시 혼동 판정 (대표 벡터 cosine)
    float adapt_min_sim;          // 적응 반영 하한 (대표 벡터 cosine)
} spk_match_params_t;

// AI 팀 보정 전 임시값 (작품소개서 예시 기준). 실제 사용 전 반드시 교체
extern const spk_match_params_t SPK_MATCH_PARAMS_PLACEHOLDER;

typedef struct {
    int     id;                                   // 서버의 표현 ID
    uint8_t always_confirm;                       // 혼동 표현: 항상 후보로 표시
    uint8_t immediate_allowed;                    // 사용자가 즉시 출력을 허용한 표현
    int     n_orig;
    float   orig[SPK_MAX_SAMPLES][SPK_EMB_DIM];   // 최초 등록본 (보존용, 점수에 안 씀)
    int     n_use, oldest;                        // oldest: 가득 찼을 때 다음에 교체할 칸
    float   use[SPK_MAX_SAMPLES][SPK_EMB_DIM];    // 점수 계산용 샘플
    float   proto[SPK_EMB_DIM];                   // 사용 샘플 평균 (정규화하지 않음)
} spk_expr_t;

typedef struct {
    spk_match_params_t p;
    int        n_expr;
    spk_expr_t expr[SPK_MAX_EXPR];
} spk_matcher_t;

typedef enum {
    SPK_DECIDE_IMMEDIATE,     // 확인 없이 바로 음성 출력
    SPK_DECIDE_CONFIRM_ONE,   // 결과 1개 표시 → 확인 후 출력
    SPK_DECIDE_CANDIDATES,    // 후보 최대 3개 → 사용자 선택
    SPK_DECIDE_REJECT         // 출력 없음 → 재발화 안내
} spk_decide_type_t;

typedef enum {
    SPK_WHY_HIGH,             // 확실
    SPK_WHY_LOW_MARGIN,       // 1·2위 차이가 작음 (애매)
    SPK_WHY_ALWAYS_CONFIRM,   // 혼동 표현으로 지정됨
    SPK_WHY_LOW_SCORE,        // 모든 후보가 기준 미만 (미등록·잡음)
    SPK_WHY_NO_EXPR           // 등록된 표현 없음
} spk_decide_why_t;

typedef struct {
    spk_decide_type_t type;
    spk_decide_why_t  why;
    float s1, s2;
    int   n_cand;
    int   cand_id[SPK_MAX_CANDIDATES];        // 표현 ID, 점수 높은 순
    float cand_score[SPK_MAX_CANDIDATES];
} spk_decision_t;

typedef enum {
    SPK_REG_OK = 0,
    SPK_REG_CONFLICT,         // 등록됨, 혼동 표현으로 지정 (conflict_id 참고)
    SPK_REG_FULL,
    SPK_REG_BAD_COUNT,
    SPK_REG_DUPLICATE_ID
} spk_reg_status_t;

typedef enum {
    SPK_ADAPT_ADDED = 0,
    SPK_ADAPT_OUTLIER,        // 대표 벡터와 너무 달라 반영 안 함
    SPK_ADAPT_UNKNOWN_ID
} spk_adapt_status_t;

void spk_match_init(spk_matcher_t *m, const spk_match_params_t *p);

// emb: L2 정규화된 등록 임베딩 n개 (1 ~ SPK_MAX_SAMPLES)
spk_reg_status_t spk_match_register(spk_matcher_t *m, int id, const float emb[][SPK_EMB_DIM], int n,
                                    int immediate_allowed, int *conflict_id);

spk_decision_t spk_match_decide(const spk_matcher_t *m, const float e[SPK_EMB_DIM]);

// 사용자가 "맞다"고 확인한 발화만 호출
spk_adapt_status_t spk_match_confirm(spk_matcher_t *m, int id, const float e[SPK_EMB_DIM]);

// 적응 결과를 버리고 최초 등록본으로 되돌림 (0: 성공, −1: 없는 ID)
int spk_match_restore(spk_matcher_t *m, int id);

float spk_match_score(const spk_matcher_t *m, int expr_idx, const float e[SPK_EMB_DIM]);

#endif
