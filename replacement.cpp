#include "replacement.h"
#include "language_detector/language_detector.h"
#include "language-specific-codes/arabic/normalize_numbers_arabic.h"
#include "language-specific-codes/english/normalize_numbers_english.h"
#include "language-specific-codes/persian/normalize_numbers_persian.h"

#include "language-specific-codes/arabic/arabic_symbols.h"
#include "language-specific-codes/persian/persian_symbols.h"

#include "language-specific-codes/general_replacements.h"
#include "language-specific-codes/arabic/arabic_replacements.h"
#include "language-specific-codes/english/english_replacements.h"
#include "language-specific-codes/persian/persian_replacements.h"

#include "replacementHelpers.h"

std::string performReplacements(Language mainlang, const std::string& input) {
    std::string result = input;

    // Remove unwanted characters
    // remove rtf, ltr, ...
    for (const auto& seq : UNWANTED_UNICODE_CHARS) {
        size_t pos = 0;
        while ((pos = result.find(seq, pos)) != std::string::npos) {
            result.erase(pos, seq.length());
        }
    }

    // Replace Arabic numbers and different forms of english letters to simple english
    // ۹ -> 9
    // "𝐀" -> "A"
    applyNormalReplacements(result, NORMAL_REPLACEMENTS_NO_SPACE_ALL);

    // Apply universal normal replacements
    // "ا…" -> "الله"
    // "α" -> "alpha"
    applyNormalReplacementsWithSpace(result, NORMAL_REPLACEMENTS_WITH_SPACE_ALL);

    // Apply universal whole word replacements
    // Empty
    //applyWholeWordReplacements(result, WHOLE_WORD_REPLACEMENTS_ALL);

    // Byte/bit unit replacements
    applyBitByteReplacements(result);

    // Handle camelCase abbreviations
    // "doTheABC" -> "do The ABC"
    seperateCamelCaseWords(result);

    // Process capital sequences (2-4 letters only, longer sequences stay unchanged)
    // "ABCD" -> "A B C D"
    seperateCapitalAbbreviations(result);

    // Remove number separators
    // 204:20 -> 04 20
    seperateClock(result);

    // 02/28/2025 -> 02 28 2025
    seperateDate(result);

    // CJK character replacement
    // "指事"-> "chinese letter  chinese letter"
    result = replace_cjk_with_placeholder(result);

    // Factorize Chinese letters
    // " chinese letter  chinese letter " -> "2 chinese letters"
    result = factorizeChineseLetters(result);

    // Main Language specific
    switch (mainlang) {
        case Language::ARABIC:
        // 🔑 -> مفتاح
        applyNormalReplacementsWithSpace(result, ARABIC_SYMBOL_REPLACEMENTS);
        break;

        case Language::PERSIAN:
        // 🔑 -> کلید
        applyNormalReplacementsWithSpace(result, PERSIAN_SYMBOL_REPLACEMENTS);
        break;

        default:
        case Language::ENGLISH:
        // 🔑 -> key
        //applyNormalReplacementsWithSpace(result, PERSIAN_SYMBOL_REPLACEMENTS);
        break;
    }

    // CONVERT TO LOWERCASE ///////////////////////////////////
    // ABCDEFG -> abcdefg
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });

    // Language-specific processing
    LanguageDetector detector(mainlang);

    std::vector<DetectedSegment> text_segments = detector.detect_segments(result);
    
    for(auto& text_segment : text_segments) {
        auto &segment_text = text_segment.text;
        auto &language = text_segment.language;

        switch (language) {
            case Language::ARABIC:
            // 8:54 م ->
            //   8:54 مساءً
            if(mainlang == Language::ARABIC) doArabicSpecificReplacements(segment_text); // Should be before ArabicNumberConverter and applyWholeWordReplacements, Causes Artifacts if called on english(?)
            // @ -> فی
            applyNormalReplacementsWithSpace(segment_text, NORMAL_REPLACEMENTS_ARABIC);
            // ا -> الف
            applyWholeWordReplacementsArabic(segment_text, WHOLE_WORD_REPLACEMENTS_ARABIC);

            if(mainlang != Language::PERSIAN) //Persian users don't work with arabic numbers
                segment_text = ArabicNumberConverter::normalize_text(segment_text);
            else
                segment_text = PersianNumberConverter::normalize_text(segment_text);
            break;

            case Language::PERSIAN:
            // ة
            applyNormalReplacements(segment_text, NORMAL_REPLACEMENTS_NO_SPACE_PERSIAN);
            // @
            applyNormalReplacementsWithSpace(segment_text, NORMAL_REPLACEMENTS_PERSIAN);
            // Empty
            applyWholeWordReplacements(segment_text, WHOLE_WORD_REPLACEMENTS_PERSIAN);

            segment_text = PersianNumberConverter::normalize_text(segment_text);
            break;

            default:
            case Language::ENGLISH:
            applyNormalReplacementsWithSpace(segment_text, NORMAL_REPLACEMENTS_ENGLISH);
            // @
            applyWholeWordReplacements(segment_text, WHOLE_WORD_REPLACEMENTS_ENGLISH);

            segment_text = EnglishNumberConverter::normalize_text(segment_text);
            break;
        }
    }


    result = LanguageDetector::reunite_segments(text_segments);

    return result;
}
