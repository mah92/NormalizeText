#include "homograph_disambiguator.h"
#include <algorithm>
#include <unordered_map>

namespace homograph {

std::string disambiguateHomograph(
    const std::string& word,
    const std::vector<std::string>& sentenceWords,
    const HomographDict& dict,
    hazm::Lemmatizer& lemmatizer,
    const std::unordered_set<std::string>& stopwords,
    const std::string& fallbackPhoneme)
{
    // Not a homograph word — return fallback
    auto it = dict.find(word);
    if (it == dict.end()) {
        return fallbackPhoneme;
    }

    const auto& phonemeOptions = it->second;
    if (phonemeOptions.empty()) {
        return fallbackPhoneme;
    }

    // Build context word frequency map (lemmatized, excluding stopwords and the target word)
    std::unordered_map<std::string, int> contextFreq;
    for (const auto& w : sentenceWords) {
        if (w == word) continue;
        if (w.size() <= 1) continue;
        if (stopwords.count(w)) continue;
        std::string lemma = lemmatizer.lemmatize(w);
        if (lemma.size() <= 1) continue;
        contextFreq[lemma]++;
    }

    // For each phoneme option, compute normalized overlap score
    std::string bestPhoneme;
    double maxScore = -1.0;

    for (const auto& [phoneme, contextWordList] : phonemeOptions) {
        // Count frequencies of context words associated with this phoneme
        std::unordered_map<std::string, int> phonemeWordFreq;
        for (const auto& cw : contextWordList) {
            phonemeWordFreq[cw]++;
        }
        int totalWords = static_cast<int>(contextWordList.size());

        // Weighted overlap: sum over context words of (contextFreq * phonemeWordFreq)
        double weightedOverlap = 0.0;
        for (const auto& [ctxWord, ctxCount] : contextFreq) {
            auto pwf = phonemeWordFreq.find(ctxWord);
            if (pwf != phonemeWordFreq.end()) {
                weightedOverlap += static_cast<double>(ctxCount) * pwf->second;
            }
        }

        double normalizedScore = (totalWords > 0)
            ? weightedOverlap / static_cast<double>(totalWords)
            : 0.0;

        if (normalizedScore > maxScore) {
            maxScore = normalizedScore;
            bestPhoneme = phoneme;
        } else if (normalizedScore == maxScore && bestPhoneme.empty()) {
            // Tiebreaker: prefer first with this score
            bestPhoneme = phoneme;
        }
    }

    return bestPhoneme.empty() ? fallbackPhoneme : bestPhoneme;
}

} // namespace homograph