#include <catch2/catch_test_macros.hpp>
#include "hazm/stopwords.hpp"

TEST_CASE("Stopwords - basic", "[stopwords]") {
    auto sw = hazm::stopwords_list();
    REQUIRE(!sw.empty());
    // Check a few common Persian stopwords exist
    REQUIRE(sw.find("\u0648") != sw.end());                         // و
    REQUIRE(sw.find("\u062F\u0631") != sw.end());                  // در
    REQUIRE(sw.find("\u0628\u0647") != sw.end());                  // به
    REQUIRE(sw.find("\u0627\u0632") != sw.end());                  // از
    REQUIRE(sw.find("\u06A9\u0647") != sw.end());                  // که
    REQUIRE(sw.find("\u0627\u06CC\u0646") != sw.end());            // این
}

TEST_CASE("Stopwords - return type", "[stopwords]") {
    auto sw = hazm::stopwords_list();
    // Should have 389 stopwords
    REQUIRE(sw.size() == 389);
}