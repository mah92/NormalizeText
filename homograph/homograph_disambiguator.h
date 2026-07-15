#ifndef HOMOGRAPH_DISAMBIGUATOR_H
#define HOMOGRAPH_DISAMBIGUATOR_H

#include "homograph_dict.h"
#include "hazm/lemmatizer.hpp"
#include <string>
#include <unordered_set>
#include <vector>

namespace homograph {

/**
 * Disambiguate a Persian homograph word using context-lemma scoring.
 *
 * @param word           The homograph word (grapheme)
 * @param sentenceWords  All words in the sentence (used for context)
 * @param dict           The homograph dictionary
 * @param lemmatizer     hazm Lemmatizer instance
 * @param stopwords      Set of Persian stopwords to exclude from context
 * @param fallbackPhoneme If word not in dict, this default IPA is returned (empty = not found)
 * @return               Best IPA phoneme string, or fallback if not a homograph
 */
std::string disambiguateHomograph(
    const std::string& word,
    const std::vector<std::string>& sentenceWords,
    const HomographDict& dict,
    hazm::Lemmatizer& lemmatizer,
    const std::unordered_set<std::string>& stopwords,
    const std::string& fallbackPhoneme = "");

} // namespace homograph

#endif // HOMOGRAPH_DISAMBIGUATOR_H