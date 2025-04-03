// language_detector.h
#ifndef LANGUAGE_DETECTOR_H
#define LANGUAGE_DETECTOR_H

#include <vector>
#include <string>
#include <unordered_set>
#include <cstdint>

enum class Language {
    UNKNOWN,    // Unrecognized characters
    SEPARATOR,   // Whitespace and punctuation
    ENGLISH,    // Latin script and symbols
    PERSIAN,    // Persian script (includes some Arabic letters)
    ARABIC     // Arabic script (with specific diacritics)
};

struct DetectedSegment {
    std::string text;
    Language language;
};

class LanguageDetector {
public:
    struct TextSegment {
        std::vector<uint32_t> characters;  // UTF-32 code points
        bool is_separator;                 // True for separator sequences
    };

    explicit LanguageDetector(Language main_language = Language::PERSIAN);
    
    std::vector<Language> detect(const std::string &utf8_input) const;
    std::vector<DetectedSegment> detect_segments(const std::string &utf8_input) const;
    static std::string reunite_segments(const std::vector<DetectedSegment>& segments);
    
    static std::string language_to_string(Language lang) {
        switch(lang) {
            case Language::SEPARATOR: return "SEP";
            case Language::ENGLISH: return "EN";
            case Language::PERSIAN: return "FA";
            case Language::ARABIC: return "AR";
            default: return "UN";
        }
    }

private:
    static const std::unordered_set<uint32_t> separators;
    static const std::unordered_set<uint32_t> persian_reset_separators;
    static const std::unordered_set<uint32_t> arabic_diacritics;
    static const std::unordered_set<uint32_t> latin_letters;
    static const std::unordered_set<uint32_t> neutral_letters;
    static const std::unordered_set<uint32_t> persian_arabic_letters;
    static const std::unordered_set<uint32_t> digits;

    std::vector<uint32_t> utf8_to_codepoints(const std::string &utf8_str) const;
    std::string codepoint_to_utf8(uint32_t cp) const;
    std::vector<LanguageDetector::TextSegment> split_into_segments(const std::vector<uint32_t> &codepoints) const;
    std::vector<Language> process_word(const std::vector<uint32_t> &word, Language &current_context) const;
    std::vector<Language> process_persian_word(const std::vector<uint32_t> &word, Language &current_context) const;
    std::vector<Language> process_arabic_word(const std::vector<uint32_t> &word, Language &current_context) const;
    std::vector<Language> process_english_word(const std::vector<uint32_t> &word, Language &current_context) const;
    bool has_arabic_diacritics(const std::vector<uint32_t> &word) const;
    bool starts_with_al(const std::vector<uint32_t> &word) const;

    Language main_language_;
};

#endif // LANGUAGE_DETECTOR_H