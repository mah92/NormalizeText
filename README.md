<div align="center">

# ﷽

---

**NormalizeText** — A High-Performance C++ Phonemization Pipeline for Multilingual TTS

</div>

This is a **C++ port** of the text normalization and phonemization pipeline from [Piper-with-LCA-Phonemizer](https://github.com/MahtaFetrat/Piper-with-LCA-Phonemizer), implementing the system described in:

***Beyond Unified Models: A Service-Oriented Approach to Low-Latency, Context-Aware Phonemization for Real-Time TTS***

The original Python pipeline is reimplemented in C++ for **low-latency, on-device TTS preprocessing**, with additional features including **rule-based multilingual language detection** and **Shakkelha Arabic diacritization**.

---

## Pipeline Overview

```
Input Text
    │
    ▼
General Replacements (numbers, symbols, abbreviations)
    │
    ▼
Language Detection (per-segment: FA / EN / AR)
    │
    ├── Arabic segments ──► Shakkelha diacritization ──► Nawar-Halabi phonemizer ──► IPA
    │
    ├── Persian segments ──► eSpeak-ng phonemization ──► Persian post-processing ──► IPA
    │                         (homograph disambiguation + ezafe detection)
    │
    └── English segments ──► eSpeak-ng phonemization ──► IPA
    │
    ▼
Output: normalized text + IPA phonemes
```

---

## Features

### Text Normalization
- **Multilingual replacements** for Arabic, Persian, English, Urdu, Turkish-Azerbaijani, Kurdi, and Pashto
- Number-to-text conversion for all supported languages
- Symbol and punctuation normalization

### Language Detection
- **Rule-based, character-level detection** of Persian, English, and Arabic segments
- Handles mixed-script sentences (e.g., Persian text with English words)
- Context-aware: uses the main language to disambiguate shared Perso-Arabic characters

### Arabic Pipeline
- **Shakkelha ONNX model** for automatic diacritization (tashkeel)
- **Nawar-Halabi rule-based phonemizer** for Arabic-to-IPA conversion

### Persian Pipeline
- **eSpeak-ng** for base phonemization
- **Homograph disambiguation** using context-lemma scoring
- **Ezafe detection** via distilled ALBERT ONNX model
- **hazm_cpp** integration: tokenizer, lemmatizer, stemmer, stopwords

### English Pipeline
- **eSpeak-ng** for phonemization (US English voice)

---

## Dependencies

| Dependency | Purpose |
|-----------|---------|
| CMake ≥ 3.10 | Build system |
| C++17 compiler | GCC 8+ or Clang 7+ |
| ONNX Runtime 1.21.0 | Shakkelha & Ezafe model inference |
| eSpeak-ng (latest) | Phonemization (FA, EN) |
| ICU (uc, i18n, io) | Unicode handling |
| Protobuf | ONNX Runtime dependency |

---

## Build

### 1. Install system dependencies

```bash
sudo apt update
sudo apt install -y git cmake build-essential libprotobuf-dev protobuf-compiler libicu-dev
```

### 2. Build & install ONNX Runtime

```bash
git clone --recursive https://github.com/microsoft/onnxruntime --depth 1 --branch v1.21.0
cd onnxruntime
./build.sh --config Release --build_shared_lib --parallel $(nproc)
sudo cmake --install build/Linux/Release --prefix /usr/local
```

### 3. Build & install eSpeak-ng (latest)

> The latest code is required for `espeak_TextToPhonemesWithTerminator`.

```bash
sudo apt install -y make autoconf automake libtool pkg-config libpcaudio-dev
git clone https://github.com/espeak-ng/espeak-ng.git
cd espeak-ng
./autogen.sh
./configure --prefix=/usr
make -j$(nproc)
sudo make install
sudo ldconfig
```

### 4. Build NormalizeText

```bash
cd NormalizeText
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 5. Run

```bash
./NormalizeCSV FA ../data/dataset_FA-EN-AR.txt
./NormalizeCSV AR ../data/dataset_AR-EN.txt
./NormalizeCSV EN ../data/dataset_EN.txt
```

**Arguments:** `<main_language: FA|EN|AR> <input_file>`

**Output files** (written next to input):
- `*-complete.csv` — original, normalized, and IPA text
- `*-normalized.csv` — normalized text only
- `*-ipa.csv` — IPA phonemes only
- `*-normalized.txt` — normalized text (no metadata)

---

## Project Structure

```
NormalizeText/
├── NormalizeCSVMain.cpp         # CLI entry point
├── normalize.cpp/h              # Core normalization pipeline
├── language_detector/           # Rule-based FA/EN/AR language detection
├── replacements/                # Text replacements for 7+ languages
│   ├── arabic/
│   ├── persian/
│   ├── english/
│   ├── urdu/
│   ├── turki-azarbaijani/
│   ├── kurdi/
│   └── pashtoo/
├── espeak_phonemize/            # eSpeak-ng C++ wrapper
├── shakkelha/                   # Arabic diacritization (ONNX)
├── nawar-halabi/                # Arabic rule-based phonemizer
├── persian_phoneme/             # Persian IPA post-processing
├── homograph/                   # Homograph dictionary & disambiguation
├── vits2-tokenizer/             # VITS2 phoneme tokenizer
├── hazm_cpp/                    # hazm Persian NLP (C++ port)
├── persian-ezafe-albert-cpp/    # ALBERT ezafe detector (C++ port)
├── json_cpp/                    # nlohmann JSON
└── assets/                      # ONNX models & data files
    ├── shakkelha.onnx
    ├── ezafe_model.onnx
    ├── ezafe_spiece.model
    ├── homograph_data.json
    ├── hazm_words.dat
    ├── hazm_verbs.dat
    └── hazm_stopwords.dat
```

---

## Additional Features (beyond original paper)

- **Automatic rule-based language detection** for Persian, English, and Arabic — no external model needed
- **Shakkelha Arabic diacritization** model integration (ONNX)
- **Full C++ implementation** — suitable for embedded and on-device TTS
- **Configurable asset paths** — all model/data paths passed via `NormalizeConfig` struct

---

## Citation

If you use this work, please cite the original paper:

```bibtex
@misc{fetrat2025servicetts,
      title={Beyond Unified Models: A Service-Oriented Approach to Low Latency, Context Aware Phonemization for Real Time TTS},
      author={Mahta Fetrat and Donya Navabi and Zahra Dehghanian and Morteza Abolghasemi and Hamid R. Rabiee},
      year={2025},
      eprint={2512.08006},
      archivePrefix={arXiv},
      primaryClass={cs.SD},
      url={https://arxiv.org/abs/2512.08006},
}
```

---

## References

- **Original repository:** [MahtaFetrat/Piper-with-LCA-Phonemizer](https://github.com/MahtaFetrat/Piper-with-LCA-Phonemizer)
- **Paper:** [arXiv:2512.08006](https://arxiv.org/abs/2512.08006)
- **Ezafe model:** [abreza/persian-ezafe-albert](https://huggingface.co/abreza/persian-ezafe-albert)
- **Homo-GE2PE (Persian G2P):** [MahtaFetrat/Homo-GE2PE-Persian](https://github.com/MahtaFetrat/Homo-GE2PE-Persian)
