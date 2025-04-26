#ifndef REPLACEMENT_HELPERS_H
#define REPLACEMENT_HELPERS_H

#include <vector>
#include <string>
#include <unordered_map>

std::vector<std::string> split(const std::string& s, char delimiter);
std::string join(const std::vector<std::string>& vec, char delimiter);
std::string toLower(const std::string& str);
std::string factorizeChineseLetters(const std::string& input);
bool is_cjk(uint32_t cp);
std::string replace_cjk_with_placeholder(const std::string& input);
void applyWholeWordReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements);
void applyWholeWordReplacementsArabic(std::string& result, const std::unordered_map<std::string, std::string>& replacements);
void applyNormalReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements);
void applyNormalReplacementsWithSpace(std::string& result, const std::unordered_map<std::string, std::string>& replacements);
void doArabicSpecificReplacements(std::string &segment_text);
void applyBitByteReplacements(std::string &result);
void seperateCapitalAbbreviations(std::string &result);
void seperateClock(std::string &result);
void seperateDate(std::string &result);
void seperateCamelCaseWords(std::string &result);


#endif