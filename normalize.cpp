#include "./normalize.h"
#include "language_detector/language_detector.h"
#include "replacements/replacements.h"
#include "espeak_phonemize/phonemize.hpp"
#include "shakkelha/shakkelha.h"
#include "nawar-halabi/ar_phonemizer.h"

#include <iostream>

void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
     std::string &normalizedString, std::string &ipaString) 
{
    const char* data_path = NULL;
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
        
        // Language-specific processing
        segment_text = performLanguageSpecificReplacements(language, mainlang, segment_text);

        if(language == Language::ARABIC) {

            int err = shakkelha_initialize("./shakkelha.onnx");
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
            else //English
                phonemeConfig.voice = "en-us";

            if(ipa_mode) {
                piper::phonemize_eSpeak(segment_text, phonemeConfig, phonemes); 
                phoneme_segment = piper::getIpaString(phonemes);
                ipaString +=phoneme_segment;
            }

            //piper::phonemize_eSpeak_Terminate();
        }
    }

    normalizedString = LanguageDetector::reunite_segments(text_segments);
}
