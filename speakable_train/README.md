# speakable_train

스피커블 임베딩 CNN 학습 (Colab GPU 또는 PC). 결과는 `model_pilot_v01`과 같은 산출물 형식이라
`speakable_arm_ref/tools`의 변환·검증 도구에 그대로 들어간다.

```
speakable_train.ipynb   Colab 노트북 (단계별 실행, 세션이 끊겨도 이어서 진행)
spk_train/
  frontend.py           학습용 전처리 (ref_frontend.py와 같은 계산, 벡터화) + 음성 증강
  data.py               manifest 읽기, 화자 단위 분할, 특징 캐시 (묶음 단위 저장)
  model.py              CNN (FPGA IP 구조 고정), AAM-softmax 헤드, BN 접기·가짜 양자화
  quant.py              활성화 보정, INT8 변환 (mult·shift), 정수 추론 시뮬레이션
  train.py              학습 루프, QAT
  evaluate.py           기기 매칭 방식 평가 (표현당 최대 5개 등록), α·T·M 보정
  export.py             산출물 디렉터리 + 테스트 벡터 (ref_model.py와 비트 일치 확인)
  pipeline.py           노트북·점검 스크립트 공용 단계 함수
frontend_base/          고정 전처리 파라미터 (preprocess.json, hann, Mel 필터)
checks/
  check_quant.py        정수 추론 ↔ ref_model.py 비트 일치 + 버그 주입 3종
  smoke_test.py         파일럿 음성으로 전체 흐름 → 기존 도구 → C 테스트까지
```

## 입력 데이터 (데이터 준비 노트북이 만든다)

`<data_root>/manifest.jsonl`, 한 줄에 발화 하나

```json
{"id": "...", "audio": "audio/xxx.flac", "speaker": "...", "text": "...", "split": "train"}
```

- audio: 16kHz mono PCM16 (wav/flac), 0.5~3초 (8,000~48,000 샘플, 범위 밖은 캐시에서 제외)
- split: 생략하면 화자 이름 해시로 train/val/test = 8/1/1. 한 화자는 한 쪽에만 들어간다
- 같은 화자가 같은 문장을 2번 이상 말한 묶음이 있어야 기기와 같은 방식(화자 안에서 등록·인식)으로 평가된다.
  없으면 화자 구분 없이 문장 단위로 평가한다 (보고서의 `mode`)

## 학습 방식

- 손실: AAM-softmax. 클래스는 `speaker_text`(화자별 문장, 기본) 또는 `text`
- 증강: 속도 0.9~1.1, 잡음 SNR 10~35dB, 잔향, 앞뒤 무음 — 발화당 원본 1 + 증강본을 캐시에 미리 저장. 학습 중 SpecAugment
- 입력 정규화: 학습 데이터 원본의 Mel 대역별 평균·표준편차 → `norm_mean.npy`, `norm_std.npy`
- INT8: BN 접기 → 활성화 99.99 백분위 보정 → 가중치 채널별 scale → mult ∈ [2^30, 2^31), shift ≤ 62. 필요하면 QAT

## 매칭 파라미터 보정

검증셋 INT8 임베딩으로 기기 점수(`α·cos(평균) + (1−α)·max cos`)를 재현해서, 아래 조건을 만족하는 T·M 중
"확실·정답 + 0.5 × 애매·후보 적중"이 가장 큰 값을 고른다. 시험셋에는 그 값을 고정해 평가만 한다.

- 등록 표현을 확실히 틀림 ≤ `max_wrong` (기본 2%)
- 미등록 음성을 확실로 오인식 ≤ `max_unreg` (기본 5%)
- 즉시 출력 기준: 각각 0.2% / 0.5% 이하, 정답률 5% 미만이면 비활성(−1)
- conflict_threshold: 같은 화자의 서로 다른 표현 대표 벡터 cosine 95백분위, adapt_min_sim: 자기 표현과의 cosine 2백분위

## PC 점검

```bash
python checks/check_quant.py
```
```bash
python checks/smoke_test.py <작업 디렉터리>
```

smoke_test는 정확도를 보지 않는다. 학습 → INT8 → 산출물 → `import_ai_artifacts.py` → `export_c.py` →
`export_frontend_c.py` → C 테스트(`spk_test`, `fe_test`)가 모두 통과하는지만 확인한다.

## 산출물 반영

노트북 마지막 셀 안내대로 `speakable_arm_ref`에서 `import_ai_artifacts.py` → `export_c.py` → `export_frontend_c.py`를
실행하고, `matching_params.json` 값을 `SPK_MATCH_PARAMS`에 반영한다. 보드·HLS 검증 순서는 저장소 CLAUDE.md 참고.
