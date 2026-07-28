#include "hazm/lemmatizer.hpp"

namespace hazm {

Lemmatizer::Lemmatizer(const std::string& words_file,
                       const std::string& verbs_file,
                       bool joined_verb_parts) {
    // Load words and verb data via WordTokenizer
    std::string wf = words_file.empty() ? data_path("words.dat") : words_file;
    std::string vf = verbs_file.empty() ? data_path("verbs.dat") : verbs_file;
    
    WordTokenizer tokenizer(wf, vf, joined_verb_parts);
    words = tokenizer.words;
    
    // Build verb conjugation dictionary (same as Python Lemmatizer.__init__)
    // Python: self.verbs["است"] = "#است"
    verbs["\u0627\u0633\u062A"] = "#\u0627\u0633\u062A";  // است → #است
    
    for (const auto& verb : tokenizer.verbs) {
        for (const auto& tense : conjugation.get_all(verb)) {
            verbs[tense] = verb;
        }
    }
    
    if (joined_verb_parts) {
        for (const auto& verb : tokenizer.verbs) {
            auto pos = verb.find('#');
            if (pos == std::string::npos) continue;
            std::string bon = verb.substr(0, pos);
            
            // for after_verb in tokenizer.after_verbs:
            //     self.verbs[f"{bon}ه_{after_verb}"] = verb
            //     self.verbs[f"ن{bon}ه_{after_verb}"] = verb
            for (const auto& after_verb : tokenizer.after_verbs) {
                verbs[bon + "\u0647_" + after_verb] = verb;
                verbs["\u0646" + bon + "\u0647_" + after_verb] = verb;
            }
            
            // for before_verb in tokenizer.before_verbs:
            //     self.verbs[f"{before_verb}_{bon}"] = verb
            for (const auto& before_verb : tokenizer.before_verbs) {
                verbs[before_verb + "_" + bon] = verb;
            }
        }
    }
}

std::string Lemmatizer::lemmatize(const std::string& word, const std::string& pos) const {
    // Python implementation (exact match):
    //
    // if not pos and word in self.words:
    //     return word
    if (pos.empty() && words.find(word) != words.end()) {
        return word;
    }
    
    // if (not pos or pos == "VERB") and word in self.verbs:
    //     return self.verbs[word]
    if ((pos.empty() || pos == "VERB") && verbs.find(word) != verbs.end()) {
        return verbs.at(word);
    }
    
    // if pos.startswith("ADJ") and word.endswith("ی"):
    //     return word
    if (pos.size() >= 3 && pos.substr(0, 3) == "ADJ") {
        // Check if word ends with "ی" (U+06CC)
        if (!word.empty()) {
            std::string yeh = "\u06CC"; // Persian yeh
            if (word.size() >= 2 && 
                static_cast<unsigned char>(word[word.size() - 2]) == 0xDB &&
                static_cast<unsigned char>(word[word.size() - 1]) == 0x8C) {
                return word;
            }
        }
    }
    
    // if pos == "PRON":
    //     return word
    if (pos == "PRON") {
        return word;
    }
    
    // if word in self.words:
    //     return word
    if (words.find(word) != words.end()) {
        return word;
    }
    
    // stem = self.stemmer.stem(word)
    // if stem and stem in self.words:
    //     return stem
    std::string stem = stemmer.stem(word);
    if (!stem.empty() && words.find(stem) != words.end()) {
        return stem;
    }
    
    // return word
    return word;
}

} // namespace hazm