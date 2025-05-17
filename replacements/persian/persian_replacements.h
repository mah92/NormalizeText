#ifndef PERSIAN_REPLACEMENTS_H
#define PERSIAN_REPLACEMENTS_H

// Whole word replacements for Persian
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_PERSIAN = {
    //{"ص", "صفحه"},
    //{"ش", "شماره"},
    //{"ج", "جلد"},
    //{"ک", "کوچه"},
    //{"خ", "خیابان"},
};

// Normal replacements for Persian
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_NO_SPACE_PERSIAN = {
    {"ۀ", "ه ی"},
    {"ة", "ت"},
    {"ك", "ک"},
};

static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_PERSIAN = {
    {"@", "اَت ساین"},
    {"#", "هَش تگ"},
    {"%", "درصد"},
    {"^", "به توان"},
    {"&", "اَند"},
    {"*", "ضرب در"},
    {"+", "به علاوه ی"},
    {"/", "اِسلَش"},
    {"\\", "بَک اِسلَش"},
    {"=", "مساوی"},  
};

#endif