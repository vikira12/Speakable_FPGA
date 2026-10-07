"""전처리 파라미터·테스트 벡터 → C 헤더 변환 (+ 참조 전처리와의 일치 검사)

사용: python export_frontend_c.py <AI 모델 디렉터리> <출력 디렉터리> [C 헤더에 넣을 테스트 벡터 수]
  생성: speakable_frontend_params.h, speakable_frontend_testvec.h
  일치 검사는 test_vectors/ 전체(audio.wav → input_int8)에 대해 수행
"""
import glob
import json
import os
import sys
import wave
import numpy as np
from ref_frontend import Frontend

FIXED = {"sample_rate": 16000, "frame_length": 400, "hop_length": 160, "n_fft": 512, "n_mels": 40,
         "time_frames": 64, "min_samples": 8000, "max_samples": 48000, "center": False}


def f64_array(name, arr):
    flat = np.asarray(arr, dtype=np.float64).reshape(-1)
    body = ",\n".join("    " + ", ".join(repr(float(v)) for v in flat[i:i + 6]) for i in range(0, len(flat), 6))
    return f"static const double {name}[{len(flat)}] = {{\n{body}\n}};\n"


def int_array(ctype, name, arr, per_line=20):
    flat = np.asarray(arr).reshape(-1)
    body = ",\n".join("    " + ", ".join(str(int(v)) for v in flat[i:i + per_line]) for i in range(0, len(flat), per_line))
    return f"static const {ctype} {name}[{len(flat)}] = {{\n{body}\n}};\n"


def read_wav(path):
    w = wave.open(path)
    if w.getnchannels() != 1 or w.getsampwidth() != 2 or w.getframerate() != 16000:
        sys.exit(f"[오류] {path}: mono 16-bit 16kHz가 아님")
    return np.frombuffer(w.readframes(w.getnframes()), dtype=np.int16)


def main(model_dir, out_dir, max_tv=None):
    fe = Frontend(model_dir)
    p = fe.p
    for k, v in FIXED.items():
        if p.get(k) != v:
            sys.exit(f"[오류] preprocess.json {k}={p.get(k)} 가 C 구현 고정값 {v}와 다름")

    # ---- 파라미터 (Mel 필터는 대역별 0이 아닌 구간만 저장) ----
    starts, lens, offs, ws = [], [], [], []
    for m in range(fe.mel_filter.shape[0]):
        nz = np.nonzero(fe.mel_filter[m])[0]
        s, e = int(nz[0]), int(nz[-1]) + 1      # 구간 안에 0이 있어도 그대로 포함하면 결과가 같음
        starts.append(s); lens.append(e - s); offs.append(len(ws)); ws.extend(fe.mel_filter[m, s:e].tolist())

    os.makedirs(out_dir, exist_ok=True)
    h = ["#ifndef SPEAKABLE_FRONTEND_PARAMS_H\n#define SPEAKABLE_FRONTEND_PARAMS_H\n\n#include <stdint.h>\n\n",
         f"// 생성 원본: {os.path.basename(os.path.abspath(model_dir))} (tools/export_frontend_c.py로 생성, 직접 수정하지 말 것)\n\n"]
    consts = {"SPK_FE_PCM_SCALE": p["pcm_scale"], "SPK_FE_HPF_A": p["hpf_a"],
              "SPK_FE_GAIN_TARGET_DBFS": p["gain_target_rms_dbfs"], "SPK_FE_GAIN_MIN_DB": p["gain_min_db"],
              "SPK_FE_GAIN_MAX_DB": p["gain_max_db"], "SPK_FE_GAIN_PEAK_CEIL": p["gain_peak_ceiling"],
              "SPK_FE_GAIN_SILENCE_FLOOR": p["gain_silence_rms_floor"], "SPK_FE_LOG_FLOOR": p["log_floor"],
              "SPK_FE_STD_FLOOR": p["std_floor"], "SPK_FE_INPUT_SCALE": fe.input_scale}
    for k, v in consts.items():
        h.append(f"#define {k:<28} ({repr(float(v))})\n")
    h.append("\n")
    h.append(f64_array("spk_fe_hann", fe.hann))
    h.append(f64_array("spk_fe_mean", fe.mean))
    h.append(f64_array("spk_fe_std", fe.std))
    h.append(int_array("int16_t", "spk_fe_mel_start", starts))
    h.append(int_array("int16_t", "spk_fe_mel_len", lens))
    h.append(int_array("int16_t", "spk_fe_mel_off", offs))
    h.append(f64_array("spk_fe_mel_w", ws))
    h.append("\n#endif\n")
    with open(os.path.join(out_dir, "speakable_frontend_params.h"), "w", encoding="utf8", newline="\n") as f:
        f.write("".join(h))

    # ---- 테스트 벡터 + 참조 전처리 일치 검사 ----
    samples = sorted(d for d in glob.glob(os.path.join(model_dir, "test_vectors", "*")) if os.path.isdir(d))
    n_c = len(samples) if max_tv is None else min(int(max_tv), len(samples))
    t = ["#ifndef SPEAKABLE_FRONTEND_TESTVEC_H\n#define SPEAKABLE_FRONTEND_TESTVEC_H\n\n#include <stdint.h>\n\n",
         f"#define SPK_FE_NUM_TV {n_c}\n\n"]
    bad_total = 0
    for k, d in enumerate(samples):
        pcm = read_wav(os.path.join(d, "audio.wav"))
        ref = np.load(os.path.join(d, "input_int8.npy")).reshape(40, 64)
        bad = int(np.sum(fe.run(pcm)["input_int8"] != ref))
        if bad:
            print(f"[불일치] {os.path.basename(d)}: 참조 전처리와 input_int8 {bad}개 다름")
        bad_total += bad
        if k < n_c:
            t.append(f"// {os.path.basename(d)}\n")
            t.append(int_array("int16_t", f"fe_tv{k}_pcm", pcm, 24))
            t.append(int_array("int8_t", f"fe_tv{k}_input", ref, 32))
            t.append(f64_array(f"fe_tv{k}_log_mel", np.load(os.path.join(d, "log_mel.npy"))))
            t.append("\n")
    t.append("static const int16_t *const spk_fe_tv_pcm[SPK_FE_NUM_TV] = { " + ", ".join(f"fe_tv{k}_pcm" for k in range(n_c)) + " };\n")
    t.append("static const int spk_fe_tv_len[SPK_FE_NUM_TV] = { " + ", ".join(f"sizeof(fe_tv{k}_pcm) / 2" for k in range(n_c)) + " };\n")
    t.append("static const int8_t *const spk_fe_tv_input[SPK_FE_NUM_TV] = { " + ", ".join(f"fe_tv{k}_input" for k in range(n_c)) + " };\n")
    t.append("static const double *const spk_fe_tv_log_mel[SPK_FE_NUM_TV] = { " + ", ".join(f"fe_tv{k}_log_mel" for k in range(n_c)) + " };\n")
    t.append("\n#endif\n")
    with open(os.path.join(out_dir, "speakable_frontend_testvec.h"), "w", encoding="utf8", newline="\n") as f:
        f.write("".join(t))

    print(f"헤더 생성 → {out_dir} (C 헤더 테스트 벡터 {n_c}개 / 일치 검사 {len(samples)}개)")
    print("참조 전처리 일치 검사: " + ("통과" if bad_total == 0 else f"불일치 {bad_total}개"))
    sys.exit(0 if bad_total == 0 else 1)


if __name__ == "__main__":
    if len(sys.argv) not in (3, 4):
        sys.exit(__doc__)
    main(*sys.argv[1:])
