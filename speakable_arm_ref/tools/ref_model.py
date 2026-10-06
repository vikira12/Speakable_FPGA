"""스피커블 CNN 정수 추론 참조 구현 (numpy)

C 구현(src/speakable_model.c)과 독립적으로 같은 정수 규약을 구현한다.
AI 팀의 PyTorch 정수 시뮬레이션 결과와 이 파일의 결과가 같아야 한다.

모델 디렉터리 파일 규약 (n = 레이어 번호, GAP인 L10은 파라미터 없음)
  L{n}_weight.npy  int8   L1: (16,1,3,3)  DW: (C,1,3,3)  PW: (OC,IC,1,1)  FC: (64,64)
  L{n}_bias.npy    int32  (OC,)
  L{n}_mult.npy    int32  (OC,)   재양자화 multiplier
  L{n}_shift.npy   int    (OC,)   재양자화 오른쪽 시프트
테스트 벡터 디렉터리 (k = 0, 1, ...)
  tv{k}_input.npy  int8   (1,40,64)
  tv{k}_L{n}.npy   int8   레이어 n의 출력 (C,H,W), L10·L11은 (64,)
"""
import os
import numpy as np

# (번호, 종류, in_c, out_c, stride, relu)
LAYERS = [
    (1,  "conv3x3", 1,  16, 2, True),
    (2,  "dw3x3",   16, 16, 1, True),
    (3,  "pw",      16, 32, 1, True),
    (4,  "dw3x3",   32, 32, 2, True),
    (5,  "pw",      32, 64, 1, True),
    (6,  "dw3x3",   64, 64, 1, True),
    (7,  "pw",      64, 64, 1, True),
    (8,  "dw3x3",   64, 64, 2, True),
    (9,  "pw",      64, 64, 1, True),
    (10, "gap",     64, 64, 1, False),
    (11, "fc",      64, 64, 1, False),
]
INPUT_SHAPE = (1, 40, 64)


def round_away_shift(x, shift):
    """x / 2^shift, 정확히 절반이면 0에서 먼 방향 (x: int64 배열, shift: 채널별 배열)"""
    x = x.astype(np.int64)
    shift = np.asarray(shift, dtype=np.int64)
    half = np.where(shift > 0, np.left_shift(np.int64(1), np.maximum(shift - 1, 0)), 0)
    mag = np.abs(x)
    q = np.where(shift > 0, np.right_shift(mag + half, shift), mag)
    return np.sign(x) * q


def requant(acc, mult, shift):
    """acc: (OC, ...) int64 → int8. 채널 축은 0번"""
    extra = (1,) * (acc.ndim - 1)
    prod = acc.astype(np.int64) * np.asarray(mult, dtype=np.int64).reshape(-1, *extra)
    out = round_away_shift(prod, np.asarray(shift).reshape(-1, *extra))
    return np.clip(out, -128, 127).astype(np.int8)


def _windows3x3(x, stride):
    """x: (C,H,W) → (C, OH, OW, 3, 3) 슬라이딩 창 (padding=1)"""
    c, h, w = x.shape
    xp = np.pad(x.astype(np.int64), ((0, 0), (1, 1), (1, 1)))
    oh = (h + 2 - 3) // stride + 1
    ow = (w + 2 - 3) // stride + 1
    win = np.empty((c, oh, ow, 3, 3), dtype=np.int64)
    for kh in range(3):
        for kw in range(3):
            win[:, :, :, kh, kw] = xp[:, kh:kh + stride * (oh - 1) + 1:stride, kw:kw + stride * (ow - 1) + 1:stride]
    return win


def run_layer(spec, p, x):
    n, kind, in_c, out_c, stride, relu = spec
    if kind == "gap":
        cnt = x.shape[1] * x.shape[2]
        s = x.astype(np.int64).reshape(x.shape[0], -1).sum(axis=1)
        q = (2 * np.abs(s) + cnt) // (2 * cnt)
        return np.clip(np.sign(s) * q, -128, 127).astype(np.int8)

    w = p["weight"].astype(np.int64)
    b = p["bias"].astype(np.int64)
    if kind == "conv3x3":
        win = _windows3x3(x, stride)                          # (IC,OH,OW,3,3)
        acc = np.einsum("ihwab,oiab->ohw", win, w.reshape(out_c, in_c, 3, 3))
    elif kind == "dw3x3":
        win = _windows3x3(x, stride)                          # (C,OH,OW,3,3)
        acc = np.einsum("chwab,cab->chw", win, w.reshape(out_c, 3, 3))
    elif kind == "pw":
        acc = np.einsum("ihw,oi->ohw", x.astype(np.int64), w.reshape(out_c, in_c))
    elif kind == "fc":
        acc = w.reshape(out_c, in_c) @ x.astype(np.int64).reshape(in_c)
    else:
        raise ValueError(kind)

    acc = acc + b.reshape(-1, *([1] * (acc.ndim - 1)))
    out = requant(acc, p["mult"], p["shift"])
    if relu:
        out = np.maximum(out, 0).astype(np.int8)
    return out


def load_model(model_dir):
    params = {}
    for spec in LAYERS:
        n, kind = spec[0], spec[1]
        if kind == "gap":
            continue
        params[n] = {k: np.load(os.path.join(model_dir, f"L{n}_{k}.npy")) for k in ("weight", "bias", "mult", "shift")}
    return params


def run_model(params, x):
    """x: (1,40,64) int8 → 레이어별 출력 리스트 (L1 ~ L11)"""
    assert x.shape == INPUT_SHAPE and x.dtype == np.int8
    outs = []
    for spec in LAYERS:
        x = run_layer(spec, params.get(spec[0]), x)
        outs.append(x)
    return outs
