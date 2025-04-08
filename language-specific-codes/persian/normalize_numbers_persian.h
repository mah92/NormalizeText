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
inline std::string group_to_words(int num) {
    static const std::vector<std::string> units = {
        "", "یک", "دو", "سه", "چهار", "پنج", "شش", "هفت", "هشت", "نه",
        "ده", "یازده", "دوازده", "سیزده", "چهارده", "پانزده", "شانزده",
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
        if (num > 0) result += " و ";
    }

    if (num >= 20) {
        result += tens.at((num / 10) * 10);
        num %= 10;
        if (num > 0) result += " و " + units[num];
    } else if (num > 0) {
        result += units[num];
    }

    return result;
}

inline std::string digit_to_word(char c) {
    static const std::vector<std::string> digits = {
        "صفر", "یک", "دو", "سه", "چهار", "پنج", "شش", "هفت", "هشت", "نه"
    };
    if (c >= '0' && c <= '9') {
        return digits[c - '0'];
    }
    return "";
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
    
    const std::vector<std::string> scales = {"", "هزار", "میلیون", "میلیارد", "تریلیون"};
    std::vector<std::string> group_words;
    
    for (size_t i = 0; i < groups.size(); ++i) {
        int num = std::stoi(groups[i]);
        if (num == 0) continue;
        
        std::string words = group_to_words(num);
        size_t scale_idx = groups.size() - 1 - i;
        
        if (scale_idx < scales.size() && !scales[scale_idx].empty()) {
            if (scale_idx == 1) {  // هزار
                if (num == 1) {
                    words = "هزار";
                } else if (num == 2) {
                    words = "دو هزار";
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
            result += " و ";
        }
        result += group;
    }
    
    return result;
}

inline std::string process_token(const std::string& token) {
    // First check if this is a phone number pattern (11 digits starting with 0)
    if (token.length() >= 11 && token[0] == '0') {
        bool all_digits = true;
        for (char c : token) {
            if (!isdigit(c)) {
                all_digits = false;
                break;
            }
        }
        if (all_digits) {
            std::vector<std::string> parts;
            for (char c : token) {
                parts.push_back(digit_to_word(c));
            }
            std::string result;
            for (size_t i = 0; i < parts.size(); ++i) {
                if (i > 0) result += "، ";
                result += parts[i];
            }
            return result;
        }
    }

    NormalizedNumber normalized = normalize(token);
    if (!normalized.is_valid) {
        return token;
    }

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

inline std::string to_persian_text(const std::string& input) {
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
        {"123", "صد و بیست و سه", false},
        {"1200", "هزار و دویست", false},
        
        // Decimal numbers
        {"12.34", "دوازده ممیز سه چهار", false},
        {"0.5", "صفر ممیز پنج", false},
        {".75", "صفر ممیز هفت پنج", false},
        
        // Mixed text and numbers
        {"کد 1234 است", "کد هزار و دویست و سی و چهار است", false},
        {"شماره 09123456789", "شماره صفر، نه، یک، دو، سه، چهار، پنج، شش، هفت، هشت، نه", false},
        
        // Edge cases
        {"", "", false}, // Empty input
        {"متن", "متن", false}, // No numbers
        {"abc", "abc", false}, // No numbers
        {"ab.c", "ab.c", false}, // No numbers
        {"ا۸ب", "ا هشت ب", false}, // Glued
        {"123..45", "123..45", false}, // Invalid number
        {"123,456,789", "صد و بیست و سه میلیون و چهارصد و پنجاه و شش هزار و هفتصد و هشتاد و نه", false} // With separators
    };

    int passed = 0;
    for (const auto& test : tests) {
        try {
            std::string result = to_persian_text(test.input);
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