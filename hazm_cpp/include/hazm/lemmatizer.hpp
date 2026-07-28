#pragma once

#include <string>
#include <unordered_map>
#include "hazm/stemmer.hpp"
#include "hazm/conjugation.hpp"
#include "hazm/word_tokenizer.hpp"

namespace hazm {

class Lemmatizer {
public:
    Lemmatizer(const std::string& words_file = "",
               const std::string& verbs_file = "",
               bool joined_verb_parts = true);
    
    std::string lemmatize(const std::string& word, const std::string& pos = "") const;

private:
    std::unordered_map<std::string, WordEntry> words;
    std::unordered_map<std::string, std::string> verbs;  // conjugated form → verb lemma (past#present)
    Stemmer stemmer;
    Conjugation conjugation;
};

} // namespace hazm