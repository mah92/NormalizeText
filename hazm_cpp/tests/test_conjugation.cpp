#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <string>
#include "hazm/conjugation.hpp"

// Helper to create Persian strings more readably
static std::string P(const char* u8) { return std::string(u8); }

TEST_CASE("Conjugation - perfective_past", "[conjugation]") {
    hazm::Conjugation conj;
    auto result = conj.perfective_past(P("\u062F\u06CC\u062F")); // دید
    std::vector<std::string> expected = {
        P("\u062F\u06CC\u062F\u0645"),      // دیدم
        P("\u062F\u06CC\u062F\u06CC"),     // دیدی
        P("\u062F\u06CC\u062F"),           // دید
        P("\u062F\u06CC\u062F\u06CC\u0645"), // دیدیم
        P("\u062F\u06CC\u062F\u06CC\u062F"), // دیدید
        P("\u062F\u06CC\u062F\u0646\u062F"), // دیدند
    };
    REQUIRE(result == expected);
}

TEST_CASE("Conjugation - negative_perfective_past", "[conjugation]") {
    hazm::Conjugation conj;
    auto result = conj.negative_perfective_past(P("\u062F\u06CC\u062F"));
    std::vector<std::string> expected = {
        P("\u0646\u062F\u06CC\u062F\u0645"),
        P("\u0646\u062F\u06CC\u062F\u06CC"),
        P("\u0646\u062F\u06CC\u062F"),
        P("\u0646\u062F\u06CC\u062F\u06CC\u0645"),
        P("\u0646\u062F\u06CC\u062F\u06CC\u062F"),
        P("\u0646\u062F\u06CC\u062F\u0646\u062F"),
    };
    REQUIRE(result == expected);
}

TEST_CASE("Conjugation - perfective_present", "[conjugation]") {
    hazm::Conjugation conj;
    auto result = conj.perfective_present(P("\u0628\u06CC\u0646")); // بین
    std::vector<std::string> expected = {
        P("\u0628\u06CC\u0646\u0645"),      // بینم
        P("\u0628\u06CC\u0646\u06CC"),     // بینی
        P("\u0628\u06CC\u0646\u062F"),     // بیند
        P("\u0628\u06CC\u0646\u06CC\u0645"), // بینیم
        P("\u0628\u06CC\u0646\u06CC\u062F"), // بینید
        P("\u0628\u06CC\u0646\u0646\u062F"), // بینند
    };
    REQUIRE(result == expected);
}

TEST_CASE("Conjugation - imperfective_past", "[conjugation]") {
    hazm::Conjugation conj;
    auto result = conj.imperfective_past(P("\u062F\u06CC\u062F"));
    REQUIRE(result.size() == 6);
    REQUIRE(result[0] == P("\u0645\u06CC\u200C\u062F\u06CC\u062F\u0645")); // می‌دیدم
}

TEST_CASE("Conjugation - present_perfect", "[conjugation]") {
    hazm::Conjugation conj;
    auto result = conj.present_perfect(P("\u062F\u06CC\u062F"));
    REQUIRE(result.size() == 7);
    // First form: دیده‌ام
    REQUIRE(result[0] == P("\u062F\u06CC\u062F\u0647\u200C\u0627\u0645"));
    // Fourth form (no suffix): دیده
    REQUIRE(result[3] == P("\u062F\u06CC\u062F\u0647"));
}

TEST_CASE("Conjugation - get_all", "[conjugation]") {
    hazm::Conjugation conj;
    auto result = conj.get_all(P("\u062F\u06CC\u062F#\u0628\u06CC\u0646")); // دید#بین
    // Should generate many forms (approximately 110+)
    REQUIRE(result.size() > 100);
    // First is infinitive: دیدن
    REQUIRE(result[0] == P("\u062F\u06CC\u062F\u0646"));
}