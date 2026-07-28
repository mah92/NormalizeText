#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>
#include "hazm/lemmatizer.hpp"
#include "hazm/stopwords.hpp"

static std::string P(const char* u8) { return std::string(u8); }

// Helper: extract Persian words from text and lemmatize them
// This mirrors the extract_and_lemmatize_persian_words function from correct_phonemes.py
static std::vector<std::pair<std::string, std::string>> lemmatize_text(
    const std::string& text,
    hazm::Lemmatizer& lemmatizer,
    const std::unordered_set<std::string>& stopwords)
{
    std::vector<std::pair<std::string, std::string>> results;
    // Simple Persian regex extraction (matching correct_phonemes.py)
    std::string persian_chars = 
        "\u0627\u0622\u0628\u067E\u062A\u062B\u062C\u0686\u062D\u062E"
        "\u062F\u0630\u0631\u0632\u0698\u0633\u0634\u0635\u0636\u0637"
        "\u0638\u0639\u063A\u0641\u0642\u06A9\u06AF\u0644\u0645\u0646"
        "\u0648\u0647\u06CC";
    
    // Extract Persian words manually (C++ doesn't have Python's findall with Unicode ranges easily)
    std::string current;
    for (size_t i = 0; i < text.size(); ) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        std::string ch;
        size_t len = 1;
        
        if (c < 0x80) { ch = text.substr(i, 1); len = 1; }
        else if ((c & 0xE0) == 0xC0) { ch = text.substr(i, 2); len = 2; }
        else if ((c & 0xF0) == 0xE0) { ch = text.substr(i, 3); len = 3; }
        else if ((c & 0xF8) == 0xF0) { ch = text.substr(i, 4); len = 4; }
        
        if (len > 1 && persian_chars.find(ch) != std::string::npos) {
            current += ch;
        } else {
            if (current.size() > 1 && stopwords.find(current) == stopwords.end()) {
                std::string lemma = lemmatizer.lemmatize(current);
                results.push_back({current, lemma});
            }
            current.clear();
        }
        i += len;
    }
    // Last word
    if (current.size() > 1 && stopwords.find(current) == stopwords.end()) {
        std::string lemma = lemmatizer.lemmatize(current);
        results.push_back({current, lemma});
    }
    
    return results;
}

TEST_CASE("Sample texts - lemmatize words from correct_phonemes.py style input", "[sample_texts]") {
    hazm::Lemmatizer lemmatizer;
    auto stopwords = hazm::stopwords_list();
    
    // Sample texts from user
    struct TestText {
        std::string text;
        int expected_min_words;
    };
    
    std::vector<TestText> samples = {
        {P("\u0645\u062F\u062A\u06CC\u0647 \u062F\u0631\u06AF\u06CC\u0631 \u067E\u0631\u0648\u0698\u0647 \u062F\u06CC\u06AF\u0631\u06CC \u0628\u0648\u062F\u0645."), 3},
        {P("\u0633\u0627\u06CC\u0646\u0633 \u0645\u062D\u0635\u0648\u0644 \u063A\u0627\u0644\u0628\u0627 \u0645\u0641\u06CC\u062F\u06CC \u0627\u0633\u062A \u06A9\u0647 \u062F\u0631 \u0628\u0646\u06CC\u0627\u062F\u0647\u0627\u06CC\u0634 \u062A\u0631\u06A9\u06CC\u0628\u06CC \u0627\u0633\u062A \u0627\u0632"), 5},
        {P("\u06AF\u0648\u0634\u06CC \u0634\u06CC\u0627\u0626\u0648\u0645\u06CC \u0648 \u062A\u0628\u0644\u062A \u0647\u0648\u0634\u0645\u0646\u062F"), 4},
        {P("\u0627\u0645\u06CC\u0631"), 1}, // امیر
    };
    
    for (const auto& s : samples) {
        auto results = lemmatize_text(s.text, lemmatizer, stopwords);
        INFO("Text: " << s.text);
        INFO("Found " << results.size() << " words");
        for (const auto& [word, lemma] : results) {
            INFO("  " << word << " -> " << lemma);
        }
        REQUIRE(results.size() >= size_t(s.expected_min_words));
    }
}

TEST_CASE("Sample text - large Persian paragraph", "[sample_texts]") {
    hazm::Lemmatizer lemmatizer;
    auto stopwords = hazm::stopwords_list();
    
    // Large sample text from user (first few sentences)
    std::string text = P(
        "\u062D\u0632\u0628\u200C\u0627\u0644\u0644\u0647 \u0646\u06CC\u0632 \u0647\u0645\u0627\u0646\u06AF\u0648\u0646\u0647 \u06A9\u0647 "
        "\u062F\u0631 \u06CC\u06A9\u06CC \u0627\u0632 \u06AF\u0632\u0627\u0631\u0634\u200C\u0647\u0627 \u0646\u0648\u0634\u062A\u0645 \u0628\u0647 "
        "\u0647\u06CC\u0686 \u0639\u0646\u0648\u0627\u0646 \u0646\u0645\u06CC\u200C\u062A\u0648\u0627\u0646\u0633\u062A \u0628\u0627 "
        "\u067E\u0627\u06CC\u0627\u0646 \u062C\u0646\u06AF \u0645\u062E\u0627\u0644\u0641\u062A \u06A9\u0646\u062F"
    );
    
    auto results = lemmatize_text(text, lemmatizer, stopwords);
    REQUIRE(results.size() > 0);
    
    // Check specific lemmas
    // حزب‌الله → حزب‌الله (should stay)
    // نوشتم → نوشت#نویس
    bool found_goft = false;
    for (const auto& [word, lemma] : results) {
        if (word == P("\u0646\u0648\u0634\u062A\u0645")) { // نوشتم
            // Should map to a verb lemma
            REQUIRE(lemma.find('#') != std::string::npos);
        }
    }
}