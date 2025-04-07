#include "replacement.h"

#include "language_detector/language_detector.h"
#include "language-specific-codes/arabic/normalize_numbers_arabic.h"
#include "language-specific-codes/english/normalize_numbers_english.h"
#include "language-specific-codes/persian/normalize_numbers_persian.h"

#include <regex>
#include <codecvt>
#include <locale>

static const std::unordered_map<std::string, std::string> utfMap = {
    // Arabic digits (٠-٩)
    {"\xD9\xA0", "0"},  // ٠ (U+0660)
    {"\xD9\xA1", "1"},  // ١ (U+0661)
    {"\xD9\xA2", "2"},  // ٢ (U+0662)
    {"\xD9\xA3", "3"},  // ٣ (U+0663)
    {"\xD9\xA4", "4"},  // ٤ (U+0664)
    {"\xD9\xA5", "5"},  // ٥ (U+0665)
    {"\xD9\xA6", "6"},  // ٦ (U+0666)
    {"\xD9\xA7", "7"},  // ٧ (U+0667)
    {"\xD9\xA8", "8"},  // ٨ (U+0668)
    {"\xD9\xA9", "9"},  // ٩ (U+0669)

    // Persian digits (۰-۹)
    {"\xDB\xB0", "0"},  // ۰ (U+06F0)
    {"\xDB\xB1", "1"},  // ۱ (U+06F1)
    {"\xDB\xB2", "2"},  // ۲ (U+06F2)
    {"\xDB\xB3", "3"},  // ۳ (U+06F3)
    {"\xDB\xB4", "4"},  // ۴ (U+06F4)
    {"\xDB\xB5", "5"},  // ۵ (U+06F5)
    {"\xDB\xB6", "6"},  // ۶ (U+06F6)
    {"\xDB\xB7", "7"},  // ۷ (U+06F7)
    {"\xDB\xB8", "8"},  // ۸ (U+06F8)
    {"\xDB\xB9", "9"},  // ۹ (U+06F9)

    // Decimal separator
    {"\xD9\xAB", "."}   // ٫ (U+066B)
};

// Table of UTF-8 sequences to remove (add more as needed)
const std::vector<std::string> UNWANTED_UNICODE_CHARS = {
    "\xE2\x80\x8F",   // U+200F (RIGHT-TO-LEFT MARK, RLM)
    "\xE2\x81\xA8",   // U+2068 (FIRST STRONG ISOLATE, FSI)
    "\xE2\x81\xA9",   // U+2069 (POP DIRECTIONAL ISOLATE, PDI)
    "\xE2\x80\xAE",   // U+202E (RIGHT-TO-LEFT OVERRIDE, RLO)
    "\xE2\x80\xAD",   // U+202D (LEFT-TO-RIGHT OVERRIDE, LRO)
    "\xE2\x80\xAB",   // U+202B (RIGHT-TO-LEFT EMBEDDING, RLE)
    "\xE2\x80\xAA",   // U+202A (LEFT-TO-RIGHT EMBEDDING, LRE)
    "\xE2\x80\xAC",   // U+202C (POP DIRECTIONAL FORMATTING, PDF)
    "\xEF\xBB\xBF",   // U+FEFF (ZERO WIDTH NO-BREAK SPACE, BOM)
    "\xE2\x80\x8E",   // U+200E (LEFT-TO-RIGHT MARK, LRM)
    // Add more here...
};

static const std::unordered_map<std::string, std::string> GENERAL_WORD_REPLACEMENTS = {
    {"mrs", "misess"},
    {"mr", "mister"},
    {"dr", "doctor"},
    {"st", "saint"},
    {"co", "company"},
    {"jr", "junior"},
    {"maj", "major"},
    {"gen", "general"},
    {"drs", "doctors"},
    {"rev", "reverend"},
    {"lt", "lieutenant"},
    {"hon", "honorable"},
    {"sgt", "sergeant"},
    {"capt", "captain"},
    {"esq", "esquire"},
    {"ltd", "limited"},
    {"col", "colonel"},
    {"ft", "fort"},

    {"bit/s", "bits per second"},
    {"kbit/s", "kilo bits per second"},
    {"mbit/s", "mega bits per second"},
    {"gbit/s", "giga bits per second"},

    {"mp3", " m p 3"},
    {"csv", " c s v"},
    {"apk", " a p k"},

};

static const std::unordered_map<std::string, std::string> ARABIC_WORD_REPLACEMENTS = {
    {" ص ", " صباحاً "},
    {" م ", " مساءً "},
};

static const std::unordered_map<std::string, std::string> PERSIAN_WORD_REPLACEMENTS = {
    {"ص.", "صفحه"},
};

std::vector<std::string> split(const std::string& s, char delimiter);
std::string join(const std::vector<std::string>& vec, char delimiter);
std::string factorizeChineseLetters(const std::string& input);
std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& str);
std::string toLower(const std::string& str);
void replaceEnglishSpecificAbreviations(std::string& result);
void replaceArabicSpecificAbreviations(std::string& result);
void replacePersianSpecificAbreviations(std::string& result);

std::string performReplacements(const std::string& input) {
    std::string result = input;

    // Replace each character sequence in the map
    for (const auto& pair : utfMap) {
        size_t pos = 0;
        while ((pos = result.find(pair.first, pos)) != std::string::npos) {
            result.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length();
        }
    }

    // Simply remove all unwanted characters
    for (const auto& seq : UNWANTED_UNICODE_CHARS) {
        size_t pos = 0;
        while ((pos = result.find(seq, pos)) != std::string::npos) {
            result.erase(pos, seq.length());  // Just remove the characters
            // Don't increment pos since we removed characters
        }
    }

    // B replacement
    result = std::regex_replace(result, std::regex("(^|\\s)B(\\b|\\s)"), "$1Byte$2");
    result = std::regex_replace(result, std::regex("([۰-۹0-9])B(\\b|\\s)"), "$1 Byte$2");
       
    // KB replacement
    result = std::regex_replace(result, std::regex("(^|\\s)KB(\\b|\\s)"), "$1Kilo Byte$2");
    result = std::regex_replace(result, std::regex("([۰-۹0-9])KB(\\b|\\s)"), "$1 Kilo Byte$2");
    
    // MB replacement
    result = std::regex_replace(result, std::regex("(^|\\s)MB(\\b|\\s)"), "$1Mega Byte$2");
    result = std::regex_replace(result, std::regex("([۰-۹0-9])MB(\\b|\\s)"), "$1 Mega Byte$2");
    
    // GB replacement
    result = std::regex_replace(result, std::regex("(^|\\s)GB(\\b|\\s)"), "$1Giga Byte$2");
    result = std::regex_replace(result, std::regex("([۰-۹0-9])GB(\\b|\\s)"), "$1 Giga Byte$2");
    
    // TB replacement
    result = std::regex_replace(result, std::regex("(^|\\s)TB(\\b|\\s)"), "$1Tera Byte$2");
    result = std::regex_replace(result, std::regex("([۰-۹0-9])TB(\\b|\\s)"), "$1 Tera Byte$2");      
    
    // Replace capital letter sequences with spaced versions
    // Add a space before Capital letters in the middle of a word(They are probably a separate word shown in camelCase)
    std::regex lowercaseToCapital("([a-z])([A-Z])");
    result = std::regex_replace(result, lowercaseToCapital, "$1 $2");

    std::regex abbrevEnd("([a-z])([A-Z]{2,})(\\b|$)");
    result = std::regex_replace(result, abbrevEnd, "$1 $2$3");

    // 2. Process all capital sequences
    std::regex capitalLetters("([A-Z]{2,})");
    std::string temp;
    std::sregex_iterator it(result.begin(), result.end(), capitalLetters);
    std::sregex_iterator end;

    size_t last_pos = 0;
    for (; it != end; ++it) {
        temp += result.substr(last_pos, it->position() - last_pos);
        std::string letters = (*it)[1].str();
        
        // Space out the abbreviation
        for (size_t i = 0; i < letters.size(); ++i) {
            if (i != 0) temp += " ";
            temp += letters[i];
        }
        
        last_pos = it->position() + it->length();
    }
    temp += result.substr(last_pos);
    result = temp;

    // Remove ALL "/" between numbers (used for date) (multiple passes if needed)
    bool changed;
    do {
        changed = false;
        std::regex slashBetweenNumbers("([٠١٢٣٤٥٦٧٨٩0-9]+)/([٠١٢٣٤٥٦٧٨٩0-9]+)");
        std::string new_result = std::regex_replace(result, slashBetweenNumbers, "$1 $2");
        if (new_result != result) {
            changed = true;
            result = new_result;
        }
    } while (changed);

    // Remove ALL ":" between numbers (used for date) (multiple passes if needed)
    do {
        changed = false;
        std::regex slashBetweenNumbers("([٠١٢٣٤٥٦٧٨٩0-9]+):([٠١٢٣٤٥٦٧٨٩0-9]+)");
        std::string new_result = std::regex_replace(result, slashBetweenNumbers, "$1 $2");
        if (new_result != result) {
            changed = true;
            result = new_result;
        }
    } while (changed);

    // CJK character replacement
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring wide = converter.from_bytes(result);
        std::wregex cjkChars(L"[\\u4E00-\\u9FFF]");
        wide = std::regex_replace(wide, cjkChars, L" chinese letter ");
        result = converter.to_bytes(wide);
    } catch (...) {
        std::regex cjkChars("[\\x{4E00}-\\x{9FFF}]");
        result = std::regex_replace(result, cjkChars, " chinese letter ");
    }

    // Factorize N " chinese letter " to "N chinese letters"
    result = factorizeChineseLetters(result);

    // Perform word replacements
    replaceEnglishSpecificAbreviations(result);

    // convert to lowercase
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    //Detect language for language specific parts
    LanguageDetector detector(Language::ARABIC);
    std::vector<DetectedSegment> text_segments = detector.detect_segments(result);
    for(auto& text_segment : text_segments){
        auto &result = text_segment.text;
        auto &language = text_segment.language;

        //For test:
        //result = result + "(" + LanguageDetector::language_to_string(language) + ")";

        if(language == Language::ENGLISH) {
            result = EnglishNumberConverter::to_english_text(result);
        } else if(language == Language::PERSIAN) {
            result = PersianNumberConverter::to_persian_text(result);
            replacePersianSpecificAbreviations(result);

        } else { //(language == Language::ARABIC) {
            replaceArabicSpecificAbreviations(result);

            result = ArabicNumberConverter::to_arabic_text(result);
        }

    }

    result = LanguageDetector::reunite_segments(text_segments);

    return result;
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string join(const std::vector<std::string>& vec, char delimiter) {
    std::string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) result += delimiter;
        result += vec[i];
    }
    return result;
}

std::string factorizeChineseLetters(const std::string& input) {
    std::string result;
    size_t pos = 0;
    const std::string pattern = " chinese letter ";
    const size_t pattern_length = pattern.length();
    
    while (pos < input.length()) {
        size_t found = input.find(pattern, pos);
        if (found == std::string::npos) {
            result += input.substr(pos);
            break;
        }
        
        // Add the part before the match
        result += input.substr(pos, found - pos);
        
        // Count consecutive occurrences
        size_t count = 0;
        while (found != std::string::npos && 
               input.substr(found, pattern_length) == pattern) {
            count++;
            found += pattern_length;
        }
        
        // Add the factorized version
        if (count > 1) {
            result += std::string(" ") + std::to_string(count) + " chinese letters ";
        } else {
            result += pattern;
        }
        
        pos = found;
    }
    
    return result;
}

std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string wstring_to_utf8(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(str);
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return lowerStr;
}

void replaceEnglishSpecificAbreviations(std::string& result) {
    std::string lowerResult = toLower(result);
    size_t pos = 0;
    
    for (const auto& pair : GENERAL_WORD_REPLACEMENTS) {
        pos = 0;
        while ((pos = lowerResult.find(pair.first, pos)) != std::string::npos) {
            // Check if it's a whole word
            if ((pos == 0 || !isalpha(lowerResult[pos-1])) &&
                (pos + pair.first.length() == lowerResult.length() || !isalpha(lowerResult[pos + pair.first.length()]))) {
                // Replace with original case preserved where possible
                result.replace(pos, pair.first.length(), pair.second);
                lowerResult.replace(pos, pair.first.length(), pair.second);
            }
            pos += pair.first.length();
        }
    }
}

void replaceArabicSpecificAbreviations(std::string& result) {
    std::string lowerResult = toLower(result);
    size_t pos = 0;
    
    for (const auto& pair : ARABIC_WORD_REPLACEMENTS) {
        pos = 0;
        while ((pos = lowerResult.find(pair.first, pos)) != std::string::npos) {
            // Check if it's a whole word
            if ((pos == 0 || !isalpha(lowerResult[pos-1])) &&
                (pos + pair.first.length() == lowerResult.length() || !isalpha(lowerResult[pos + pair.first.length()]))) {
                // Replace with original case preserved where possible
                result.replace(pos, pair.first.length(), pair.second);
                lowerResult.replace(pos, pair.first.length(), pair.second);
            }
            pos += pair.first.length();
        }
    }
}

void replacePersianSpecificAbreviations(std::string& result) {
    std::string lowerResult = toLower(result);
    size_t pos = 0;
    
    for (const auto& pair : PERSIAN_WORD_REPLACEMENTS) {
        pos = 0;
        while ((pos = lowerResult.find(pair.first, pos)) != std::string::npos) {
            // Check if it's a whole word
            if ((pos == 0 || !isalpha(lowerResult[pos-1])) &&
                (pos + pair.first.length() == lowerResult.length() || !isalpha(lowerResult[pos + pair.first.length()]))) {
                // Replace with original case preserved where possible
                result.replace(pos, pair.first.length(), pair.second);
                lowerResult.replace(pos, pair.first.length(), pair.second);
            }
            pos += pair.first.length();
        }
    }
}