#include <catch2/catch_test_macros.hpp>
#include "hazm/stemmer.hpp"

TEST_CASE("Stemmer basic tests", "[stemmer]") {
    hazm::Stemmer stemmer;
    
    REQUIRE(stemmer.stem("\u06A9\u062A\u0627\u0628\u200C\u0647\u0627") == "\u06A9\u062A\u0627\u0628"); // کتاب‌ها → کتاب
    REQUIRE(stemmer.stem("\u06A9\u062A\u0627\u0628\u06CC") == "\u06A9\u062A\u0627\u0628"); // کتابی → کتاب
    REQUIRE(stemmer.stem("\u06A9\u062A\u0627\u0628\u200C\u0647\u0627\u06CC") == "\u06A9\u062A\u0627\u0628"); // کتاب‌هایی → کتاب
    REQUIRE(stemmer.stem("\u06A9\u062A\u0627\u0628\u0647\u0627\u06CC\u0634\u0627\u0646") == "\u06A9\u062A\u0627\u0628"); // کتابهایشان → کتاب
    REQUIRE(stemmer.stem("\u0627\u0646\u062F\u06CC\u0634\u0647\u200C\u0627\u0634") == "\u0627\u0646\u062F\u06CC\u0634\u0647"); // اندیشه‌اش → اندیشه
    REQUIRE(stemmer.stem("\u062E\u0627\u0646\u06C0") == "\u062E\u0627\u0646\u0647"); // خانۀ → خانه
}

TEST_CASE("Stemmer empty/edge cases", "[stemmer]") {
    hazm::Stemmer stemmer;
    
    REQUIRE(stemmer.stem("") == "");
    REQUIRE(stemmer.stem("\u0622\u0628") == "\u0622\u0628"); // آب → آب (water, no suffix)
}