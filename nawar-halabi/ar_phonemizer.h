#ifndef AR_PHONEMIZER_H
#define AR_PHONEMIZER_H

#include <string>

//arabic text to nawar
std::string phonemize(const std::string& text);

//nawar to ipa
std::string convertNawarToIPA(const std::string& phonemesStr);

//phonemize + convertNawarToIPA
std::string convertArabicToIPA(const std::string& inputStr); 

#endif