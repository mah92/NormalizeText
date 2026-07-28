#include <catch2/catch_test_macros.hpp>
#include <string>
#include "hazm/lemmatizer.hpp"

static std::string P(const char* u8) { return std::string(u8); }

TEST_CASE("Lemmatizer - basic lemmatization", "[lemmatizer]") {
    hazm::Lemmatizer lemmatizer;
    
    // کتاب‌ها → کتاب
    REQUIRE(lemmatizer.lemmatize(P("\u06A9\u062A\u0627\u0628\u200C\u0647\u0627")) == 
            P("\u06A9\u062A\u0627\u0628"));
    
    // آتشفشان → آتشفشان (already lemma)
    REQUIRE(lemmatizer.lemmatize(P("\u0622\u062A\u0634\u0641\u0634\u0627\u0646")) == 
            P("\u0622\u062A\u0634\u0641\u0634\u0627\u0646"));
    
    // می‌روم → رفت#رو
    REQUIRE(lemmatizer.lemmatize(P("\u0645\u06CC\u200C\u0631\u0648\u0645")) == 
            P("\u0631\u0641\u062A#\u0631\u0648"));
    
    // گفته_شده_است → گفت#گو
    REQUIRE(lemmatizer.lemmatize(P("\u06AF\u0641\u062A\u0647_\u0634\u062F\u0647_\u0627\u0633\u062A")) == 
            P("\u06AF\u0641\u062A#\u06AF\u0648"));
    
    // نچشیده_است → چشید#چش
    REQUIRE(lemmatizer.lemmatize(P("\u0646\u0686\u0634\u06CC\u062F\u0647_\u0627\u0633\u062A")) == 
            P("\u0686\u0634\u06CC\u062F#\u0686\u0634"));
}

TEST_CASE("Lemmatizer - with POS", "[lemmatizer]") {
    hazm::Lemmatizer lemmatizer;
    
    // مردم with N → مردم (noun, already lemma)
    REQUIRE(lemmatizer.lemmatize(P("\u0645\u0631\u062F\u0645"), "N") == 
            P("\u0645\u0631\u062F\u0645"));
    
    // اجتماعی with AJ → اجتماعی (adjective with ی, stays)
    REQUIRE(lemmatizer.lemmatize(P("\u0627\u062C\u062A\u0645\u0627\u0639\u06CC"), "AJ") == 
            P("\u0627\u062C\u062A\u0645\u0627\u0639\u06CC"));
}