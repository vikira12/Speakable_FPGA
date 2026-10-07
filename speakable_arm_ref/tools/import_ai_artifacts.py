"""AI 팀 산출물(model_pilot_v01 형식) → speakable_arm_ref 파일 규약으로 변환

사용: python import_ai_artifacts.py <AI 모델 디렉터리> <출력 디렉터리>
  입력: {name}_weight_int8.npy, {name}_bias_int32.npy, model_spec.json(multiplier·shift),
        test_vectors/<샘플>/input_int8.npy, {name}_int8.npy
  출력: <출력>/L{n}_*.npy, <출력>/tv/tv{k}_*.npy  (ref_model.py 규약)
"""
import glob
import json
import os
import sys
import numpy as np

# AI 팀 레이어 이름 → 레이어 번호
NAME_TO_N = {"conv1": 1, "dw2": 2, "pw3": 3, "dw4": 4, "pw5": 5, "dw6": 6,
             "pw7": 7, "dw8": 8, "pw9": 9, "gap10": 10, "fc11": 11}


def main(src, dst):
    tv_dst = os.path.join(dst, "tv")
    os.makedirs(tv_dst, exist_ok=True)

    spec = json.load(open(os.path.join(src, "model_spec.json"), encoding="utf8"))
    for L in spec["layers"]:
        n = NAME_TO_N[L["name"]]
        if L["type"] == "gap":
            continue
        name = L["name"]
        np.save(os.path.join(dst, f"L{n}_weight.npy"), np.load(os.path.join(src, f"{name}_weight_int8.npy")).astype(np.int8))
        np.save(os.path.join(dst, f"L{n}_bias.npy"), np.load(os.path.join(src, f"{name}_bias_int32.npy")).astype(np.int32))
        np.save(os.path.join(dst, f"L{n}_mult.npy"), np.array(L["multiplier"], dtype=np.int64).astype(np.int32))
        np.save(os.path.join(dst, f"L{n}_shift.npy"), np.array(L["shift"], dtype=np.int32))

    samples = sorted(d for d in glob.glob(os.path.join(src, "test_vectors", "*")) if os.path.isdir(d))
    with open(os.path.join(tv_dst, "index.txt"), "w", encoding="utf8") as idx:
        for k, d in enumerate(samples):
            idx.write(f"tv{k}\t{os.path.basename(d)}\n")
            np.save(os.path.join(tv_dst, f"tv{k}_input.npy"), np.load(os.path.join(d, "input_int8.npy")).reshape(1, 40, 64))
            for name, n in NAME_TO_N.items():
                a = np.load(os.path.join(d, f"{name}_int8.npy"))
                np.save(os.path.join(tv_dst, f"tv{k}_L{n}.npy"), a.reshape(a.shape[1:]))

    print(f"변환 완료 → {dst} (레이어 파라미터 10개, 테스트 벡터 {len(samples)}개)")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main(*sys.argv[1:])
