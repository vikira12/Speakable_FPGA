#include "speakable_match.h"
#include <math.h>
#include <string.h>

const spk_match_params_t SPK_MATCH_PARAMS_PLACEHOLDER = {
    .alpha               = 0.70f,    // matching_params.json 초기값 (최적화 전)
    .threshold           = 0.70f,    // 작품소개서 예시값
    .margin              = 0.10f,    // 작품소개서 예시값
    .immediate_threshold = -1.0f,    // 보정 전에는 즉시 출력 비활성
    .immediate_margin    = 0.0f,
    .conflict_threshold  = 0.90f,
    .adapt_min_sim       = 0.50f,
};

static float dot(const float *a, const float *b) {
    float s = 0.0f;
    for (int i = 0; i < SPK_EMB_DIM; i++) s += a[i] * b[i];
    return s;
}

static float cosine(const float *a, const float *b) {
    const float na = sqrtf(dot(a, a)), nb = sqrtf(dot(b, b));
    return (na > 0.0f && nb > 0.0f) ? dot(a, b) / (na * nb) : 0.0f;
}

static void update_proto(spk_expr_t *x) {
    for (int d = 0; d < SPK_EMB_DIM; d++) {
        float s = 0.0f;
        for (int i = 0; i < x->n_use; i++) s += x->use[i][d];
        x->proto[d] = s / (float)x->n_use;
    }
}

static int find(const spk_matcher_t *m, int id) {
    for (int k = 0; k < m->n_expr; k++)
        if (m->expr[k].id == id) return k;
    return -1;
}

void spk_match_init(spk_matcher_t *m, const spk_match_params_t *p) {
    memset(m, 0, sizeof(*m));
    m->p = *p;
}

float spk_match_score(const spk_matcher_t *m, int k, const float e[SPK_EMB_DIM]) {
    const spk_expr_t *x = &m->expr[k];
    float best = -1.0f;
    for (int i = 0; i < x->n_use; i++) { const float c = dot(e, x->use[i]); if (c > best) best = c; }
    return m->p.alpha * cosine(e, x->proto) + (1.0f - m->p.alpha) * best;
}

spk_reg_status_t spk_match_register(spk_matcher_t *m, int id, const float emb[][SPK_EMB_DIM], int n,
                                    int immediate_allowed, int *conflict_id) {
    if (n < 1 || n > SPK_MAX_SAMPLES) return SPK_REG_BAD_COUNT;
    if (find(m, id) >= 0)            return SPK_REG_DUPLICATE_ID;
    if (m->n_expr >= SPK_MAX_EXPR)   return SPK_REG_FULL;

    spk_expr_t *x = &m->expr[m->n_expr];
    memset(x, 0, sizeof(*x));
    x->id = id;
    x->immediate_allowed = (uint8_t)(immediate_allowed != 0);
    x->n_orig = x->n_use = n;
    memcpy(x->orig, emb, sizeof(float) * SPK_EMB_DIM * n);
    memcpy(x->use, emb, sizeof(float) * SPK_EMB_DIM * n);
    update_proto(x);

    // 혼동 표현 자동 지정: 사용자에게 표현 변경을 강요하지 않고 두 표현 모두 즉시 출력을 막음
    spk_reg_status_t st = SPK_REG_OK;
    for (int k = 0; k < m->n_expr; k++) {
        if (cosine(x->proto, m->expr[k].proto) >= m->p.conflict_threshold) {
            x->always_confirm = 1;
            m->expr[k].always_confirm = 1;
            if (conflict_id) *conflict_id = m->expr[k].id;
            st = SPK_REG_CONFLICT;
        }
    }
    m->n_expr++;
    return st;
}

spk_decision_t spk_match_decide(const spk_matcher_t *m, const float e[SPK_EMB_DIM]) {
    spk_decision_t r;
    memset(&r, 0, sizeof(r));
    r.s1 = r.s2 = -1.0f;
    if (m->n_expr == 0) {
        r.type = SPK_DECIDE_REJECT;
        r.why  = SPK_WHY_NO_EXPR;
        return r;
    }

    // 상위 3개 점수 유지 (삽입 정렬)
    int   top_k[SPK_MAX_CANDIDATES] = { 0 };
    float top_s[SPK_MAX_CANDIDATES] = { 0 };
    int   n_top = 0;
    for (int k = 0; k < m->n_expr; k++) {
        const float s = spk_match_score(m, k, e);
        int pos = n_top;
        while (pos > 0 && top_s[pos - 1] < s) pos--;
        if (pos >= SPK_MAX_CANDIDATES) continue;
        for (int j = (n_top < SPK_MAX_CANDIDATES ? n_top : SPK_MAX_CANDIDATES - 1); j > pos; j--) {
            top_k[j] = top_k[j - 1];
            top_s[j] = top_s[j - 1];
        }
        top_k[pos] = k;
        top_s[pos] = s;
        if (n_top < SPK_MAX_CANDIDATES) n_top++;
    }

    r.s1 = top_s[0];
    r.s2 = (n_top > 1) ? top_s[1] : -1.0f;   // 표현이 하나뿐이면 경쟁 후보 없음
    const spk_expr_t *best = &m->expr[top_k[0]];

    if (r.s1 < m->p.threshold) {
        r.type = SPK_DECIDE_REJECT;
        r.why  = SPK_WHY_LOW_SCORE;
        return r;
    }

    const int confident = (r.s1 - r.s2) >= m->p.margin;
    if (confident && !best->always_confirm) {
        r.why  = SPK_WHY_HIGH;
        r.n_cand = 1;
        r.cand_id[0] = best->id;
        r.cand_score[0] = r.s1;
        const int strict = m->p.immediate_threshold >= 0.0f &&
                           r.s1 >= m->p.immediate_threshold &&
                           (r.s1 - r.s2) >= m->p.immediate_margin;
        r.type = (best->immediate_allowed && strict) ? SPK_DECIDE_IMMEDIATE : SPK_DECIDE_CONFIRM_ONE;
        return r;
    }

    // TODO(보정 후 확정): 후보 포함 기준. 현재는 점수 상위 3개를 그대로 제시
    r.type = SPK_DECIDE_CANDIDATES;
    r.why  = confident ? SPK_WHY_ALWAYS_CONFIRM : SPK_WHY_LOW_MARGIN;
    r.n_cand = n_top;
    for (int j = 0; j < n_top; j++) {
        r.cand_id[j] = m->expr[top_k[j]].id;
        r.cand_score[j] = top_s[j];
    }
    return r;
}

spk_adapt_status_t spk_match_confirm(spk_matcher_t *m, int id, const float e[SPK_EMB_DIM]) {
    const int k = find(m, id);
    if (k < 0) return SPK_ADAPT_UNKNOWN_ID;
    spk_expr_t *x = &m->expr[k];

    // 기존 등록 정보와 크게 다른 음성은 잘못된 확인일 수 있으므로 반영하지 않음
    if (cosine(e, x->proto) < m->p.adapt_min_sim) return SPK_ADAPT_OUTLIER;

    if (x->n_use < SPK_MAX_SAMPLES) {
        memcpy(x->use[x->n_use++], e, sizeof(float) * SPK_EMB_DIM);
    } else {
        memcpy(x->use[x->oldest], e, sizeof(float) * SPK_EMB_DIM);
        x->oldest = (x->oldest + 1) % SPK_MAX_SAMPLES;
    }
    update_proto(x);
    return SPK_ADAPT_ADDED;
}

int spk_match_restore(spk_matcher_t *m, int id) {
    const int k = find(m, id);
    if (k < 0) return -1;
    spk_expr_t *x = &m->expr[k];
    x->n_use = x->n_orig;
    x->oldest = 0;
    memcpy(x->use, x->orig, sizeof(float) * SPK_EMB_DIM * x->n_orig);
    update_proto(x);
    return 0;
}
