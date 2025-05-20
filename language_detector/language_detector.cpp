#include "language_detector.h"
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <string>
#include <vector>

// Initialize static character sets with proper Unicode code points
const std::unordered_set<uint32_t> LanguageDetector::separators = {
    '\n',      // Whitespace
    ';', '.', '!', '?', '"', '\'', '`', // ASCII punctuation
    '(', ')', '{', '}', '[', ']',  // Brackets
    0x061B, // Arabic semicolon (؛)
    0x060C, // Arabic comma (،)
    0x061F, // Arabic question mark (؟)
    0x00AB, // Left-pointing double angle quotation mark («)
    0x00BB  // Right-pointing double angle quotation mark (»)
};

// Arabic vowel diacritics that force Arabic detection
const std::unordered_set<uint32_t> LanguageDetector::arabic_diacritics = {
    0x064E, // ARABIC FATHA (َ)
    0x064F, // ARABIC DAMMA (ُ)
    //Not FATHATAN (ً)
    0x064D, // ARABIC KASRATAN (ٍ)
    0x064C, // ARABIC DAMMATAN (ٌ)
    0x0652  // ARABIC SUKUN (ْ)
    // Not Kasre (ِ)
};

const std::unordered_set<uint32_t> LanguageDetector::latin_letters = {
    // Basic Latin letters (both cases)
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
};

// Common symbols not within separators but don't change language context
const std::unordered_set<uint32_t> LanguageDetector::neutral_letters = {
    ' ', '\t', ',', ':', '@','#','$','%','^','&','*','_','-','+','~'
};

// Add digits set definition (Latin 0-9 and Arabic ۰-۹)
const std::unordered_set<uint32_t> LanguageDetector::digits = {
    '0','1','2','3','4','5','6','7','8','9',
    0x0660, 0x0661, 0x0662, 0x0663, 0x0664, 0x0665, 0x0666, 0x0667, 0x0668, 0x0669
};

// Persian and Arabic letters (Unicode code points)
const std::unordered_set<uint32_t> LanguageDetector::persian_arabic_letters = {
    // Arabic letters common to both languages
    0x0622, // ARABIC LETTER ALEF WITH MADDA ABOVE (آ)
    0x0627, // ARABIC LETTER ALEF (ا)
    0x0623, // ARABIC LETTER ALEF WITH HAMZA ABOVE (أ)
    0x0625, // ARABIC LETTER ALEF WITH HAMZA BELOW (إ)
    0x0626, // ARABIC LETTER YEH WITH HAMZA ABOVE (ئ)
    0x0624, // ARABIC LETTER WAW WITH HAMZA ABOVE (ؤ)
    0x0621, // ARABIC LETTER HAMZA (ء)
    0x0628, // ARABIC LETTER BEH (ب)
    
    // Persian-specific letters
    0x067E, // ARABIC LETTER PEH (پ) - Persian 'pe'
    0x062A, // ARABIC LETTER TEH (ت)
    0x062B, // ARABIC LETTER THEH (ث)
    0x062C, // ARABIC LETTER JEEM (ج)
    0x0686, // ARABIC LETTER TCHEH (چ) - Persian 'che'
    0x062D, // ARABIC LETTER HAH (ح)
    0x062E, // ARABIC LETTER KHAH (خ)
    0x062F, // ARABIC LETTER DAL (د)
    0x0630, // ARABIC LETTER THAL (ذ)
    0x0631, // ARABIC LETTER REH (ر)
    0x0632, // ARABIC LETTER ZAIN (ز)
    0x0698, // ARABIC LETTER JEH (ژ) - Persian 'zhe'
    0x0633, // ARABIC LETTER SEEN (س)
    0x0634, // ARABIC LETTER SHEEN (ش)
    0x0635, // ARABIC LETTER SAD (ص)
    0x0636, // ARABIC LETTER DAD (ض)
    0x0637, // ARABIC LETTER TAH (ط)
    0x0638, // ARABIC LETTER ZAH (ظ)
    0x0639, // ARABIC LETTER AIN (ع)
    0x063A, // ARABIC LETTER GHAIN (غ)
    0x0641, // ARABIC LETTER FEH (ف)
    0x0642, // ARABIC LETTER QAF (ق)
    0x06A9, // ARABIC LETTER KEHEH (ک) - Persian 'ke'
    0x0644, // ARABIC LETTER LAM (ل)
    0x0645, // ARABIC LETTER MEEM (م)
    0x0646, // ARABIC LETTER NOON (ن)
    0x0648, // ARABIC LETTER WAW (و)
    0x0647, // ARABIC LETTER HEH (ه)
    0x06CC, // ARABIC LETTER FARSI YEH (ی) - Persian 'ye'
    0x064A  // ARABIC LETTER YEH (ي) - Arabic 'ya'
};

// Constructor
LanguageDetector::LanguageDetector(Language main_language) : main_language_(main_language) {}

// Convert UTF-8 string to Unicode code points
std::vector<uint32_t> LanguageDetector::utf8_to_codepoints(const std::string &utf8_str) const {
    std::vector<uint32_t> codepoints;
    for (size_t i = 0; i < utf8_str.size();) {
        uint32_t cp = 0;
        uint8_t c = utf8_str[i];
        
        // 1-byte sequence (0xxxxxxx)
        if ((c & 0x80) == 0x00) {
            cp = c;
            i++;
        }
        // 2-byte sequence (110xxxxx)
        else if ((c & 0xE0) == 0xC0) {
            cp = ((utf8_str[i++] & 0x1F) << 6);
            cp |= (utf8_str[i++] & 0x3F);
        }
        // 3-byte sequence (1110xxxx)
        else if ((c & 0xF0) == 0xE0) {
            cp = ((utf8_str[i++] & 0x0F) << 12);
            cp |= ((utf8_str[i++] & 0x3F) << 6);
            cp |= (utf8_str[i++] & 0x3F);
        }
        // 4-byte sequence (11110xxx)
        else if ((c & 0xF8) == 0xF0) {
            cp = ((utf8_str[i++] & 0x07) << 18);
            cp |= ((utf8_str[i++] & 0x3F) << 12);
            cp |= ((utf8_str[i++] & 0x3F) << 6);
            cp |= (utf8_str[i++] & 0x3F);
        }
        // Invalid sequence (skip)
        else {
            i++;
            continue;
        }
        codepoints.push_back(cp);
    }
    return codepoints;
}

std::string LanguageDetector::codepoint_to_utf8(uint32_t cp) const {
    std::string result;
    
    if (cp <= 0x7F) {
        // 1-byte sequence
        result += static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        // 2-byte sequence
        result += static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        // 3-byte sequence
        result += static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0x10FFFF) {
        // 4-byte sequence
        result += static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
        result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    }
    // Invalid code points are skipped
    
    return result;
}

// Split text into segments (words and separators)
std::vector<LanguageDetector::TextSegment> LanguageDetector::split_into_segments(const std::vector<uint32_t> &codepoints) const {
    std::vector<TextSegment> segments;
    if (codepoints.empty()) return segments;

    TextSegment current_segment;
    current_segment.is_separator = separators.count(codepoints[0]);
    current_segment.characters.push_back(codepoints[0]);

    for (size_t i = 1; i < codepoints.size(); ++i) {
        uint32_t cp = codepoints[i];
        bool is_sep = separators.count(cp);
        
        if (current_segment.is_separator == is_sep) {
            current_segment.characters.push_back(cp);
        } else {
            segments.push_back(current_segment);
            current_segment.characters.clear();
            current_segment.is_separator = is_sep;
            current_segment.characters.push_back(cp);
        }
    }
    
    if (!current_segment.characters.empty()) {
        segments.push_back(current_segment);
    }
    
    return segments;
}

// Check if word contains Arabic diacritics
int LanguageDetector::arabic_diacritics_count(const std::vector<uint32_t> &word) const {
    int count = 0;
    for (uint32_t cp : word) {
        if (arabic_diacritics.count(cp)) count++;
    }
    return count;
}

static const std::unordered_set<std::u32string> AL_EXCEPTIONS = {
    U"الله",
    U"اللّه",
    U"الرحمن",
    U"الرّحمن",
    U"الرحیم", //در بسم الله الرحمن الرحیم
    U"الرّحیم", //در بسم الله الرحمن الرحیم
    U"الهی",
    U"الحسن",
    U"الحسین",
    U"الشهدا", //سید الشهدا
    U"الشّهدا", //سید الشهدا
    U"العابدین", //زین العابدین
    U"الباقر",
    U"العلوم", //باقر العلوم
    U"الصادق",
    U"الصّادق",
    U"الرضا",
    U"الرّضا",
    U"الکاظم",
    U"العسکری",
    U"العسگری",
    U"المهدی",
    U"الزمان", //صاحب الزمان
    U"الزّمان", //صاحب الزمان
    U"الاسلام", //حجت الاسلام
    U"السلام", //علیه السلام
    U"السّلام", //علیه السلام
    U"الدین",
    U"الدّین",
    U"آله",
    U" ال", //L
    U"الاصل", //ایرانی الاصل
    U"التأسیس", // جدیدالتأسیس
    U"المقدور", // حتی‌المقدور
    U"الدستور", // حسب‌الدستور
    U"الدّستور", // حسب‌الدستور
    U"الوکاله", // حق‌الوکاله
    U"الامر", // خاتم الامر
    U"الایتام", // دار الایتام
    U"الخمر", // دائم‌الخمر
    U"العمل", // دستورالعمل
    U"‌الانتشار", // کثیر‌الانتشار
    U"السابق", // کمافی‌السابق
    U"السّابق", // کمافی‌السابق
    U"التحریر", // لوازم‌التحریر
    U"التّحریر", // لوازم‌التحریر
    U"القدر", // لیله‌القدر
    U"الهویه", // مجهول‌الهویه
    U"الهوّیه", // مجهول‌الهویه
    U"الأسف", // مع‌الأسف
    U"الفارق", // مع‌الفارق
    U"القلب", // قسی‌القلب
    U"المثل", // فی‌المثل
    U"الحال", // فی‌الحال
    U"الفور", // فی‌الفور
    U"الجمله", // فی‌الجمله
    U"العاده", // فوق‌العاده
    U"الذکر", // فوق‌الذکر
    U"الذّکر", // فوق‌الذکر
    U"التحصیل", // فارغ‌التحصیل
    U"التّحصیل", // فارغ‌التحصیل
    U"المطالبه", // عندالمطالبه
    U"الزوم", // عندالزوم
    U"الّزوم", // عندالزوم
    U"الظاهر", // علی‌الظاهر
    U"الظّاهر", // علی‌الظاهر
    U"السویه", // علی‌السویه
    U"السّویه", // علی‌السویه
    U"الدوام", // علی‌الدوام
    U"الدّوام", // علی‌الدوام
    U"الحساب", // علی‌الحساب
    U"الجثه", // عظیم‌الجثه
    U"النفس", // ضعیف‌النفس
    U"النّفس", // ضعیف‌النفس
    U"العبور", // صعب‌العبور
    U"العلاج", // صعب‌العلاج
    U"الوصول", // سهل‌الوصول
    U"السیر", // سریع‌السیر
    U"الحجه", // ذی‌الحجه
    U"الحجّه", // ذی‌الحجه
    U"الاثر", // مفقود‌الاثر
    U"الکار", // ممنوع‌الکار
    U"العضو", // ناقص‌العضو
    U"المجموع", // حیث‌المجموع
    U"العین", // نصب‌العین
    U" الی",
    U"الو",
    U"الیگارشی",
    U"الحمد", //الحمدلله
    U"الست",
    U"الفاظ",
    U"الوان",
    U"الهه",
    U"الا ",
    U"الهام",
    U"العبد",
    U"الاف",
    U"الّاف",
    U"القاب",
    U"التیام",
    U"الحان",
    U"الزام",
    U"الک",
    U"التهاب",
    U"الساعه",
    U"الحاد",
    U"الاغ",
    U"الف ",
    U"الفبا",
    U"البسه",
    U"الفت",
    U"الگو",
    U"الگوریتم",
    U"الماس",
    U" الکی",
    U"البته",
    U"الان",
    U"الزام",
    U"الآن",
    U"الحق",
    U"الحاق",
    U"الغرض",
    U"التقاط",
    U"الحق",
    U"الملل", //بین الملل
    U"الاول", //جمادی الاول
    U"الاوّل", //جمادی الاول
    U"الثانی", //جمادی الثانی
    U"الثّانی", //جمادی الثانی
    U"الکتر", //الکترونیک - الکتریکی - الکتریسیته - الکترون
    U"النهار", //نصف النهار
    U"النّهار", //نصف النهار
    U"الاود", //رید الاود
    U"العجاب", //سریع العجابه
    U"العمل", //حق العمل
    U"الوصول", //سهل الوصول
    U"الاضلاع", //متوازی الاضلاع
    U"الساقین", //متساوی الساقین
    U"السّاقین", //متساوی الساقین
    U"القضات", //قاضی القضات
    U"الوصف", //زائد الوصف
    U"المسايل", //توضیح المسائل
    U"المسائل", //توضیح المسائل
    U"الوسایل", //تحریر الوسایل
    U"الوسائل", //تحریر الوسائل
    U"الوسیل", //تحریر الوسیله
    U"الکل",
    
    // Names
    U"الیکا",
    U"الیور",
    U"السا",
    U"الیسا",
    U"المیرا",
    U"النا",
    U"الناز",
    U"الیزابت",
    U"الیاس",
    U"البرز",
    U"الوند",
    U"البلاغه", //نهج البلاغه
    U"الاحادیث", //جامع الاحادیث
    U"النصره",
    U"النّصره",
    U"القاعده‌",
    U"الخناصر",
    U"السفیره",
    U"الکساندر",
    U"الغد",
    U"السیسی",
    U"الشیبانی",
    U"البیاضیه",
    U"الناقوره",
    U"العالم",
    U"المنار",
    U"المیادین",
    U"العربیه",
    U"الاهلی",
    U"الهلال",
    U"الریان",
    U"النصر",
    U"الناز",
    U"الحره",
    U"الحرّه",
    U"الحربی", //اعلام الحربی
    U"الخیام",
    U"الاقصی", // مسجد الاقصی
    U"المقدس", // بیت المقدس
    U"المقدّس", // بیت المقدس
    U"القدس",
    U"الشام",
    U"الشّام",
    U"البقاع",
    U"اللوبیا",
    U"القسام",
    U"القسّام",
    U"السنوار",
    U"الاسد",
    U"الجولانی",
    U"الجزیره",
    U"الشیخ", // جبل الشیخ
    U"الیرموک",
    U"العرب", //عین العرب
    U"البیاضیه",
    U"العدید",
    U"التنف",
    U"التّنف",
};

// Check if word starts with Arabic definite article "ال"
bool LanguageDetector::starts_with_al(const std::vector<uint32_t>& word) const {
    // First check if it starts with ال
    if (word.size() < 2 || word[0] != 0x0627 || word[1] != 0x0644) {
        return false;
    }

    // Convert the word to u32string for comparison
    std::u32string word_str(word.begin(), word.end());

    // Check if this word is in our exceptions list
    return AL_EXCEPTIONS.find(word_str) == AL_EXCEPTIONS.end();
}

// Process a word segment according to language rules
std::vector<Language> LanguageDetector::process_sentence(const std::vector<uint32_t> &word, Language &current_context) const {
    // Process based on main language
    if (main_language_ == Language::PERSIAN) {
        return process_sentence_main_is_persian(word, current_context);
    } else if (main_language_ == Language::ARABIC) {
        return process_sentence_main_is_arabic(word, current_context);
    } else if (main_language_ == Language::ENGLISH) {
        return process_sentence_main_is_english(word, current_context);
    }
    return std::vector<Language>(word.size(), Language::UNKNOWN);
}

// Main detection function
std::vector<Language> LanguageDetector::detect(const std::string &utf8_input) const {
    std::vector<uint32_t> codepoints = utf8_to_codepoints(utf8_input);
    std::vector<LanguageDetector::TextSegment> segments = split_into_segments(codepoints);
    std::vector<Language> result;
    Language current_context = main_language_;

    for (const TextSegment &segment : segments) {
        if (segment.is_separator) {
            // Add separator tags
            result.insert(result.end(), segment.characters.size(), Language::SEPARATOR);
            
        } else {
            // Process sentence and get language tags
            std::vector<Language> tags = process_sentence(segment.characters, current_context);
            result.insert(result.end(), tags.begin(), tags.end());
        }
    }
    
    return result;
}

// Process word when main language is Persian
std::vector<Language> LanguageDetector::process_sentence_main_is_persian(const std::vector<uint32_t>& word, Language& current_context) const {
    std::vector<Language> tags;
    bool maybe_arabic = false;
    
    if(current_context == Language::UNKNOWN)
    // || current_context == Language::SEPARATOR) //. is a seperator and should not change language as seen in 3.14
        current_context = Language::PERSIAN;

    //current_context = Language::PERSIAN;

    // Handle Arabic diacritics and ال prefix
    bool st_al = starts_with_al(word); //Needed for single arabic words.
    if (arabic_diacritics_count(word)>= 2 || st_al ) { //There are at least two signs of arabic
        maybe_arabic = true;
    }

    for (uint32_t cp : word) {
    
        if (digits.count(cp) || neutral_letters.count(cp)) {
            //Keep previous language
        }
        else if (latin_letters.count(cp)) {
            current_context = Language::ENGLISH;
        }
        else if (persian_arabic_letters.count(cp)) {
            if(maybe_arabic)
                current_context = Language::ARABIC;
            else
                current_context = Language::PERSIAN;
        }
        tags.push_back(current_context);
    }

    return tags;
}

// Process word when main language is Arabic
std::vector<Language> LanguageDetector::process_sentence_main_is_arabic(const std::vector<uint32_t>& word, Language& current_context) const {
    std::vector<Language> tags;

    if(current_context == Language::UNKNOWN || current_context == Language::SEPARATOR)
        current_context = Language::ARABIC;

    for (uint32_t cp : word) {
        if (digits.count(cp) || neutral_letters.count(cp)) {
            if(current_context == Language::UNKNOWN)
                current_context = Language::ENGLISH;
            else {
                ; //Keep previous language
            }
        }
        else if (latin_letters.count(cp)) {
            current_context = Language::ENGLISH;
        }
        else if (current_context == Language::ENGLISH && persian_arabic_letters.count(cp)) {
            current_context = Language::ARABIC;
        }
        tags.push_back(current_context);
    }

    return tags;
}

// Process word when main language is English
std::vector<Language> LanguageDetector::process_sentence_main_is_english(const std::vector<uint32_t>& word, Language& current_context) const {
    std::vector<Language> tags;

    if(current_context == Language::UNKNOWN || current_context == Language::SEPARATOR)
        current_context = Language::ENGLISH;

    for (uint32_t cp : word) {
        if (digits.count(cp) || neutral_letters.count(cp)) {
            if(current_context == Language::UNKNOWN)
                current_context = Language::ENGLISH;
            else {
                ; //Keep previous language
            }
        }
        else if (latin_letters.count(cp)) {
            current_context = Language::ENGLISH;
        }
        else if (persian_arabic_letters.count(cp)) {
            current_context = Language::UNKNOWN;
        }
        tags.push_back(current_context);
    }

    return tags;
}

std::vector<DetectedSegment> LanguageDetector::detect_segments(const std::string &utf8_input) const {
    std::vector<Language> tags = detect(utf8_input);
    std::vector<uint32_t> codepoints = utf8_to_codepoints(utf8_input);
    std::vector<DetectedSegment> segments;
    
    if (codepoints.empty()) {
        return segments;
    }
    
    DetectedSegment current_segment;
    current_segment.language = tags[0];
    current_segment.text = codepoint_to_utf8(codepoints[0]);
    
    for (size_t i = 1; i < codepoints.size(); ++i) {
        Language current_tag = tags[i];
        uint32_t cp = codepoints[i];
        std::string char_str = codepoint_to_utf8(cp);
        
        // Merge separators with previous segment
        if (current_tag == Language::SEPARATOR) {
            current_segment.text += char_str;
        }
        // Merge with previous segment if same language
        else if (current_tag == current_segment.language) {
            current_segment.text += char_str;
        }
        // Start new segment
        else {
            segments.push_back(current_segment);
            current_segment.text = char_str;
            current_segment.language = current_tag;
        }
    }
    
    // Add the last segment
    if (!current_segment.text.empty()) {
        segments.push_back(current_segment);
    }
    
    return segments;
}

std::string LanguageDetector::reunite_segments(const std::vector<DetectedSegment>& segments) {
    std::string result;
    for (const auto& segment : segments) {
        result += segment.text;
    }

    return result;
}
