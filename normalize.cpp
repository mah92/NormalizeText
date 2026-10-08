#include "./normalize.h"
#include "language_detector/language_detector.h"
#include "replacements/replacements.h"
#include "espeak_phonemize/phonemize.hpp"
#include "shakkelha/shakkelha.h"
#include "nawar-halabi/ar_phonemizer.h"
#include "persian_phoneme/persian_postprocess.h"
#include "grain_phones.h"

#include <fstream>
#include <map>
#include <set>
#include <sstream>

#include <iostream>
#include <chrono>
#include <cstdio>
#include <vector>

namespace {

bool persianResourcesInitialized = false;

// ---------------------------------------------------------------------------
// One phonemized segment: the detector's language decision kept together with
// its IPA, so callers can tag each segment with its OWN language (ADR-048).
// ---------------------------------------------------------------------------
struct SegmentIpa {
    Language language;
    std::string text;
    std::string ipa;
};

// espeak voice for a segment (ADR-048, single place): the English voice is used
// ONLY for segments the detector calls English, so Arabic script can never
// reach it — that path spelled Persian letters as «arabic alef / arabic re».
inline const char* voiceForSegment(Language language) {
    return (language == Language::ENGLISH) ? "en-us" : "fa";
}

// Shared core: general replacements -> language detection -> per-segment phonemization.
// `normalized` receives the reunited normalised text; `segments` receives one entry per detected
// segment (language + source text + IPA). Both public entry points below build on this.
void buildSegments(Language mainlang, const std::string& input, const NormalizeConfig& config,
                   std::string& normalized, std::vector<SegmentIpa>& segments) {
    using clock = std::chrono::high_resolution_clock;
    using ms = std::chrono::duration<double, std::milli>;
    const auto t_total = clock::now();

    const char* data_path = config.espeak_data_path.empty() ? NULL : config.espeak_data_path.c_str();
    piper::eSpeakPhonemeConfig phonemeConfig;
    std::vector<std::vector<char32_t>> phonemes;

    auto t0 = clock::now();
    normalized = performGeneralReplacements(mainlang, input);
    LanguageDetector detector(mainlang);
    std::vector<DetectedSegment> text_segments = detector.detect_segments(normalized);
    auto t1 = clock::now();

    double espeak_ms = 0, postprocess_ms = 0, espeak_init_ms = 0, pers_init_ms = 0, lang_repl_ms = 0;

    for (auto& text_segment : text_segments) {
        auto& segment_text = text_segment.text;
        auto& language = text_segment.language;
        std::string phoneme_segment;

        auto t_lr0 = clock::now();
        segment_text = performLanguageSpecificReplacements(language, mainlang, segment_text);
        lang_repl_ms += ms(clock::now() - t_lr0).count();

        if (language == Language::ARABIC) {
            int err = shakkelha_initialize(config.shakkelha_onnx.c_str());
            if (err != 0) { std::cerr << "Shakkelha model not found!" << std::endl; return; }
            segment_text = shakkelha_diacritize(segment_text);
            phoneme_segment = convertArabicToIPA(segment_text);
        } else {
            auto t_espeak_init0 = clock::now();
            int err = piper::phonemize_eSpeak_Init(data_path);
            espeak_init_ms += ms(clock::now() - t_espeak_init0).count();
            if (err != 0) { std::cerr << "Espeak data folder not found!" << std::endl; return; }

            phonemes.clear();
            phonemeConfig.voice = voiceForSegment(language);

            auto t_esp = clock::now();
            piper::phonemize_eSpeak(segment_text, phonemeConfig, phonemes);
            phoneme_segment = piper::getIpaString(phonemes);
            espeak_ms += ms(clock::now() - t_esp).count();

            if (language == Language::PERSIAN) {
                auto t_pers_init0 = clock::now();
                if (!persianResourcesInitialized) {
                    persian_phoneme::initPersianResources(
                        config.ezafe_model_onnx.c_str(),
                        config.ezafe_model_spiece.c_str(),
                        config.hazm_words.c_str(),
                        config.hazm_verbs.c_str(),
                        config.hazm_stopwords.c_str(),
                        config.homograph_data.c_str());
                    persianResourcesInitialized = true;
                }
                pers_init_ms += ms(clock::now() - t_pers_init0).count();

                auto t_post0 = clock::now();
                phoneme_segment = persian_phoneme::postprocessPersianIPA(segment_text, phoneme_segment);
                postprocess_ms += ms(clock::now() - t_post0).count();
            }
        }

        segments.push_back(SegmentIpa{language, segment_text, phoneme_segment});
    }

    normalized = LanguageDetector::reunite_segments(text_segments);

    const double total_ms = ms(clock::now() - t_total).count();
    const double repl_detect_ms = ms(t1 - t0).count();
    const double compute_ms = repl_detect_ms + lang_repl_ms + espeak_ms + postprocess_ms;
    const double load_ms = espeak_init_ms + pers_init_ms;
    fprintf(stderr, "[NORM] total=%.1fms  load=%.1fms  compute=%.1fms  repl=%.1f  lang_repl=%.1f  espeak=%.1f  postproc=%.1f\n",
            total_ms, load_ms, compute_ms, repl_detect_ms, lang_repl_ms, espeak_ms, postprocess_ms);
}

}  // namespace

// The asset paths every normalisation needs. One source of truth: the wrappers below and any
// caller of normalizeToTaggedPhones use it, so ezafe / homograph / shakkelha can never be lost
// by an empty path in a hand-built config.
NormalizeConfig defaultNormalizeConfig()
{
    NormalizeConfig config;
    config.espeak_data_path = "";          // empty -> use the built-in default
    config.shakkelha_onnx = "./assets/shakkelha.onnx";
    config.ezafe_model_onnx = "./assets/ezafe_model.onnx";
    config.ezafe_model_spiece = "./assets/ezafe_spiece.model";
    config.hazm_words = "./assets/hazm_words.dat";
    config.hazm_verbs = "./assets/hazm_verbs.dat";
    config.hazm_stopwords = "./assets/hazm_stopwords.dat";
    config.homograph_data = "./assets/homograph_data.json";
    return config;
}

// Original signature — wraps the config-based version with default paths
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
     std::string &normalizedString, std::string &ipaString)
{
    normalizeString(mainlang, ipa_mode, input, normalizedString, ipaString,
                    defaultNormalizeConfig());
}

// Config-based version — behaviour unchanged (segment order, IPA concatenation, [NORM] timing line).
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
     std::string &normalizedString, std::string &ipaString,
     const NormalizeConfig& config)
{
    (void)ipa_mode;   // IPA mode is the only mode this pipeline implements
    std::vector<SegmentIpa> segments;
    buildSegments(mainlang, input, config, normalizedString, segments);
    if (ipa_mode) {
        for (const SegmentIpa& seg : segments) ipaString += seg.ipa;
    } else {
        ipaString.clear();   // raw mode never produced IPA before either
    }
}

// ADR-048: the GrainSpeech phone stream, tagged per segment.
bool normalizeToTaggedPhones(const Language mainlang, const std::string& input,
                             const NormalizeConfig& config, std::string& taggedPhones,
                             std::string* ipaOut, std::string* normalizedOut)
{
    std::vector<SegmentIpa> segments;
    std::string normalized;
    buildSegments(mainlang, input, config, normalized, segments);

    // The model's symbol table (cached): when present every segment's phones are first mapped onto
    // the symbols the model really carries, so the returned stream is already in its alphabet.
    static std::map<std::string, std::set<std::string>> symbolCache;
    const std::set<std::string>* known = nullptr;
    if (!config.symbols_file.empty()) {
        auto it = symbolCache.find(config.symbols_file);
        if (it == symbolCache.end()) {
            std::set<std::string> symbols;
            std::ifstream in(config.symbols_file);
            std::string line;
            while (std::getline(in, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (!line.empty()) symbols.insert(line);
            }
            it = symbolCache.emplace(config.symbols_file, std::move(symbols)).first;
        }
        known = &it->second;
    }

    taggedPhones.clear();
    for (const SegmentIpa& seg : segments) {
        if (seg.ipa.empty()) continue;
        const std::string langTag = (voiceForSegment(seg.language)[0] == 'e') ? "en" : "fa";
        std::string part = grain::toModelPhones(seg.ipa, seg.ipa, langTag);
        if (part.empty()) continue;
        if (known) {
            std::vector<std::string> tokens;
            std::istringstream ss(part);
            std::string tok;
            while (ss >> tok) tokens.push_back(tok);
            tokens = grain::mapPhonesToInventory(tokens, *known);
            part.clear();
            for (size_t i = 0; i < tokens.size(); ++i) {
                if (i) part += " ";
                part += tokens[i];
            }
        }
        if (!taggedPhones.empty()) taggedPhones += " ";
        taggedPhones += part;
    }

    if (ipaOut) {
        ipaOut->clear();
        for (const SegmentIpa& seg : segments) *ipaOut += seg.ipa;
    }
    if (normalizedOut) *normalizedOut = normalized;
    return !taggedPhones.empty();
}


// (end of normalize.cpp additions)

