#include "./normalize.h"
#include "language_detector/language_detector.h"
#include "replacements/replacements.h"
#include "espeak_phonemize/phonemize.hpp"
#include "shakkelha/shakkelha.h"
#include "nawar-halabi/ar_phonemizer.h"
#include "persian_phoneme/persian_postprocess.h"

#include <iostream>

static bool persianResourcesInitialized = false;

// Original signature — wraps the config-based version with default paths
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
     std::string &normalizedString, std::string &ipaString) 
{
    NormalizeConfig config;
    config.espeak_data_path = "";         // empty → use default
    config.shakkelha_onnx = "./shakkelha.onnx";
    config.ezafe_model_onnx = "./ezafe_model/model_quantized.onnx";
    config.ezafe_model_spiece = "./ezafe_model/spiece.model";
    config.hazm_words = "./hazm_data/words.dat";
    config.hazm_verbs = "./hazm_data/verbs.dat";
    config.hazm_stopwords = "./hazm_data/stopwords.dat";
    config.homograph_data = "./homograph/homograph_data.json";
    normalizeString(mainlang, ipa_mode, input, normalizedString, ipaString, config);
}

// Config-based version — NO hardcoded paths
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
     std::string &normalizedString, std::string &ipaString,
     const NormalizeConfig& config) 
{
    const char* data_path = config.espeak_data_path.empty() ? NULL : config.espeak_data_path.c_str();
    piper::eSpeakPhonemeConfig phonemeConfig;
    piper::PhonemeIdConfig idConfig;
    std::vector <std::vector <char32_t>> phonemes;

    normalizedString = performGeneralReplacements(mainlang, input);

    LanguageDetector detector(mainlang);

    std::vector<DetectedSegment> text_segments = detector.detect_segments(normalizedString);
    
    for(auto& text_segment : text_segments) {
        auto &segment_text = text_segment.text;
        auto &language = text_segment.language;
        std::string phoneme_segment;
        
        segment_text = performLanguageSpecificReplacements(language, mainlang, segment_text);

        if(language == Language::ARABIC) {
            int err = shakkelha_initialize(config.shakkelha_onnx.c_str());
            if(err!=0) {
                std::cout << "Shakkelha model not found!" << std::endl;
                return;
            }
            segment_text = shakkelha_diacritize(segment_text);
            if(ipa_mode) {
                phoneme_segment = convertArabicToIPA(segment_text); 
                ipaString += phoneme_segment;
            }
        } else {
            int err = piper::phonemize_eSpeak_Init(data_path);
            if(err!=0) {
                std::cout << "Espeak data folder not found!" << std::endl;
                return;
            }
            phonemes.clear();
            if(language == Language::PERSIAN)
                phonemeConfig.voice = "fa";
            else
                phonemeConfig.voice = "en-us";

            if(ipa_mode) {
                piper::phonemize_eSpeak(segment_text, phonemeConfig, phonemes); 
                phoneme_segment = piper::getIpaString(phonemes);

                if(language == Language::PERSIAN) {
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
                    phoneme_segment = persian_phoneme::postprocessPersianIPA(
                        segment_text, phoneme_segment);
                }
                ipaString += phoneme_segment;
            }
        }
    }
    normalizedString = LanguageDetector::reunite_segments(text_segments);
}
