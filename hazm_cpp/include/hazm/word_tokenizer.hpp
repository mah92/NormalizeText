#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "hazm/utils.hpp"

namespace hazm {

class WordTokenizer {
public:
    WordTokenizer(const std::string& words_file = "",
                  const std::string& verbs_file = "",
                  bool join_verb_parts = true);
    
    // Word dictionary: word -> (count, tags)
    std::unordered_map<std::string, WordEntry> words;
    
    // Verb list (past#present strings)
    std::vector<std::string> verbs;
    
    // Derived verb data (for join_verb_parts)
    std::unordered_set<std::string> bons;         // past roots
    std::unordered_set<std::string> verbe;         // bons + "ه", "ن" + bons + "ه"
    std::unordered_set<std::string> before_verbs;
    std::unordered_set<std::string> after_verbs;

private:
    void init_verb_parts(const std::string& verbs_file);
};

} // namespace hazm