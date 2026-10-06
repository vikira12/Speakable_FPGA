"""실제 가중치가 오기 전, 파이프라인 검증용 더미 모델과 테스트 벡터를 만든다.

사용: python make_dummy.py <출력 디렉터리>
  <출력>/L{n}_*.npy, <출력>/tv/tv{k}_*.npy 생성 (파일 규약은 ref_model.py 참고)
"""
import os
import sys
import numpy as np
import ref_model as rm

SHIFT = 20          # 더미 재양자화 시프트
TARGET = 100        # 채널별 최대 |출력|이 이 정도가 되도록 multiplier 보정
NUM_TV = 3


def weight_shape(kind, in_c, out_c):
    return {"conv3x3": (out_c, in_c, 3, 3), "dw3x3": (out_c, 1, 3, 3),
            "pw": (out_c, in_c, 1, 1), "fc": (out_c, in_c)}[kind]


def main(out_dir):
    rng = np.random.default_rng(2026)
    tv_dir = os.path.join(out_dir, "tv")
    os.makedirs(tv_dir, exist_ok=True)

    inputs = [np.clip(rng.normal(0, 40, rm.INPUT_SHAPE), -128, 127).astype(np.int8) for _ in range(NUM_TV)]

    params = {}
    x = inputs[0]
    for spec in rm.LAYERS:
        n, kind, in_c, out_c, stride, relu = spec
        if kind != "gap":
            p = {"weight": rng.integers(-127, 128, weight_shape(kind, in_c, out_c), dtype=np.int8),
                 "bias": rng.integers(-2000, 2001, out_c, dtype=np.int32)}
            # 누산값 범위를 보고 채널별 multiplier 보정 (1/2^SHIFT 배 단위)
            acc_max = _acc_maxabs(spec, p, x)
            p["mult"] = np.maximum(1, np.round(TARGET / np.maximum(acc_max, 1) * (1 << SHIFT))).astype(np.int32)
            p["shift"] = np.full(out_c, SHIFT, dtype=np.int32)
            params[n] = p
            for k, v in p.items():
                np.save(os.path.join(out_dir, f"L{n}_{k}.npy"), v)
        x = rm.run_layer(spec, params.get(n), x)

    for k, inp in enumerate(inputs):
        np.save(os.path.join(tv_dir, f"tv{k}_input.npy"), inp)
        for spec, out in zip(rm.LAYERS, rm.run_model(params, inp)):
            np.save(os.path.join(tv_dir, f"tv{k}_L{spec[0]}.npy"), out)

    print(f"더미 모델 → {out_dir}, 테스트 벡터 {NUM_TV}개 → {tv_dir}")


def _acc_maxabs(spec, p, x):
    """재양자화 전 누산값의 채널별 최대 절댓값"""
    n, kind, in_c, out_c, stride, relu = spec
    w = p["weight"].astype(np.int64)
    if kind in ("conv3x3", "dw3x3"):
        win = rm._windows3x3(x, stride)
        if kind == "conv3x3":
            acc = np.einsum("ihwab,oiab->ohw", win, w.reshape(out_c, in_c, 3, 3))
        else:
            acc = np.einsum("chwab,cab->chw", win, w.reshape(out_c, 3, 3))
    elif kind == "pw":
        acc = np.einsum("ihw,oi->ohw", x.astype(np.int64), w.reshape(out_c, in_c))
    else:
        acc = w.reshape(out_c, in_c) @ x.astype(np.int64).reshape(in_c)
    acc = acc + p["bias"].astype(np.int64).reshape(-1, *([1] * (acc.ndim - 1)))
    return np.abs(acc).reshape(out_c, -1).max(axis=1)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "dummy_model")
