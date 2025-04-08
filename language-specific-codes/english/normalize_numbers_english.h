// normalize_numbers_english.h
//Besm ALLAH
//Test by:
// g++ -x c++ -DENGLISH_NUMBER_CONVERTER_ENABLE_TESTS -include normalize_numbers_english.h -o test_program /dev/null && ./test_program

#ifndef ENGLISH_NUMBER_CONVERTER_H
#define ENGLISH_NUMBER_CONVERTER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace EnglishNumberConverter {

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
    
    for(char c : input) {
        // Handle separators and spaces
        if(c == ',' || c == '-' || c == ' ') {
            continue;
        }
        
        // Handle decimal point
        if(c == '.') {
            if(decimal_point_found) {
                result.is_valid = false;
                return result;
            }
            decimal_point_found = true;
            continue;
        }
        
        // Handle digits
        if(isdigit(c)) {
            if(decimal_point_found) {
                result.fractional_part.push_back(c);
            } else {
                result.integer_part.push_back(c);
            }
        } else {
            result.is_valid = false;
            return result;
        }
    }
    
    // Post-processing
    if(result.integer_part.empty() && !result.fractional_part.empty()) {
        result.integer_part = "0";
    }

    // Remove leading zeros
    if(!result.integer_part.empty()) {
        size_t first_non_zero = result.integer_part.find_first_not_of('0');
        if(first_non_zero != std::string::npos) {
            result.integer_part = result.integer_part.substr(first_non_zero);
        } else {
            result.integer_part = "0";
        }
    }

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
inline std::string group_to_words(const std::string& group) {
    const std::string ones[] = {"", "one", "two", "three", "four", "five", 
                               "six", "seven", "eight", "nine"};
    const std::string teens[] = {"ten", "eleven", "twelve", "thirteen", 
                                "fourteen", "fifteen", "sixteen", 
                                "seventeen", "eighteen", "nineteen"};
    const std::string tens[] = {"", "", "twenty", "thirty", "forty", "fifty",
                               "sixty", "seventy", "eighty", "ninety"};
    
    std::string padded = std::string(3 - group.length(), '0') + group;
    int hundred = padded[0] - '0';
    int ten = padded[1] - '0';
    int one = padded[2] - '0';
    
    std::string words;
    if(hundred > 0) {
        words += ones[hundred] + " hundred";
    }
    
    if(ten != 0 || one != 0) {
        if(!words.empty()) words += " ";
        if(ten == 1) {
            words += teens[one];
        } else {
            if(ten > 1) {
                words += tens[ten];
                if(one > 0) words += "-" + ones[one];
            } else if(one > 0) {
                words += ones[one];
            }
        }
    }
    return words;
}

inline std::string digit_to_word(char c) {
    switch(c) {
        case '0': return "zero";
        case '1': return "one";
        case '2': return "two";
        case '3': return "three";
        case '4': return "four";
        case '5': return "five";
        case '6': return "six";
        case '7': return "seven";
        case '8': return "eight";
        case '9': return "nine";
        default: return "";
    }
}

inline SpecialNumberInfo detect_special_type(const std::string& normalized) {
    SpecialNumberInfo info;
    int len = normalized.length();
    
    if(len > 9) {
        info.type = "unclassified";
        info.groups = std::vector<int>(len, 1);
    }
    
    return info;
}

// ==================== Public API ====================
inline std::string number_to_words(const std::string& normalized) {
    if(normalized == "0") return "zero";
    
    std::vector<std::string> groups = split_into_groups(normalized);
    std::reverse(groups.begin(), groups.end());
    
    const std::vector<std::string> scales = {"", "thousand", "million", "billion", "trillion"};
    std::string result;
    
    for(size_t i = 0; i < groups.size(); ++i) {
        std::string words = group_to_words(groups[i]);
        if(!words.empty()) {
            if(!result.empty()) result += " ";
            result += words;
            size_t scale_idx = groups.size() - 1 - i;
            if(scale_idx < scales.size() && !scales[scale_idx].empty())
                result += " " + scales[scale_idx];
        }
    }
    return result;
}

inline std::string process_token(const std::string& token) {
    NormalizedNumber normalized = normalize(token);
    if(!normalized.is_valid) {
        return token;
    }

    SpecialNumberInfo special_info = detect_special_type(normalized.integer_part);
    if(!special_info.groups.empty() || normalized.integer_part.length() > 9) {
        std::vector<std::string> parts;
        for(char c : normalized.integer_part) {
            parts.push_back(digit_to_word(c));
        }
        
        std::string result;
        for(size_t i = 0; i < parts.size(); ++i) {
            if(i > 0) result += ", ";
            result += parts[i];
        }

        if(!normalized.fractional_part.empty()) {
            result += " point ";
            for(char c : normalized.fractional_part) {
                result += digit_to_word(c) + " ";
            }
        }
        return result;
    }

    if(!normalized.fractional_part.empty()) {
        std::string int_part = number_to_words(normalized.integer_part);
        std::string frac_part;
        for(char c : normalized.fractional_part) {
            if(!frac_part.empty()) frac_part += " ";
            frac_part += digit_to_word(c);
        }
        return int_part + " point " + frac_part;
    }

    return number_to_words(normalized.integer_part);
}

inline std::string to_english_text(const std::string& input) {
    std::string result;
    std::string current_token;
    
    for(char c : input) {
        if(isdigit(c) || c == '.' || c == ',') {
            current_token += c;
        } else {
            if(!current_token.empty()) {
                result += process_token(current_token);
                current_token.clear();
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
#ifdef ENGLISH_NUMBER_CONVERTER_ENABLE_TESTS

inline void run_tests() {
    struct TestCase {
        std::string input;
        std::string expected;
        bool should_throw;
    };

    std::vector<TestCase> tests = {
        // Basic numbers
        {"0", "zero", false},
        {"5", "five", false},
        {"12", "twelve", false},
        {"123", "one hundred twenty-three", false},
        {"1200", "one thousand two hundred", false},
        
        // Decimal numbers
        {"12.34", "twelve point three four", false},
        {"0.5", "zero point five", false},
        {".75", "zero point seven five", false},
        
        // Mixed text and numbers
        {"The code is 1234", "The code is one thousand two hundred thirty-four", false},
        {"Call 555-1234", "Call five hundred fifty-five-one thousand two hundred thirty-four", false},
        
        // Edge cases
        {"", "", false}, // Empty input
        {"abc", "abc", false}, // No numbers
        {"ab.c", "ab.c", false}, // No numbers
        {"v8a", "v eight a", false}, // Glued
        {"123..45", "123..45", false}, // Invalid number
        {"123,456,789", "one hundred twenty-three million four hundred fifty-six thousand seven hundred eighty-nine", false} // With separators

    };

    int passed = 0;
    for (const auto& test : tests) {
        try {
            std::string result = to_english_text(test.input);
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

#endif // ENGLISH_NUMBER_CONVERTER_ENABLE_TESTS

} // namespace EnglishNumberConverter

#ifdef ENGLISH_NUMBER_CONVERTER_ENABLE_TESTS
int main() {
    EnglishNumberConverter::run_tests();
    return 0;
}
#endif // ENGLISH_NUMBER_CONVERTER_ENABLE_TESTS

#endif // ENGLISH_NUMBER_CONVERTER_H