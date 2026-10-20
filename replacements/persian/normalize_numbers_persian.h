// normalize_numbers_persian.h
//Besm ALLAH
//Test by: 
// g++ -x c++ -DPERSIAN_NUMBER_CONVERTER_ENABLE_TESTS -include normalize_numbers_persian.h -o test_program /dev/null && ./test_program

#ifndef PERSIAN_NUMBER_CONVERTER_H
#define PERSIAN_NUMBER_CONVERTER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace PersianNumberConverter {

// ==================== Core Data Structures ====================
struct NormalizedNumber {
    std::string integer_part;
    std::string fractional_part;
    bool is_valid;
};

struct SpecialNumberInfo {
    std::string type;
    std::vector<int> groups;
};

// ==================== Core Functions ====================
inline NormalizedNumber normalize(const std::string& input) {
    NormalizedNumber result;
    result.is_valid = true;
    bool decimal_point_found = false;
    
    for(size_t i = 0; i < input.size(); ) {
        char c = input[i];
        
        // Handle thousand separators (comma)
        if(c == ',') {
            // Only allow commas every 3 digits before decimal
            if(decimal_point_found || 
               (result.integer_part.length() % 3 != 0 && !result.integer_part.empty())) {
                result.is_valid = false;
                return result;
            }
            i++;
            continue;
        }
        
        if(c == '.') {
            if(decimal_point_found) {
                result.is_valid = false;
                return result;
            }
            decimal_point_found = true;
            i++;
            continue;
        }
        
        if(isdigit(c)) {
            if(decimal_point_found) {
                result.fractional_part.push_back(c);
            } else {
                result.integer_part.push_back(c);
            }
            i++;
        } else {
            result.is_valid = false;
            return result;
        }
    }

    // Post-processing
    if(result.integer_part.empty() && !result.fractional_part.empty()) {
        result.integer_part = "0";
    }

    // Don't remove leading zeros - we need them for special numbers like phone numbers
    // if(!result.integer_part.empty()) {
    //     size_t first_non_zero = result.integer_part.find_first_not_of('0');
    //     if(first_non_zero != std::string::npos) {
    //         result.integer_part = result.integer_part.substr(first_non_zero);
    //     } else {
    //         result.integer_part = "0";
    //     }
    // }

    return result;
}

inline std::vector<std::string> split_into_groups(const std::string& s, int group_size = 3) {
    std::vector<std::string> groups;
    int len = s.length();
    for(int i = len; i > 0; i -= group_size) {
        int start = std::max(i - group_size, 0);
        groups.push_back(s.substr(start, i - start));
    }
    return groups;
}

// ==================== Conversion Logic ====================
inline std::string group_to_words(int num) {
    static const std::vector<std::string> units = {
        "", "یک", "دو", "سه", "چهار", "پنج", "شش", "هفت", "هشت", "نُه",
        "دَه", "یازده", "دوازده", "سیزده", "چهارده", "پانزده", "شانزده",
        "هفده", "هجده", "نوزده"
    };
    
    static const std::unordered_map<int, std::string> tens = {
        {20, "بیست"}, {30, "سی"}, {40, "چهل"}, {50, "پنجاه"},
        {60, "شصت"}, {70, "هفتاد"}, {80, "هشتاد"}, {90, "نود"}
    };

    static const std::unordered_map<int, std::string> hundreds = {
        {100, "صد"}, {200, "دویست"}, {300, "سیصد"}, {400, "چهارصد"},
        {500, "پانصد"}, {600, "ششصد"}, {700, "هفتصد"}, {800, "هشتصد"}, {900, "نهصد"}
    };

    std::string result;
    int hundred = (num / 100) * 100;
    if (hundred > 0) {
        result += hundreds.at(hundred);
        num %= 100;
        if (num > 0) result += "ُ ";
    }

    if (num >= 20) {
        result += tens.at((num / 10) * 10);
        num %= 10;
        if (num > 0) result += "ُ " + units[num];
    } else if (num > 0) {
        result += units[num];
    }

    return result;
}

inline std::string digit_to_word(char c) {
    switch(c) {
        case '0': return "صفر";
        case '1': return "یک";
        case '2': return "دو";
        case '3': return "سه";
        case '4': return "چهار";
        case '5': return "پنج";
        case '6': return "شش";
        case '7': return "هفت";
        case '8': return "هشت";
        case '9': return "نُه";
        default: return "";
    }
}

inline SpecialNumberInfo detect_special_type(const std::string& normalized) {
    SpecialNumberInfo info;
    int len = normalized.length();

    // Phone numbers (11 digits starting with 0)
    if (len == 11 && normalized[0] == '0') {
        info.type = "شماره تلفن";
        info.groups = std::vector<int>(len, 1); // Process all digits individually
        return info;
    }

    // Credit card numbers (16 digits)
    if (len == 16) {
        info.type = "شماره کارت";
        info.groups = {4, 4, 4, 4}; // Group as XXXX-XXXX-XXXX-XXXX
        return info;
    }

    // National ID (10 digits)
    if (len == 10) {
        info.type = "کد ملی";
        info.groups = {3, 3, 3, 1}; // Standard Iranian ID format
        return info;
    }

    // Other long numbers
    if (len > 9) {
        info.type = "نامعین";
        info.groups = std::vector<int>(len, 1); // Process all digits individually
    }

    return info;
}

// ==================== Public API ====================
inline std::string number_to_words(const std::string& normalized) {
    if (normalized == "0") return "صفر";
    
    std::vector<std::string> groups = split_into_groups(normalized);
    std::reverse(groups.begin(), groups.end()); // Reverse groups to process highest first
    
    // simple-app branch: stop at میلیارد (تریلیون never occurs in speech here)
    const std::vector<std::string> scales = {"", "هِزار", "میلیون", "میلیارد"};
    std::vector<std::string> group_words;
    
    for (size_t i = 0; i < groups.size(); ++i) {
        int num = std::stoi(groups[i]);
        if (num == 0) continue;
        
        std::string words = group_to_words(num);
        size_t scale_idx = groups.size() - 1 - i;
        
        if (scale_idx < scales.size() && !scales[scale_idx].empty()) {
            if (scale_idx == 1) {  // هِزار
                if (num == 1) {
                    words = "هِزار";
                } else if (num == 2) {
                    words = "دو هِزار";
                } else {
                    words += " " + scales[scale_idx];
                }
            } else {
                words += " " + scales[scale_idx];
            }
        }
        
        group_words.push_back(words);
    }
    
    // Combine groups in order
    std::string result;
    for (const auto& group : group_words) {
        if (!result.empty()) {
            result += "ُ ";
        }
        result += group;
    }
    
    return result;
}

// ---------------------------------------------------------------------------------------------
// Ali's number policy for the simple app (2026-10-13)
// ---------------------------------------------------------------------------------------------

/** True when the character is a digit or a separator that may sit INSIDE a number. */
inline bool isDigitLikeOrSep(char c) {
    return (c >= '0' && c <= '9') || c == ' ' || c == '\t' || c == '-' || c == '\xE2';  // 0xE2 = start of ZWNJ/٬ in UTF-8
}

/** Two digits read as one two-digit number («۶۲» -> «شصت و دو»); a leading zero is spoken. */
inline std::string pair_to_words(const std::string& two) {
    if (two.size() != 2) return number_to_words(two);
    int v = std::stoi(two);
    if (two[0] == '0') {
        // «۰۶» -> «صفر شش»: the listener must hear that the pair starts with a zero
        return std::string("صفر ") + digit_to_word(two[1]);
    }
    return number_to_words(two);
}

/** The 11-digit mobile rule: صفر + 3 + 3 + 2 + 2 (his example 09125502218). */
inline std::string mobile_to_words(const std::string& digits) {
    if (digits.size() != 11) return std::string();
    std::string out = "صفر";
    const int lens[4] = {3, 3, 2, 2};
    size_t pos = 1;
    for (int k = 0; k < 4; ++k) {
        std::string part = digits.substr(pos, lens[k]);
        pos += lens[k];
        out += "، " + number_to_words(part);
    }
    return out;
}

/** Digit by digit, every digit, with a short pause between them. */
inline std::string digits_to_words(const std::string& digits) {
    std::string out;
    for (char c : digits) {
        if (!out.empty()) out += "، ";
        out += digit_to_word(c);
    }
    return out;
}

inline std::string process_token(const std::string& token) {
    NormalizedNumber normalized = normalize(token);
    if (!normalized.is_valid) {
        return token;
    }

    // --- Ali's policy (simple-app) -----------------------------------------------------------
    const std::string& digits = normalized.integer_part;
    if (normalized.fractional_part.empty()) {
        if (digits.size() == 11 && digits[0] == '0' && digits[1] == '9') {
            return mobile_to_words(digits);                       // 09xxxxxxxxx
        }
        if (digits.size() == 16) {
            // card number: two digits at a time (the 4-group form is joined before this point)
            std::string out;
            for (size_t i = 0; i + 1 < digits.size(); i += 2) {
                if (!out.empty()) out += "، ";
                out += pair_to_words(digits.substr(i, 2));
            }
            return out;
        }
        if (digits.size() >= 13) {
            return digits_to_words(digits);                        // long run: every digit
        }
    }
    // -----------------------------------------------------------------------------------------

    // Handle special numbers (phone numbers, IDs, etc.)
    SpecialNumberInfo special_info = detect_special_type(normalized.integer_part);
    if (!special_info.groups.empty()) {
        std::vector<std::string> parts;
        for (char c : normalized.integer_part) {
            parts.push_back(digit_to_word(c));
        }

        std::string result;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) result += "، ";
            result += parts[i];
        }

        if (!normalized.fractional_part.empty()) {
            result += " ممیز ";
            for (char c : normalized.fractional_part) {
                result += digit_to_word(c) + " ";
            }
        }
        return result;
    }

    // Handle regular numbers with fractions
    if (!normalized.fractional_part.empty()) {
        std::string int_part = number_to_words(normalized.integer_part);
        std::string frac_part;
        for (char c : normalized.fractional_part) {
            if (!frac_part.empty()) frac_part += " ";
            frac_part += digit_to_word(c);
        }
        return int_part + " ممیز " + frac_part;
    }

    // Handle regular integer numbers
    return number_to_words(normalized.integer_part);
}


/** simple-app: join «6219 8618 2654 3298» (also ZWNJ / dash separated) into one token so the
 *  16-digit card rule in process_token sees it as a single number. */
inline std::string join_card_groups(const std::string& in) {
    static const std::string zwnj = "\xE2\x80\x8C";
    std::string out;
    size_t i = 0;
    while (i < in.size()) {
        // try to match 4 digits, separator, 4 digits, separator, 4 digits, separator, 4 digits
        size_t lens[4] = {0, 0, 0, 0};
        size_t pos[4] = {i, 0, 0, 0};
        size_t j = i;
        bool ok = true;
        for (int g = 0; g < 4 && ok; ++g) {
            pos[g] = j;
            size_t d = 0;
            while (d < 4 && j < in.size() && isdigit((unsigned char)in[j])) { ++j; ++d; }
            lens[g] = d;
            if (d != 4) { ok = false; break; }
            if (g == 3) break;                                     // no separator after the last group
            size_t k = j;
            if (k < in.size() && (unsigned char)in[k] == 0x20) { ++k; }            // plain space
            else if (in.compare(k, zwnj.size(), zwnj) == 0) { k += zwnj.size(); }   // ZWNJ
            else if (k < in.size() && in[k] == '-') { ++k; }                        // dash
            else { ok = false; break; }
            j = k;
        }
        if (ok) {
            for (int g = 0; g < 4; ++g) out += in.substr(pos[g], 4);
            i = j;
        } else {
            out += in[i];
            ++i;
        }
    }
    return out;
}

inline std::string normalize_text(const std::string& input_raw) {
    const std::string input = join_card_groups(input_raw);   // simple-app: card groups -> one token
    std::string result;
    std::string current_token;
    
    for(size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        
        if(isdigit(c)) {
            // Add space before number if previous character isn't already a space or at start
            if(current_token.empty() && !result.empty() && !isspace(result.back())) {
                result += ' ';
            }
            current_token += c;
        } 
        else if(c == ',' || ((unsigned char)c == 0xD9 && i + 1 < input.size() &&
                             ((unsigned char)input[i+1] == 0xAC || (unsigned char)input[i+1] == 0xAB))) {
            // simple-app: a group separator INSIDE a number is dropped (never read as «ویرگول»).
            // 0xD9 0xAC = ٬ (U+066C group separator), 0xD9 0xAB = ٫ (U+066B decimal separator).
            bool is_persian_sep = ((unsigned char)c == 0xD9);
            bool prev_is_digit = (i > 0 && isdigit(input[i-1]));
            int following = 0;
            size_t start = i + (is_persian_sep ? 2 : 1);
            for (size_t j = start; j < input.size() && following < 4; ++j) {
                if (isdigit(input[j])) ++following; else break;
            }
            if (prev_is_digit && following == 3) { i = start - 1; continue; }   // ۱۲٬۳۴۵ -> skip
            if (is_persian_sep) {                                                // stray: keep the letter-safe bytes
                result += input.substr(i, 2); i += 1; continue;
            }
            // plain comma: legacy handling
            if(current_token.empty() && !result.empty() && !isspace(result.back())) result += ' ';
            current_token += c;
        }
        else if(c == '.') {
            // Check if the previous or next character is a digit
            bool valid_decimal = false;
            
            // Check previous character
            if(i > 0 && isdigit(input[i-1])) {
                valid_decimal = true;
            }
            // Check next character
            else if(i < input.size()-1 && isdigit(input[i+1])) {
                valid_decimal = true;
            }
            
            if(valid_decimal) {
                current_token += c;
            } else {
                // Not a valid decimal point or separator
                if(!current_token.empty()) {
                    result += process_token(current_token);
                    current_token.clear();
                    // Add space after number
                    if(!isspace(c)) {
                        result += ' ';
                    }
                }
                result += c;
            }
        }
        else {
            if(!current_token.empty()) {
                result += process_token(current_token);
                current_token.clear();
                // Add space after number
                if(!isspace(c)) {
                    result += ' ';
                }
            }
            result += c;
        }
    }
    
    if(!current_token.empty()) {
        result += process_token(current_token);
    }
    
    return result;
}

// ==================== Test Suite ====================
#ifdef PERSIAN_NUMBER_CONVERTER_ENABLE_TESTS

inline void run_tests() {
    struct TestCase {
        std::string input;
        std::string expected;
        bool should_throw;
    };

    std::vector<TestCase> tests = {
        // Basic numbers
        {"0", "صفر", false},
        {"5", "پنج", false},
        {"12", "دوازده", false},
        {"2", "دو", false},
        {"123", "صدُ بیستُ سه", false},
        {"1200", "هِزارُ دویست", false},
        {"9", "نُه", false},
        {"9000", "نُه هِزار", false},
        
        // Decimal numbers
        {"12.34", "دوازده ممیز سه چهار", false},
        {"0.5", "صفر ممیز پنج", false},
        {".75", "صفر ممیز هفت پنج", false},
        
        // Mixed text and numbers
        {"کد 1234 است", "کد هِزارُ دویستُ سیُ چهار است", false},
        {"شماره 09123456789", "شماره صفر، نهصدُ دوازده، سیصدُ چهلُ پنج، شصتُ هفت، هشتادُ نُه", false}, // simple-app: mobile 0 + 3 + 3 + 2 + 2
        
        // Edge cases
        {"", "", false}, // Empty input
        {"متن", "متن", false}, // No numbers
        {"abc", "abc", false}, // No numbers
        {"ab.c", "ab.c", false}, // No numbers
        {"ا8ب", "ا هشت ب", false}, // Glued
        {"123..45", "123..45", false}, // Invalid number
        {"123,456,789", "صدُ بیستُ سه میلیونُ چهارصدُ پنجاهُ شش هِزارُ هفتصدُ هشتادُ نُه", false} // With separators
    };

    int passed = 0;
    for (const auto& test : tests) {
        try {
            std::string result = normalize_text(test.input);
            if (test.should_throw) {
                std::cout << "FAIL: '" << test.input << "' should throw but returned '" 
                          << result << "'\n";
                continue;
            }
            if (result == test.expected) {
                passed++;
                std::cout << "PASS: '" << test.input << "' -> '" << result << "'\n";
            } else {
                std::cout << "FAIL: '" << test.input << "'\n"
                          << "  Expected: '" << test.expected << "'\n"
                          << "  Got:      '" << result << "'\n";
            }
        } catch (...) {
            if (!test.should_throw) {
                std::cout << "FAIL: '" << test.input << "' threw unexpected exception\n";
            } else {
                passed++;
                std::cout << "PASS: '" << test.input << "' correctly threw exception\n";
            }
        }
    }

    std::cout << "\nTests completed. Passed " << passed << " out of " 
              << tests.size() << " tests.\n";
}

#endif // PERSIAN_NUMBER_CONVERTER_ENABLE_TESTS

} // namespace PersianNumberConverter

#ifdef PERSIAN_NUMBER_CONVERTER_ENABLE_TESTS
int main() {
    PersianNumberConverter::run_tests();
    return 0;
}
#endif // PERSIAN_NUMBER_CONVERTER_ENABLE_TESTS

#endif // PERSIAN_NUMBER_CONVERTER_H