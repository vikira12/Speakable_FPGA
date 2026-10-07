"""스피커블 음성 전처리 참조 구현 (numpy): PCM16 16kHz → 입력 INT8 (1×40×64)

AI 팀 preprocess.json 사양을 따른다. 단계별 중간값을 돌려주므로 테스트 벡터와 단계마다 비교할 수 있다.
  hpf → gain → STFT power → mel_raw (40×T) → mel_64 (면적 가중 평균) → log_mel → 정규화(input_fp32) → input_int8
"""
import json
import os
import numpy as np


class Frontend:
    def __init__(self, model_dir):
        self.p = json.load(open(os.path.join(model_dir, "preprocess.json"), encoding="utf8"))
        self.hann = np.load(os.path.join(model_dir, "hann.npy"))
        self.mel_filter = np.load(os.path.join(model_dir, "mel_filter.npy"))
        self.mean = np.load(os.path.join(model_dir, "norm_mean.npy"))
        self.std = np.load(os.path.join(model_dir, "norm_std.npy"))
        self.input_scale = json.load(open(os.path.join(model_dir, "quant_params.json"), encoding="utf8"))["input_scale"]

    def hpf(self, x):
        a = self.p["hpf_a"]
        y = np.empty_like(x)
        prev_x = 0.0
        prev_y = 0.0
        for i in range(len(x)):
            prev_y = x[i] - prev_x + a * prev_y
            prev_x = x[i]
            y[i] = prev_y
        return y

    def gain(self, y):
        p = self.p
        rms = max(np.sqrt(np.mean(y * y)), p["gain_silence_rms_floor"])
        g_db = np.clip(p["gain_target_rms_dbfs"] - 20.0 * np.log10(rms), p["gain_min_db"], p["gain_max_db"])
        g = 10.0 ** (g_db / 20.0)
        peak = np.max(np.abs(y)) * g
        if peak > p["gain_peak_ceiling"]:
            g *= p["gain_peak_ceiling"] / peak
        return y * g

    def mel_raw(self, y):
        p = self.p
        fl, hop, nfft = p["frame_length"], p["hop_length"], p["n_fft"]
        n_frames = 1 + (len(y) - fl) // hop
        frames = np.stack([y[t * hop:t * hop + fl] * self.hann for t in range(n_frames)])
        spec = np.abs(np.fft.rfft(frames, n=nfft, axis=1)) ** 2          # (T, 257)
        return self.mel_filter @ spec.T                                   # (40, T)

    @staticmethod
    def area_resize(m, out_t):
        """시간 축 면적 가중 평균: 출력 프레임 j는 입력 구간 [j·T/out, (j+1)·T/out)의 겹친 넓이로 가중 평균"""
        t_in = m.shape[1]
        out = np.zeros((m.shape[0], out_t))
        for j in range(out_t):
            a, b = j * t_in / out_t, (j + 1) * t_in / out_t
            acc = np.zeros(m.shape[0])
            for i in range(int(np.floor(a)), int(np.ceil(b))):
                w = min(b, i + 1) - max(a, i)
                if w > 0:
                    acc += w * m[:, i]
            out[:, j] = acc / (b - a)
        return out

    def run(self, pcm16):
        p = self.p
        x = pcm16.astype(np.float64) / p["pcm_scale"]
        y = self.gain(self.hpf(x))
        mel_raw = self.mel_raw(y)
        mel_64 = self.area_resize(mel_raw, p["time_frames"])
        log_mel = np.log(np.maximum(mel_64, p["log_floor"]))
        norm = (log_mel - self.mean[:, None]) / np.maximum(self.std, p["std_floor"])[:, None]
        # AI 팀 파이프라인은 정규화 값을 float32로 저장한 뒤 양자화함 (float64 그대로 쓰면 반올림 경계에서 1 차이 발생)
        norm = norm.astype(np.float32).astype(np.float64)
        q = norm / self.input_scale
        q = np.sign(q) * np.floor(np.abs(q) + 0.5)                        # 0에서 먼 방향 반올림
        inp = np.clip(q, -128, 127).astype(np.int8)
        return {"mel_raw": mel_raw, "mel_64": mel_64, "log_mel": log_mel, "input_fp32": norm, "input_int8": inp}
