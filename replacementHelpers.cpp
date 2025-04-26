#include "replacementHelpers.h"

#include <regex>
#include <codecvt>
#include <locale>

#include <unicode/regex.h>
#include <unicode/unistr.h>
#include <unicode/regex.h>  // Correct header for ICU regex
#include <unicode/ustream.h>

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

void applyWholeWordReplacementsArabic(std::string& result, const std::unordered_map<std::string, std::string>& replacements) {
    if (replacements.empty()) return;

    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(result);

    // Build combined pattern and replacement strings
    icu::UnicodeString patternStr;
    std::vector<icu::UnicodeString> replacementStrs;
    bool first = true;

    for (const auto& replacement : replacements) {
        const icu::UnicodeString target = icu::UnicodeString::fromUTF8(replacement.first);
        replacementStrs.push_back(icu::UnicodeString::fromUTF8(replacement.second));

        if (!first) patternStr.append("|");
        patternStr.append("\\b(");
        patternStr.append(target);
        patternStr.append(")\\b");
        first = false;
    }

    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher matcher(patternStr, 0, status);
    if (U_FAILURE(status)) return;

    matcher.reset(ustr);
    icu::UnicodeString resultStr;
    int32_t lastEnd = 0;

    while (matcher.find(status) && U_SUCCESS(status)) {
        // Append text between matches
        resultStr.append(ustr, lastEnd, matcher.start(status) - lastEnd);
        
        // Find which group matched and use corresponding replacement
        for (int32_t i = 1; i <= matcher.groupCount(); ++i) {
            if (matcher.start(i, status) >= 0) {
                resultStr.append(replacementStrs[i-1]);
                break;
            }
        }
        
        lastEnd = matcher.end(status);
    }

    // Append remaining text
    if (U_SUCCESS(status)) {
        resultStr.append(ustr, lastEnd, ustr.length() - lastEnd);
        
        // Convert back to std::string
        std::string newResult;
        resultStr.toUTF8String(newResult);
        result = newResult;
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

void applyNormalReplacementsWithSpace(std::string& result, const std::unordered_map<std::string, std::string>& replacements) {
    for (const auto& pair : replacements) {
        size_t pos = 0;
        while ((pos = result.find(pair.first, pos)) != std::string::npos) {
            // Always add space before and after
            std::string replacement = " " + pair.second + " ";
            
            // Perform the replacement
            result.replace(pos, pair.first.length(), replacement);
            
            // Move position forward (skip past the replacement + spaces)
            pos += replacement.length();
        }
    }
}

bool is_cjk(uint32_t cp) {
    return (cp >= 0x4E00 && cp <= 0x9FFF);
}

std::string replace_cjk_with_placeholder(const std::string& input) {
    std::string output;
    size_t i = 0;
    while (i < input.size()) {
        unsigned char c = input[i];
        uint32_t codepoint = 0;
        int bytes = 0;

        // Decode UTF-8 to codepoint
        if (c < 0x80) {
            codepoint = c;
            bytes = 1;
        } else if ((c & 0xE0) == 0xC0) {
            codepoint = ((c & 0x1F) << 6) | (input[i + 1] & 0x3F);
            bytes = 2;
        } else if ((c & 0xF0) == 0xE0) {
            codepoint = ((c & 0x0F) << 12) |
                        ((input[i + 1] & 0x3F) << 6) |
                        (input[i + 2] & 0x3F);
            bytes = 3;
        } else if ((c & 0xF8) == 0xF0) {
            codepoint = ((c & 0x07) << 18) |
                        ((input[i + 1] & 0x3F) << 12) |
                        ((input[i + 2] & 0x3F) << 6) |
                        (input[i + 3] & 0x3F);
            bytes = 4;
        }

        if (is_cjk(codepoint)) {
            output += " chinese letter ";
        } else {
            output += input.substr(i, bytes);
        }

        i += bytes;
    }

    return output;
}

void doArabicSpecificReplacements(std::string &segment_text)
{
    // Matches: number → whitespace → "ص" → followed by non-Arabic OR end-of-string
    std::regex pattern1(R"((\d+\s)ص([^\wء-ي]|$))");  // [^\wء-ي] = not a word char or Arabic letter
    std::string replacement1 = "$1صباحاً ";
    segment_text = std::regex_replace(segment_text, pattern1, replacement1);

    // Matches: number → whitespace → "م" → followed by non-Arabic OR end-of-string
    std::regex pattern2(R"((\d+\s)م([^\wء-ي]|$))");  // [^\wء-ي] = not a word char or Arabic letter
    std::string replacement2 = "$1مساءً ";
    segment_text = std::regex_replace(segment_text, pattern2, replacement2);
}

void applyBitByteReplacements(std::string &result) {
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
}

void seperateCapitalAbbreviations(std::string &result) {
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
}

void seperateClock(std::string &result)
{
    bool changed;
    do {
        changed = false;
        std::regex slashBetweenNumbers("([0-9]+)/([0-9]+)");
        std::string new_result = std::regex_replace(result, slashBetweenNumbers, "$1 $2");
        if (new_result != result) {
            changed = true;
            result = new_result;
        }
    } while (changed);
}

void seperateDate(std::string &result) {

    bool changed;
    do {
        changed = false;
        std::regex colonBetweenNumbers("([0-9]+):([0-9]+)");
        std::string new_result = std::regex_replace(result, colonBetweenNumbers, "$1 $2");
        if (new_result != result) {
            changed = true;
            result = new_result;
        }
    } while (changed);
}

void seperateCamelCaseWords(std::string &result) {
    result = std::regex_replace(result, std::regex("([a-z])([A-Z])"), "$1 $2");
    result = std::regex_replace(result, std::regex("([a-z])([A-Z]{2,})(\\b|$)"), "$1 $2$3");
}