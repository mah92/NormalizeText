#include "./normalize.h"
#include "language_detector/language_detector.h"
#include "replacements/replacements.h"
#include "espeak_phonemize/phonemize.hpp"
#include "nawar-halabi/ar_phonemizer.h"

void normalizeString(Language mainlang, const std::string& input,
     std::string &normalizedString, std::vector <std::vector <char32_t>> &phonemes, std::string &phonemeString, std::string &idString) 
{
    const char* data_path = NULL;
    piper::eSpeakPhonemeConfig phonemeConfig;
    piper::PhonemeIdConfig idConfig;

    normalizedString = performGeneralReplacements(mainlang, input);

    LanguageDetector detector(mainlang);

    std::vector<DetectedSegment> text_segments = detector.detect_segments(normalizedString);
    
    for(auto& text_segment : text_segments) {
        auto &segment_text = text_segment.text;
        auto &language = text_segment.language;
        
        // Language-specific processing
        segment_text = performLanguageSpecificReplacements(language, mainlang, segment_text);

        if(language == Language::ARABIC) {
            //phonemes
        } else {
            piper::phonemize_eSpeak_Init(data_path);

            phonemes.clear();

            if(language == Language::PERSIAN)
                phonemeConfig.voice = "fa";
            else //English
                phonemeConfig.voice = "en-us";

            piper::phonemize_eSpeak(segment_text, phonemeConfig, phonemes); 
        
            phonemeString = piper::getPhonemeString(phonemes);

            idString = piper::getIdString(phonemes, idConfig);

            //piper::phonemize_eSpeak_Terminate();
        }
    }

    normalizedString = LanguageDetector::reunite_segments(text_segments);
}
