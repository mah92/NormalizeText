#include "replacement.h"
#include "language_detector/language_detector.h"
#include "language-specific-codes/arabic/normalize_numbers_arabic.h"
#include "language-specific-codes/english/normalize_numbers_english.h"
#include "language-specific-codes/persian/normalize_numbers_persian.h"

#include "language-specific-codes/arabic/arabic_symbols.h"
#include "language-specific-codes/persian/persian_symbols.h"

#include <regex>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <vector>

// UTF-8 character replacements
static const std::unordered_map<std::string, std::string> ARABIC_NUMBER_REPLACEMENTS = {
    // Arabic digits (٠-٩)
    {"\xD9\xA0", "0"}, {"\xD9\xA1", "1"}, {"\xD9\xA2", "2"}, 
    {"\xD9\xA3", "3"}, {"\xD9\xA4", "4"}, {"\xD9\xA5", "5"},
    {"\xD9\xA6", "6"}, {"\xD9\xA7", "7"}, {"\xD9\xA8", "8"},
    {"\xD9\xA9", "9"},

    // Persian digits (۰-۹)
    {"\xDB\xB0", "0"}, {"\xDB\xB1", "1"}, {"\xDB\xB2", "2"},
    {"\xDB\xB3", "3"}, {"\xDB\xB4", "4"}, {"\xDB\xB5", "5"},
    {"\xDB\xB6", "6"}, {"\xDB\xB7", "7"}, {"\xDB\xB8", "8"},
    {"\xDB\xB9", "9"},

    // Decimal separator
    {"\xD9\xAB", "."}
};

// Characters to remove
const std::vector<std::string> UNWANTED_UNICODE_CHARS = {
    "\xE2\x80\x8F", "\xE2\x81\xA8", "\xE2\x81\xA9", "\xE2\x80\xAE",
    "\xE2\x80\xAD", "\xE2\x80\xAB", "\xE2\x80\xAA", "\xE2\x80\xAC",
    "\xEF\xBB\xBF", "\xE2\x80\x8E"
};

// =============== REPLACEMENT TABLES ===============

// Whole word replacements for all languages
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_ALL = {

};

// Whole word replacements for Arabic
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_ARABIC = {
    //{" ص ", " صباحاً "},
    //{" م ", " مساءً "},
    //{"أ", "ألف"},
    //{"ب", "باء"},
};

// Whole word replacements for English
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_ENGLISH = {
    {"approx", "approximately"},
    {"etc", "et cetera"},
    {"e.g", "for example"},
    {"i.e", "that is"},

    //Honorifics
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

    {"bit/s" , "bits per second"},
    {"kbit/s", "kilo bits per second"},
    {"mbit/s", "mega bits per second"},
    {"gbit/s", "giga bits per second"},
    {"tbit/s", "tera bits per second"},
   
    {"b/s" , "bits per second"},
    {"kb/s", "kilo bits per second"},
    {"mb/s", "mega bits per second"},
    {"gb/s", "giga bits per second"},
    {"tb/s", "tera bits per second"},

    {"bps" , "bits per second"},
    {"kbps", "kilo bits per second"},
    {"mbps", "mega bits per second"},
    {"gbps", "giga bits per second"},
    {"tbps", "tera bits per second"},

    //{"am", "A M"},
    {"pm", "p m"},

    {"c++", "c plus plus"},
    {"c#", "c sharp"},

    // File Extensions
    {"txt" , " t x t"},
    {"doc" , " duck"},
    {"docx", " duck x"},
    {"pdf" , " p d f"},
    {"rtf" , " r t f"},
    {"odt" , " o d t"},
    {"ppt" , " p p t"},
    {"pptx", " p p t x"},
    {"xls" , " x l s"},
    {"xlsx", " x l s x"},
    {"csv" , " c s v"},
    {"jpg" , " j p g"},
    {"jpeg", " j peg"},
    {"png" , " p n g"},
    {"bmp" , " b m p"},
    {"svg" , " s v g"},
    {"webp", " web p"},
    {"aac" , " a a c"},
    {"ogg" , " o g g"},
    {"m4a" , " m 4 a"},
    {"mp3" , " m p 3"},
    {"mp4" , " m p 4"},
    {"avi" , " a v i"},
    {"mkv" , " m k v"},
    {"wmv" , " w m v"},
    {"flv" , " f l v"},
    {"webm", " web m"},
    {"7z"  , " seven zip"},
    {"gz"  , " g z"},
    {"msi" , " m s i"},
    {"dll" , " d l l"},
    {"sh"  , " s h"},
    {"apk" , " a p k"},
    {"html", " h t m l"},
    {"htm" , " h t m"},
    {"css" , " c s s"},
    {"js"  , " j s"},
    {"cpp" , " c p p"},
    {"php" , " p h p"},
    {"json", " j son"},
    {"xml" , " x m l"},
    {"sql" , " s q l"},
    {"db"  , " d b"},
    {"mdb" , " m d b"},
    {"sqlite", " s q lite"},
    {"accdb", " a c c d b"},

    {"qwerty", " q verty"},
    
    //Internet
    {"http", "h t t p"},
    {"https", "h t t p s"},
    {"www", "w w w"},
    {"ftp", "f t p"},
};

// Whole word replacements for Persian
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_PERSIAN = {
    //{"ص", "صفحه"},
    //{"ش", "شماره"},
    //{"ج", "جلد"},
    //{"ک", "کوچه"},
    //{"خ", "خیابان"},
};

// Normal replacements (anywhere in string) for all languages
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_ALL = {

};

// Normal replacements for Arabic
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_ARABIC = {
    {"(ص)", "صلى الله عليه و آله"},
    {"(ع)", "علیه السلام"},
    {"ﷻ", "جل جلاله"},

    {"@", " في "},
    {"#", " شباك "},
    {"%", " بالمئة"},
    {"^", " مرفوع للقوة "},
    {"&", " و "},
    {"*", " مضروب في "},
    {"+", " زائد "},
    {"/", " مقسوم على "},
    {"=", " يساوي "},
};

// Normal replacements for English
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_ENGLISH = {
    {"w/", "with"},
    {"w/o", "without"},
    {"b/c", "because"},

    {"@", " at sign "},
    {"#", " hash "},
    {"%", " percent "},
    {"^", " power "},
    {"&", " and "},
    {"*", " multiply "},
    {"+", " plus "},
    {"/", " slash "},
    {"=", " equals "},
};

// Normal replacements for Persian
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_PERSIAN = {
    {"ۀ", "ه ی"},
    {"ة", "ت"},
    {"ك", "ک"},

    {"@", " اَت ساین "},
    {"#", " هَش تگ "},
    {"%", " درصد "},
    {"^", " به توان "},
    {"&", " and "},
    {"*", " ضرب در "},
    {"+", " به علاوه ی "},
    {"/", " تقسیم بر "},
    {"=", " مساوی "},  
};

void doArabicSpecificReplacements(std::string &segment_text)
{
    std::regex pattern1("(\\s)ص");
    std::string replacement1 = "$1صباحاً";
    segment_text = std::regex_replace(segment_text, pattern1, replacement1);

    std::regex pattern2("(\\s)م");
    std::string replacement2 = "$1مساءً";
    segment_text = std::regex_replace(segment_text, pattern2, replacement2);
}

// =============== HELPER FUNCTIONS ===============
std::vector<std::string> split(const std::string& s, char delimiter);
std::string join(const std::vector<std::string>& vec, char delimiter);
std::string toLower(const std::string& str);
std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& str);
std::string factorizeChineseLetters(const std::string& input);
void applyWholeWordReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements);
void applyNormalReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements);

// =============== MAIN REPLACEMENT FUNCTION ===============

std::string performReplacements(const std::string& input) {
    std::string result = input;

    // Remove unwanted characters
    for (const auto& seq : UNWANTED_UNICODE_CHARS) {
        size_t pos = 0;
        while ((pos = result.find(seq, pos)) != std::string::npos) {
            result.erase(pos, seq.length());
        }
    }

    // Apply universal normal replacements
    applyNormalReplacements(result, NORMAL_REPLACEMENTS_ALL);

    // Replace Arabic numbers
    applyNormalReplacements(result, ARABIC_NUMBER_REPLACEMENTS);

    // Apply universal whole word replacements
    applyWholeWordReplacements(result, WHOLE_WORD_REPLACEMENTS_ALL);

    // Byte unit replacements
    result = std::regex_replace(result, std::regex("(^|\\s)B(\\b|\\s)"), "$1Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])B(\\b|\\s)"), "$1 Byte$2");
    result = std::regex_replace(result, std::regex("(^|\\s)KB(\\b|\\s)"), "$1Kilo Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])KB(\\b|\\s)"), "$1 Kilo Byte$2");
    result = std::regex_replace(result, std::regex("(^|\\s)MB(\\b|\\s)"), "$1Mega Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])MB(\\b|\\s)"), "$1 Mega Byte$2");
    result = std::regex_replace(result, std::regex("(^|\\s)GB(\\b|\\s)"), "$1Giga Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])GB(\\b|\\s)"), "$1 Giga Byte$2");
    result = std::regex_replace(result, std::regex("(^|\\s)TB(\\b|\\s)"), "$1Tera Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])TB(\\b|\\s)"), "$1 Tera Byte$2");

    result = std::regex_replace(result, std::regex("(^|\\s)b(\\b|\\s)"), "$1bit$2");
    result = std::regex_replace(result, std::regex("([0-9])b(\\b|\\s)"), "$1 bit$2");
    result = std::regex_replace(result, std::regex("(^|\\s)Kb(\\b|\\s)"), "$1Kilo bit$2");
    result = std::regex_replace(result, std::regex("([0-9])Kb(\\b|\\s)"), "$1 Kilo bit$2");
    result = std::regex_replace(result, std::regex("(^|\\s)Mb(\\b|\\s)"), "$1Mega bit$2");
    result = std::regex_replace(result, std::regex("([0-9])Mb(\\b|\\s)"), "$1 Mega bit$2");
    result = std::regex_replace(result, std::regex("(^|\\s)Gb(\\b|\\s)"), "$1Giga bit$2");
    result = std::regex_replace(result, std::regex("([0-9])Gb(\\b|\\s)"), "$1 Giga bit$2");
    result = std::regex_replace(result, std::regex("(^|\\s)Tb(\\b|\\s)"), "$1Tera bit$2");
    result = std::regex_replace(result, std::regex("([0-9])Tb(\\b|\\s)"), "$1 Tera bit$2");

    // Handle camelCase and abbreviations
    // "doTheABC" -> "do The ABC"
    result = std::regex_replace(result, std::regex("([a-z])([A-Z])"), "$1 $2");
    result = std::regex_replace(result, std::regex("([a-z])([A-Z]{2,})(\\b|$)"), "$1 $2$3");

    // Process capital sequences (2-4 letters only, longer sequences stay unchanged)
    // "ABCD" -> "A B C D"

    std::regex capitalLetters(R"(\b([A-Z]{2,4})(?![A-Z]))");  // Uses word boundary instead of lookbehind
    std::string temp;
    std::sregex_iterator it(result.begin(), result.end(), capitalLetters);
    std::sregex_iterator end;
    size_t last_pos = 0;

    for (; it != end; ++it) {
        temp += result.substr(last_pos, it->position() - last_pos);
        std::string letters = (*it)[1].str();
        for (size_t i = 0; i < letters.size(); ++i) {
            if (i != 0) temp += " ";
            temp += letters[i];
        }
        last_pos = it->position() + it->length();
    }
    temp += result.substr(last_pos);
    result = temp;

    // Remove number separators
    // 204:20 -> 04 20
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

    // 02/28/2025 -> 02 28 2025
    do {
        changed = false;
        std::regex colonBetweenNumbers("([٠١٢٣٤٥٦٧٨٩0-9]+):([٠١٢٣٤٥٦٧٨٩0-9]+)");
        std::string new_result = std::regex_replace(result, colonBetweenNumbers, "$1 $2");
        if (new_result != result) {
            changed = true;
            result = new_result;
        }
    } while (changed);

    // CJK character replacement
    // "指事"-> "chinese letter  chinese letter"
    try {
        std::wstring wide = utf8_to_wstring(result);
        std::wregex cjkChars(L"[\\u4E00-\\u9FFF]");
        wide = std::regex_replace(wide, cjkChars, L" chinese letter ");
        result = wstring_to_utf8(wide);
    } catch (...) {
        std::regex cjkChars("[\\x{4E00}-\\x{9FFF}]");
        result = std::regex_replace(result, cjkChars, " chinese letter ");
    }

    // Factorize Chinese letters
    // " chinese letter  chinese letter " -> "2 chinese letters"
    result = factorizeChineseLetters(result);

    // CONVERT TO LOWERCASE ///////////////////////////////////
    // ABCDEFG -> abcdefg
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });

    // Language-specific processing
    Language mainlang = Language::ARABIC; //Main language
    LanguageDetector detector(mainlang);

    std::vector<DetectedSegment> text_segments = detector.detect_segments(result);
    
    for(auto& text_segment : text_segments) {
        auto &segment_text = text_segment.text;
        auto &language = text_segment.language;

        switch (language) {
            case Language::ARABIC:
            applyNormalReplacements(segment_text, NORMAL_REPLACEMENTS_ARABIC);
            applyNormalReplacements(segment_text, ARABIC_SYMBOL_REPLACEMENTS);
            applyWholeWordReplacements(segment_text, WHOLE_WORD_REPLACEMENTS_ARABIC);
            segment_text = ArabicNumberConverter::to_arabic_text(segment_text);

            doArabicSpecificReplacements(segment_text);
            break;

            case Language::PERSIAN:
            applyNormalReplacements(segment_text, NORMAL_REPLACEMENTS_PERSIAN);
            applyNormalReplacements(segment_text, PERSIAN_SYMBOL_REPLACEMENTS);
            applyWholeWordReplacements(segment_text, WHOLE_WORD_REPLACEMENTS_PERSIAN);
            segment_text = PersianNumberConverter::to_persian_text(segment_text);
            break;

            default:
            case Language::ENGLISH:
            //applyNormalReplacements(segment_text, );
            applyNormalReplacements(segment_text, NORMAL_REPLACEMENTS_ENGLISH);
            applyWholeWordReplacements(segment_text, WHOLE_WORD_REPLACEMENTS_ENGLISH);
            segment_text = EnglishNumberConverter::to_english_text(segment_text);
            break;
        }
    }


    result = LanguageDetector::reunite_segments(text_segments);

    return result;
}

// =============== HELPER FUNCTIONS ===============

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

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return lowerStr;
}

std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string wstring_to_utf8(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(str);
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
        
        result += input.substr(pos, found - pos);
        size_t count = 0;
        
        while (found != std::string::npos && 
               input.substr(found, pattern_length) == pattern) {
            count++;
            found += pattern_length;
        }
        
        if (count > 1) {
            result += " " + std::to_string(count) + " chinese letters ";
        } else {
            result += pattern;
        }
        
        pos = found;
    }
    
    return result;
}

void applyWholeWordReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements) {
    for (const auto& pair : replacements) {
        std::regex pattern("\\b" + pair.first + "\\b");
        result = std::regex_replace(result, pattern, pair.second);
    }
}

void applyNormalReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements) {
    for (const auto& pair : replacements) {
        size_t pos = 0;
        while ((pos = result.find(pair.first, pos)) != std::string::npos) {
            result.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length();
        }
    }
}
