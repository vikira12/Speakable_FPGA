#include <stdio.h>
#include <math.h>
#include <string.h>
#include "speakable_match.h"

// ----------------------------------------------------------------------
// 매칭·판정 단위 테스트: 방향을 정해 만든 임베딩으로 판정 분기를 하나씩 확인
// ----------------------------------------------------------------------

static int n_check = 0, n_fail = 0;
#define CHECK(cond, msg) do { n_check++; if (!(cond)) { n_fail++; printf("  [FAIL] %s (line %d)\r\n", msg, __LINE__); } } while (0)

static spk_matcher_t M;   // 약 90KB라 정적 할당

// 축 a 방향에서 축 b 쪽으로 deg도 기운 단위 벡터
static void dir(float *v, int a, int b, float deg) {
    memset(v, 0, sizeof(float) * SPK_EMB_DIM);
    const float r = deg * 3.14159265f / 180.0f;
    v[a] = cosf(r);
    if (b >= 0) v[b] = sinf(r);
}

// 축 a 주변 샘플 n개 (축 b·c 쪽으로 조금씩 흔들림)
static void cluster(float out[][SPK_EMB_DIM], int n, int a, int b, float spread) {
    for (int i = 0; i < n; i++) {
        dir(out[i], a, b, spread * (float)(i - n / 2));
        out[i][(a + 20) % SPK_EMB_DIM] += 0.05f * (float)(i % 2 ? 1 : -1);
        float nrm = 0.0f;
        for (int d = 0; d < SPK_EMB_DIM; d++) nrm += out[i][d] * out[i][d];
        nrm = sqrtf(nrm);
        for (int d = 0; d < SPK_EMB_DIM; d++) out[i][d] /= nrm;
    }
}

int main(void) {
    float s[SPK_MAX_SAMPLES][SPK_EMB_DIM], q[SPK_EMB_DIM];
    spk_decision_t r;
    int conflict = -1;
    spk_match_params_t p = SPK_MATCH_PARAMS_PLACEHOLDER;

    // ---- 등록 전 ----
    spk_match_init(&M, &p);
    dir(q, 0, -1, 0);
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_REJECT && r.why == SPK_WHY_NO_EXPR, "등록 표현 없음 → 거절");

    // ---- 점수 공식: 직교하는 샘플 2개 → cos(e, c) = 1/√2, max cos(e, s) = 1 ----
    dir(s[0], 0, -1, 0);
    dir(s[1], 1, -1, 0);
    spk_match_register(&M, 1, s, 2, 0, 0);
    dir(q, 0, -1, 0);
    {
        const float expect = p.alpha * 0.70710678f + (1.0f - p.alpha) * 1.0f;
        CHECK(fabsf(spk_match_score(&M, 0, q) - expect) < 1e-5f, "점수 = α·cos(e, 대표) + (1−α)·max cos(e, 샘플)");
    }
    spk_match_init(&M, &p);

    // ---- 표현 하나뿐: 경쟁 후보가 없으므로 확실 ----
    cluster(s, 5, 0, 1, 3.0f);
    CHECK(spk_match_register(&M, 100, s, 5, 0, 0) == SPK_REG_OK, "표현 A 등록");
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_CONFIRM_ONE && r.cand_id[0] == 100 && r.s2 < 0.0f, "표현 1개 → 확인 후 출력");

    // ---- 표현 3개 ----
    cluster(s, 5, 1, 2, 3.0f);
    CHECK(spk_match_register(&M, 200, s, 5, 0, 0) == SPK_REG_OK, "표현 B 등록");
    cluster(s, 5, 2, 3, 3.0f);
    CHECK(spk_match_register(&M, 300, s, 5, 0, 0) == SPK_REG_OK, "표현 C 등록");

    dir(q, 0, 1, 5.0f);
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_CONFIRM_ONE && r.why == SPK_WHY_HIGH && r.cand_id[0] == 100, "A 근처 → 확실(A)");

    dir(q, 10, -1, 0);
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_REJECT && r.why == SPK_WHY_LOW_SCORE && r.n_cand == 0, "등록 안 된 방향 → 거절");

    dir(q, 0, 1, 44.0f);   // A와 B 사이: S1 ≥ 0.70, 1·2위 차이 < 0.10
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_CANDIDATES && r.why == SPK_WHY_LOW_MARGIN, "A·B 사이 → 애매(후보)");
    CHECK(r.n_cand == 3 && r.cand_id[0] == 100 && r.cand_id[1] == 200, "후보는 점수 높은 순 (A, B, ...)");
    CHECK(r.cand_score[0] >= r.cand_score[1] && r.cand_score[1] >= r.cand_score[2], "후보 점수 내림차순");

    // ---- 등록 시 혼동 표현 자동 지정 ----
    cluster(s, 5, 0, 1, 2.0f);   // A와 거의 같은 발화로 새 표현 D 등록
    CHECK(spk_match_register(&M, 400, s, 5, 0, &conflict) == SPK_REG_CONFLICT && conflict == 100, "A와 비슷한 D → 혼동 지정");
    dir(q, 0, -1, 0);
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_CANDIDATES, "혼동 표현은 점수가 높아도 후보로 표시");
    CHECK(r.why == SPK_WHY_ALWAYS_CONFIRM || r.why == SPK_WHY_LOW_MARGIN, "혼동 표현 판정 사유");

    // 혼동 지정만으로 후보가 되는지 따로 확인: 1·2위 차이는 충분히 크지만 혼동 표현인 경우
    p.conflict_threshold = 0.80f;
    spk_match_init(&M, &p);
    cluster(s, 5, 0, 1, 1.0f);
    spk_match_register(&M, 100, s, 5, 0, 0);
    dir(q, 0, 8, 35.0f);
    for (int i = 0; i < 5; i++) memcpy(s[i], q, sizeof(q));   // A와 35° 떨어진 표현 (cos 0.82 ≥ 0.80)
    CHECK(spk_match_register(&M, 200, s, 5, 0, &conflict) == SPK_REG_CONFLICT, "35° 떨어진 표현 → 혼동 지정");
    dir(q, 0, -1, 0);
    r = spk_match_decide(&M, q);
    CHECK(r.s1 - r.s2 >= p.margin, "(전제) 1·2위 차이는 Margin 이상");
    CHECK(r.type == SPK_DECIDE_CANDIDATES && r.why == SPK_WHY_ALWAYS_CONFIRM, "차이가 커도 혼동 표현이면 후보로 표시");
    p = SPK_MATCH_PARAMS_PLACEHOLDER;

    // ---- 즉시 출력: 사용자가 허용 + 엄격한 기준 통과일 때만 ----
    spk_match_init(&M, &p);                       // 임시값: 즉시 출력 비활성
    cluster(s, 5, 3, 4, 2.0f);
    spk_match_register(&M, 500, s, 5, 1, 0);
    cluster(s, 5, 5, 6, 2.0f);
    spk_match_register(&M, 600, s, 5, 0, 0);
    dir(q, 3, -1, 0);
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_CONFIRM_ONE, "보정 전(즉시 기준 없음) → 허용 표현도 확인 후 출력");

    p.immediate_threshold = 0.90f;
    p.immediate_margin = 0.30f;
    M.p = p;
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_IMMEDIATE && r.cand_id[0] == 500, "허용 표현 + 엄격한 기준 통과 → 즉시 출력");
    dir(q, 3, 4, 30.0f);         // 확실하지만 엄격한 기준(0.90)은 못 넘음
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_CONFIRM_ONE, "엄격한 기준 미달 → 확인 후 출력");
    dir(q, 5, -1, 0);
    r = spk_match_decide(&M, q);
    CHECK(r.type == SPK_DECIDE_CONFIRM_ONE && r.cand_id[0] == 600, "즉시 출력을 허용하지 않은 표현 → 확인 후 출력");

    // ---- 확인 기반 적응 ----
    p = SPK_MATCH_PARAMS_PLACEHOLDER;
    spk_match_init(&M, &p);
    cluster(s, 5, 0, 1, 3.0f);
    spk_match_register(&M, 100, s, 5, 0, 0);
    dir(q, 10, -1, 0);
    CHECK(spk_match_confirm(&M, 100, q) == SPK_ADAPT_OUTLIER, "대표 벡터와 먼 확인 발화 → 이상치로 제외");
    CHECK(spk_match_confirm(&M, 999, q) == SPK_ADAPT_UNKNOWN_ID, "없는 표현 ID");

    float dev[SPK_MAX_SAMPLES][SPK_EMB_DIM];
    for (int i = 0; i < SPK_MAX_SAMPLES; i++) {   // 기기 마이크 특성 때문에 조금 다른 방향
        dir(dev[i], 0, 7, 25.0f + (float)i);
        CHECK(spk_match_confirm(&M, 100, dev[i]) == SPK_ADAPT_ADDED, "확인 발화 반영");
        if (i == 0) {
            CHECK(memcmp(M.expr[0].use[0], dev[0], sizeof(dev[0])) == 0, "가장 오래된 샘플(웹 등록 1번)부터 교체");
            CHECK(memcmp(M.expr[0].use[1], s[1], sizeof(s[1])) == 0, "나머지 웹 샘플은 유지");
        }
    }
    CHECK(memcmp(M.expr[0].use, dev, sizeof(dev)) == 0, "5번 확인 후 웹 샘플이 모두 기기 샘플로 교체");
    CHECK(memcmp(M.expr[0].orig, s, sizeof(float) * SPK_EMB_DIM * 5) == 0, "최초 등록본은 보존");
    CHECK(spk_match_restore(&M, 100) == 0 && memcmp(M.expr[0].use, s, sizeof(float) * SPK_EMB_DIM * 5) == 0,
          "최초 등록본으로 되돌리기");

    // ---- 등록 오류 ----
    CHECK(spk_match_register(&M, 100, s, 5, 0, 0) == SPK_REG_DUPLICATE_ID, "중복 ID");
    CHECK(spk_match_register(&M, 101, s, 0, 0, 0) == SPK_REG_BAD_COUNT, "샘플 0개");
    CHECK(spk_match_register(&M, 102, s, SPK_MAX_SAMPLES + 1, 0, 0) == SPK_REG_BAD_COUNT, "샘플 초과");
    spk_match_init(&M, &p);
    for (int k = 0; k < SPK_MAX_EXPR; k++) {
        dir(s[0], k % SPK_EMB_DIM, -1, 0);
        spk_match_register(&M, 1000 + k, s, 1, 0, 0);
    }
    CHECK(spk_match_register(&M, 9999, s, 1, 0, 0) == SPK_REG_FULL, "표현 수 초과");

    printf("%s 매칭·판정 단위 테스트 %d/%d\r\n", n_fail ? "[FAIL]" : "[PASS]", n_check - n_fail, n_check);
    return n_fail ? 1 : 0;
}
