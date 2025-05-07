#ifndef ARABIC_REPLACEMENTS_H
#define ARABIC_REPLACEMENTS_H

// Whole word replacements for Arabic
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_ARABIC = {
    // Arabic Letters
    {"ء", "همزة"},
    {"أ", "ألف"},
    {"ا", "ألف"},
    {"آ", "ألف مدة"},
    {"ب", "باء"},
    {"ت", "تاء"},
    {"ث", "ثاء"},
    {"ج", "جيم"},
    {"ح", "حاء"},
    {"خ", "خاء"},
    {"د", "دال"},
    {"ذ", "ذال"},
    {"ر", "راء"},
    {"ز", "زاي"},
    {"س", "سين"},
    {"ش", "شين"},
    {"ص", "صاد"},
    {"ض", "ضاد"},
    {"ط", "طاء"},
    {"ظ", "ظاء"},
    {"ع", "عين"},
    {"غ", "غين"},
    {"ف", "فاء"},
    {"ق", "قاف"},
    {"ك", "كاف"},  // Standard Arabic kaf
    {"ل", "لام"},
    {"م", "ميم"},
    {"ن", "نون"},
    //{"و", "واو"},
    {"ه", "هاء"},
    {"ي", "ياء"},
    {"ى", "ألف مقصورة"}, //Letter is really different from the above
    {"ة", "تاء مربوطة"},

    // Persian Additional Letters
    {"پ", "په"},      // Pe
    {"چ", "چه"},      // Che
    {"ژ", "ژه"},      // Zhe
    {"گ", "گاف"},     // Gaf
    {"ک", "کاف"},     // Persian Kaf (different from Arabic ك)

    // Arabic Variants
    {"إ", "ألف"},
    {"ئ", "ياء"},
    {"ؤ", "واو"},
};

// Normal replacements for Arabic
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_ARABIC = {
    {"@", "في"},
    {"#", "شباك"},
    {"%", "بالمئة"},
    {"^", "مرفوع للقوة"},
    {"&", "و"},
    {"*", "مضروب في"},
    {"+", "زائد"},
    {"/", "مقسوم على"},
    {"\\", "بَک اِسلَش"},
    {"=", "يساوي"},
};

#endif