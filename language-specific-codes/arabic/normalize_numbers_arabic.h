// normalize_numbers_arabic.h
//Besm ALLAH
//Test by: 
// g++ -x c++ -DARABIC_NUMBER_CONVERTER_ENABLE_TESTS -include normalize_numbers_arabic.h -o test_program /dev/null && ./test_program

#ifndef ARABIC_NUMBER_CONVERTER_H
#define ARABIC_NUMBER_CONVERTER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace ArabicNumberConverter {

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
    
    // First remove all commas from the input
    std::string processed_input;
    for(char c : input) {
        if(c != ',') {
            processed_input += c;
        }
    }
    
    for(char c : processed_input) {
        // Handle decimal point
        if(c == '.') {
            if(decimal_point_found) {
                result.is_valid = false;
                return result;
            }
            decimal_point_found = true;
            continue;
        }
        
        if(c == ' ') {
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

    // Post-processing remains the same
    if(result.integer_part.empty() && !result.fractional_part.empty()) {
        result.integer_part = "0";
    }

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
inline std::string group_to_words(int num) {
    static const std::vector<std::string> units = {
        "", "واحد", "اثنان", "ثلاثة", "أربعة", "خمسة", "ستة", "سبعة", "ثمانية", "تسعة",
        "عشرة", "أحد عشر", "اثنا عشر", "ثلاثة عشر", "أربعة عشر", "خمسة عشر", 
        "ستة عشر", "سبعة عشر", "ثمانية عشر", "تسعة عشر"
    };
    
    static const std::unordered_map<int, std::string> tens = {
        {20, "عشرون"}, {30, "ثلاثون"}, {40, "أربعون"}, {50, "خمسون"},
        {60, "ستون"}, {70, "سبعون"}, {80, "ثمانون"}, {90, "تسعون"}
    };

    static const std::unordered_map<int, std::string> hundreds = {
        {100, "مائة"}, {200, "مئتان"}, {300, "ثلاثمائة"}, {400, "أربعمائة"},
        {500, "خمسمائة"}, {600, "ستمائة"}, {700, "سبعمائة"}, {800, "ثمانمائة"}, {900, "تسعمائة"}
    };

    std::string result;
    int hundred = (num / 100) * 100;
    if (hundred > 0) {
        result += hundreds.at(hundred);
        num %= 100;
        if (num > 0) result += " و ";
    }

    if (num >= 20) {
        if (num % 10 != 0) {
            result += units[num % 10] + " و " + tens.at((num / 10) * 10);
        } else {
            result += tens.at((num / 10) * 10);
        }
    } else if (num > 0) {
        result += units[num];
    }

    return result;
}

inline std::string digit_to_word(char c) {
    switch(c) {
        case '0': return "صفر";
        case '1': return "واحد";
        case '2': return "اثنان";
        case '3': return "ثلاثة";
        case '4': return "أربعة";
        case '5': return "خمسة";
        case '6': return "ستة";
        case '7': return "سبعة";
        case '8': return "ثمانية";
        case '9': return "تسعة";
        default: return "";
    }
}

inline SpecialNumberInfo detect_special_type(const std::string& normalized) {
    SpecialNumberInfo info;
    int len = normalized.length();
    
    if(len > 9) {
        info.type = "نامعین";
        info.groups = std::vector<int>(len, 1);
    }
    
    return info;
}

// ==================== Public API ====================
inline std::string number_to_words(const std::string& normalized) {
    if(normalized == "0") return "صفر";
    
    std::vector<std::string> groups = split_into_groups(normalized);
    std::reverse(groups.begin(), groups.end());
    
    const std::vector<std::string> scales = {"", "ألف", "مليون", "مليار", "تريليون"};
    std::vector<std::string> group_words;
    
    for(size_t i = 0; i < groups.size(); ++i) {
        int group_num = std::stoi(groups[i]);
        if(group_num == 0) continue;
        
        std::string words = group_to_words(group_num);
        size_t scale_idx = groups.size() - 1 - i; // Correct scale index
        
        if(scale_idx > 0) {
            // Handle scale words
            if(group_num == 1) {
                words = scales[scale_idx]; // e.g., "ألف" instead of "واحد ألف"
            } 
            else if(group_num == 2) {
                words = scales[scale_idx] + "ان"; // Dual form
            }
            else if(group_num >= 3 && group_num <= 10) {
                if(scale_idx == 1) { // Thousands plural
                    words += " آلاف";
                } else {
                    words += " " + scales[scale_idx] + "ات"; // Millions, etc.
                }
            }
            else {
                words += " " + scales[scale_idx]; // General case
            }
        }
        
        group_words.push_back(words);
    }
    
    // Join groups in natural order
    std::string result;
    for(size_t i = 0; i < group_words.size(); ++i) {
        if(i > 0) result += " و ";
        result += group_words[i];
    }
    
    return result;
}

inline std::string process_token(const std::string& token) {
    NormalizedNumber normalized = normalize(token);
    if(!normalized.is_valid) {
        return token;
    }

    SpecialNumberInfo special_info = detect_special_type(normalized.integer_part);
    if(!special_info.groups.empty()) {
        std::vector<std::string> parts;
        for(char c : normalized.integer_part) {
            parts.push_back(digit_to_word(c));
        }
        
        std::string result;
        for(size_t i = 0; i < parts.size(); ++i) {
            if(i > 0) result += "، ";
            result += parts[i];
        }

        if(!normalized.fractional_part.empty()) {
            result += " فاصلة ";
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
        return int_part + " فاصلة " + frac_part;
    }

    return number_to_words(normalized.integer_part);
}

inline std::string to_arabic_text(const std::string& input) {
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
#ifdef ARABIC_NUMBER_CONVERTER_ENABLE_TESTS

inline void run_tests() {
    struct TestCase {
        std::string input;
        std::string expected;
        bool should_throw;
    };

    std::vector<TestCase> tests = {
        // Basic numbers
        {"0", "صفر", false},
        {"5", "خمسة", false},
        {"12", "اثنا عشر", false},
        {"123", "مائة و ثلاثة و عشرون", false},
        {"449", "أربعمائة و تسعة و أربعون", false},
        {"864", "ثمانمائة و أربعة و ستون", false},
          
        {"1200", "ألف و مئتان", false},
        {"2025", "ألفان و خمسة و عشرون", false},
        {"3555", "ثلاثة آلاف و خمسمائة و خمسة و خمسون", false},
        
        // Decimal numbers
        {"12.34", "اثنا عشر فاصلة ثلاثة أربعة", false},
        {"0.5", "صفر فاصلة خمسة", false},
        {".75", "صفر فاصلة سبعة خمسة", false},
        
        // Mixed text and numbers
        {"The code is 1234", "The code is ألف و مئتان و أربعة و ثلاثون", false},
        {"Call 555-1234", "Call خمسمائة و خمسة و خمسون-ألف و مئتان و أربعة و ثلاثون", false},
        
        // Edge cases
        {"", "", false}, // Empty input
        {"abc", "abc", false}, // No numbers
        {"ab.c", "ab.c", false}, // No numbers
        {"ا۸ب", "ا ثمانیه ب", false}, // Glued
        {"123..45", "123..45", false}, // Invalid number
        {"123,456,789", "مائة و ثلاثة و عشرون مليون و أربعمائة و ستة و خمسون ألف و سبعمائة و تسعة و ثمانون", false} // With separators
    };

    int passed = 0;
    for (const auto& test : tests) {
        try {
            std::string result = to_arabic_text(test.input);
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

#endif // ARABIC_NUMBER_CONVERTER_ENABLE_TESTS

} // namespace ArabicNumberConverter

#ifdef ARABIC_NUMBER_CONVERTER_ENABLE_TESTS
int main() {
    ArabicNumberConverter::run_tests();
    return 0;
}
#endif // ARABIC_NUMBER_CONVERTER_ENABLE_TESTS

#endif // ARABIC_NUMBER_CONVERTER_H