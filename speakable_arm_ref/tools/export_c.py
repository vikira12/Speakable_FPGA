"""모델·테스트 벡터 npy → C 헤더 변환 (+ 참조 구현과의 일치 검사)

사용: python export_c.py <모델 디렉터리> <테스트 벡터 디렉터리> <출력 디렉터리> [C 헤더에 넣을 테스트 벡터 수]
  생성: speakable_weights.h, speakable_testvec.h
참조 구현 일치 검사는 테스트 벡터 전체에 대해 하고, C 헤더에는 앞에서부터 지정한 개수만 넣음
파일 규약은 ref_model.py 참고
"""
import glob
import os
import re
import sys
import numpy as np
import ref_model as rm


def c_array(ctype, name, arr, per_line=24):
    flat = np.asarray(arr).reshape(-1)
    body = []
    for i in range(0, len(flat), per_line):
        body.append("    " + ", ".join(str(int(v)) for v in flat[i:i + per_line]) + ",")
    return f"static const {ctype} {name}[{len(flat)}] = {{\n" + "\n".join(body) + "\n};\n"


def expected_weight_size(kind, in_c, out_c):
    return {"conv3x3": out_c * in_c * 9, "dw3x3": out_c * 9, "pw": out_c * in_c, "fc": out_c * in_c}[kind]


def check_range(name, arr, lo, hi):
    if arr.min() < lo or arr.max() > hi:
        sys.exit(f"[오류] {name} 값 범위 {arr.min()}~{arr.max()} 가 {lo}~{hi}를 벗어남")


def main(model_dir, tv_dir, out_dir, max_tv=None):
    os.makedirs(out_dir, exist_ok=True)
    params = rm.load_model(model_dir)

    # ---- 가중치 ----
    parts = ["#ifndef SPEAKABLE_WEIGHTS_H\n#define SPEAKABLE_WEIGHTS_H\n\n#include <stdint.h>\n\n",
             f"// 생성 원본: {os.path.basename(os.path.abspath(model_dir))} (tools/export_c.py로 생성, 직접 수정하지 말 것)\n\n"]
    for n, kind, in_c, out_c, stride, relu in rm.LAYERS:
        if kind == "gap":
            continue
        p = params[n]
        if p["weight"].size != expected_weight_size(kind, in_c, out_c):
            sys.exit(f"[오류] L{n} weight 크기 {p['weight'].shape} 가 {kind} {in_c}->{out_c}와 맞지 않음")
        for k, size in (("bias", out_c), ("mult", out_c), ("shift", out_c)):
            if p[k].size != size:
                sys.exit(f"[오류] L{n} {k} 크기 {p[k].shape}, 기대 ({size},)")
        check_range(f"L{n} weight", p["weight"], -128, 127)
        check_range(f"L{n} shift", p["shift"], 0, 62)
        parts.append(c_array("int8_t", f"L{n}_w", p["weight"]))
        parts.append(c_array("int32_t", f"L{n}_b", p["bias"]))
        parts.append(c_array("int32_t", f"L{n}_mult", p["mult"]))
        parts.append(c_array("int8_t", f"L{n}_shift", p["shift"]))
        parts.append("\n")
    parts.append("#endif\n")
    with open(os.path.join(out_dir, "speakable_weights.h"), "w", encoding="utf8", newline="\n") as f:
        f.write("".join(parts))

    # ---- 테스트 벡터 + 참조 구현 일치 검사 ----
    ks = sorted(int(re.search(r"tv(\d+)_input", p).group(1)) for p in glob.glob(os.path.join(tv_dir, "tv*_input.npy")))
    if not ks:
        sys.exit(f"[오류] {tv_dir}에 tv*_input.npy 가 없음")

    n_c = len(ks) if max_tv is None else min(int(max_tv), len(ks))
    parts = ["#ifndef SPEAKABLE_TESTVEC_H\n#define SPEAKABLE_TESTVEC_H\n\n#include <stdint.h>\n#include \"speakable_model.h\"\n\n",
             f"#define SPK_NUM_TV {n_c}\n\n"]
    all_ok = True
    for idx, k in enumerate(ks):
        inp = np.load(os.path.join(tv_dir, f"tv{k}_input.npy"))
        ref_outs = rm.run_model(params, inp.astype(np.int8).reshape(rm.INPUT_SHAPE))
        to_header = idx < n_c
        if to_header:
            parts.append(c_array("int8_t", f"tv{idx}_input", inp))
        for spec, ref in zip(rm.LAYERS, ref_outs):
            n = spec[0]
            exp = np.load(os.path.join(tv_dir, f"tv{k}_L{n}.npy")).astype(np.int8)
            if exp.size != ref.size or not np.array_equal(exp.reshape(-1), ref.reshape(-1)):
                bad = int(np.sum(exp.reshape(-1) != ref.reshape(-1))) if exp.size == ref.size else -1
                print(f"[불일치] tv{k} L{n}: 제공된 출력과 참조 구현 결과가 다름 (다른 원소 {bad}개)")
                all_ok = False
            if to_header:
                parts.append(c_array("int8_t", f"tv{idx}_L{n}", exp))
        if to_header:
            parts.append("\n")

    # 재양자화 경계값 단위 테스트 (정확히 절반·음수·포화·shift 0): 실제 데이터에선 거의 안 나오는 경우를 직접 검사
    cases = [(3, 1, 1), (-3, 1, 1), (1, 1, 1), (-1, 1, 1), (5, 1, 1), (-5, 1, 1),
             (6, 1, 2), (-6, 1, 2), (2, 1, 2), (-2, 1, 2), (10, 1, 2), (-10, 1, 2),
             (7, 3, 1), (-7, 3, 1), (1000, 1, 0), (-1000, 1, 0), (127, 1, 0), (-128, 1, 0),
             (2**31 - 1, 2**30, 60), (-(2**31), 2**30, 60), (123456, 1518500250, 40), (-123456, 1518500250, 40)]
    rng = np.random.default_rng(7)
    for _ in range(64):
        cases.append((int(rng.integers(-2**31, 2**31)), int(rng.integers(1, 2**31)), int(rng.integers(30, 63))))
    expect = [int(rm.requant(np.array([a], dtype=np.int64), [m], [s])[0]) for a, m, s in cases]
    parts.append(f"#define SPK_NUM_RQ_CASES {len(cases)}\n")
    parts.append("static const struct { int32_t acc; int32_t mult; int8_t shift; int8_t expect; } spk_rq_cases[] = {\n")
    for (a, m, s), e in zip(cases, expect):
        parts.append(f"    {{ {a}, {m}, {s}, {e} }},\n")
    parts.append("};\n\n")

    parts.append(f"static const int8_t *const spk_tv_input[SPK_NUM_TV] = {{ "
                 + ", ".join(f"tv{i}_input" for i in range(n_c)) + " };\n")
    parts.append("static const int8_t *const spk_tv_expect[SPK_NUM_TV][SPK_NUM_LAYERS] = {\n")
    for i in range(n_c):
        parts.append("    { " + ", ".join(f"tv{i}_L{spec[0]}" for spec in rm.LAYERS) + " },\n")
    parts.append("};\n\n#endif\n")
    with open(os.path.join(out_dir, "speakable_testvec.h"), "w", encoding="utf8", newline="\n") as f:
        f.write("".join(parts))

    print(f"헤더 생성 → {out_dir} (C 헤더 테스트 벡터 {n_c}개 / 일치 검사 {len(ks)}개)")
    print("참조 구현 일치 검사: " + ("통과" if all_ok else "불일치 있음 (정수 규약 확인 필요)"))
    sys.exit(0 if all_ok else 1)


if __name__ == "__main__":
    if len(sys.argv) not in (4, 5):
        sys.exit(__doc__)
    main(*sys.argv[1:])
