#include <catch2/catch_test_macros.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include "hazm/lemmatizer.hpp"

// This test cross-validates C++ lemmatizer against Python hazm.
// A Python script generates word→lemma mappings first.

TEST_CASE("Cross-validate lemmatizer against Python hazm", "[cross-validate]") {
    hazm::Lemmatizer lemmatizer;
    
    // Test words from Python test suite
    struct TestCase {
        std::string word;
        std::string pos;
        std::string expected;
    };
    
    std::vector<TestCase> test_cases = {
        // Python test_lemmatize cases
        {"\u06A9\u062A\u0627\u0628\u200C\u0647\u0627", "", "\u06A9\u062A\u0627\u0628"},               // کتاب‌ها → کتاب
        {"\u0622\u062A\u0634\u0641\u0634\u0627\u0646", "", "\u0622\u062A\u0634\u0641\u0634\u0627\u0646"}, // آتشفشان → آتشفشان
        {"\u0645\u06CC\u200C\u0631\u0648\u0645", "", "\u0631\u0641\u062A#\u0631\u0648"},                 // می‌روم → رفت#رو
        {"\u06AF\u0641\u062A\u0647_\u0634\u062F\u0647_\u0627\u0633\u062A", "", "\u06AF\u0641\u062A#\u06AF\u0648"}, // گفته_شده_است → گفت#گو
        {"\u0646\u0686\u0634\u06CC\u062F\u0647_\u0627\u0633\u062A", "", "\u0686\u0634\u06CC\u062F#\u0686\u0634"}, // نچشیده_است → چشید#چش
        // POS-specific
        {"\u0645\u0631\u062F\u0645", "N", "\u0645\u0631\u062F\u0645"},                                 // مردم N → مردم
        {"\u0627\u062C\u062A\u0645\u0627\u0639\u06CC", "AJ", "\u0627\u062C\u062A\u0645\u0627\u0639\u06CC"}, // اجتماعی AJ → اجتماعی
        // Additional verb conjugations verified against Python
        {"\u0686\u0634\u06CC\u062F\u0645", "", "\u0686\u0634\u06CC\u062F#\u0686\u0634"},                 // چشیدم → چشید#چش
        {"\u0646\u0645\u06CC\u200C\u062E\u0648\u0631\u0645", "", "\u062E\u0648\u0631\u062F#\u062E\u0648\u0631"}, // نمی‌خورم → خورد#خور
        // Plural nouns (stemming via Lemmatizer)
        {"\u06A9\u062A\u0627\u0628\u200C\u0647\u0627", "", "\u06A9\u062A\u0627\u0628"},                 // کتاب‌ها → کتاب
    };
    
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failures;
    
    for (const auto& tc : test_cases) {
        std::string result = lemmatizer.lemmatize(tc.word, tc.pos);
        if (result == tc.expected) {
            passed++;
        } else {
            failed++;
            failures.push_back("Word: [" + tc.word + "] pos=[" + tc.pos + 
                              "] expected=[" + tc.expected + "] got=[" + result + "]");
        }
    }
    
    for (const auto& f : failures) {
        WARN(f);
    }
    INFO("Passed: " << passed << ", Failed: " << failed);
    
    REQUIRE(failed == 0);
}