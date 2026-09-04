#include "./normalize.h"
#include "language_detector/language_detector.h"
#include "replacements/replacements.h"
#include "espeak_phonemize/phonemize.hpp"
#include "shakkelha/shakkelha.h"
#include "nawar-halabi/ar_phonemizer.h"
#include "persian_phoneme/persian_postprocess.h"

#include <iostream>
#include <chrono>
#include <cstdio>

static bool persianResourcesInitialized = false;

// Original signature — wraps the config-based version with default paths
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
     std::string &normalizedString, std::string &ipaString) 
{
    NormalizeConfig config;
    config.espeak_data_path = "";         // empty → use default
    config.shakkelha_onnx = "./assets/shakkelha.onnx";
    config.ezafe_model_onnx = "./assets/ezafe_model.onnx";
    config.ezafe_model_spiece = "./assets/ezafe_spiece.model";
    config.hazm_words = "./assets/hazm_words.dat";
    config.hazm_verbs = "./assets/hazm_verbs.dat";
    config.hazm_stopwords = "./assets/hazm_stopwords.dat";
    config.homograph_data = "./assets/homograph_data.json";
    normalizeString(mainlang, ipa_mode, input, normalizedString, ipaString, config);
}

// Config-based version
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
     std::string &normalizedString, std::string &ipaString,
     const NormalizeConfig& config) 
{
    using clock = std::chrono::high_resolution_clock;
    using ms = std::chrono::duration<double, std::milli>;

    auto t_total = clock::now();

    const char* data_path = config.espeak_data_path.empty() ? NULL : config.espeak_data_path.c_str();
    piper::eSpeakPhonemeConfig phonemeConfig;
    piper::PhonemeIdConfig idConfig;
    std::vector <std::vector <char32_t>> phonemes;

    // --- replacements + detection ---
    auto t0 = clock::now();
    normalizedString = performGeneralReplacements(mainlang, input);
    LanguageDetector detector(mainlang);
    std::vector<DetectedSegment> text_segments = detector.detect_segments(normalizedString);
    auto t1 = clock::now();

    double espeak_ms = 0, postprocess_ms = 0, espeak_init_ms = 0, pers_init_ms = 0, lang_repl_ms = 0;
    
    for(auto& text_segment : text_segments) {
        auto &segment_text = text_segment.text;
        auto &language = text_segment.language;
        std::string phoneme_segment;
        
        auto t_lr0 = clock::now();
        segment_text = performLanguageSpecificReplacements(language, mainlang, segment_text);
        lang_repl_ms += ms(clock::now() - t_lr0).count();

        if(language == Language::ARABIC) {
            int err = shakkelha_initialize(config.shakkelha_onnx.c_str());
            if(err!=0) { std::cout << "Shakkelha model not found!" << std::endl; return; }
            segment_text = shakkelha_diacritize(segment_text);
            if(ipa_mode) { phoneme_segment = convertArabicToIPA(segment_text); ipaString += phoneme_segment; }
        } else {
            auto t_espeak_init0 = clock::now();
            int err = piper::phonemize_eSpeak_Init(data_path);
            espeak_init_ms += ms(clock::now() - t_espeak_init0).count();

            if(err!=0) { std::cout << "Espeak data folder not found!" << std::endl; return; }
            phonemes.clear();
            phonemeConfig.voice = (language == Language::PERSIAN) ? "fa" : "en-us";

            if(ipa_mode) {
                auto t_esp = clock::now();
                piper::phonemize_eSpeak(segment_text, phonemeConfig, phonemes); 
                phoneme_segment = piper::getIpaString(phonemes);
                espeak_ms += ms(clock::now() - t_esp).count();

                if(language == Language::PERSIAN) {
                    auto t_pers_init0 = clock::now();
                    if(!persianResourcesInitialized) {
                        persian_phoneme::initPersianResources(
                            config.ezafe_model_onnx.c_str(),
                            config.ezafe_model_spiece.c_str(),
                            config.hazm_words.c_str(),
                            config.hazm_verbs.c_str(),
                            config.hazm_stopwords.c_str(),
                            config.homograph_data.c_str()
                        );
                        persianResourcesInitialized = true;
                    }
                    pers_init_ms += ms(clock::now() - t_pers_init0).count();

                    auto t_post0 = clock::now();
                    phoneme_segment = persian_phoneme::postprocessPersianIPA(segment_text, phoneme_segment);
                    postprocess_ms += ms(clock::now() - t_post0).count();
                }
                ipaString += phoneme_segment;
            }
        }
    }

    normalizedString = LanguageDetector::reunite_segments(text_segments);

    double total_ms = ms(clock::now() - t_total).count();
    double repl_detect_ms = ms(t1 - t0).count();
    double compute_ms = repl_detect_ms + lang_repl_ms + espeak_ms + postprocess_ms;
    double load_ms = espeak_init_ms + pers_init_ms;

    fprintf(stderr, "[NORM] total=%.1fms  load=%.1fms  compute=%.1fms  repl=%.1f  lang_repl=%.1f  espeak=%.1f  postproc=%.1f\n",
            total_ms, load_ms, compute_ms, repl_detect_ms, lang_repl_ms, espeak_ms, postprocess_ms);
}
