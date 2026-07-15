#include "persian_postprocess.h"
#include "../homograph/homograph_disambiguator.h"
#include "hazm/stopwords.hpp"

#include <algorithm>
#include <memory>
#include <sstream>

namespace persian_phoneme {

// Static globals, initialized once
static std::unique_ptr<EzafeDetector> g_ezafeDetector;
static std::unique_ptr<hazm::Lemmatizer> g_lemmatizer;
static std::unordered_set<std::string> g_stopwords;
static homograph::HomographDict g_homographDict;
static bool g_initialized = false;

void initPersianResources(
    const std::string& ezafeOnnxPath,
    const std::string& ezafeSpiecePath,
    const std::string& hazmWordsPath,
    const std::string& hazmVerbsPath,
    const std::string& hazmStopwordsPath,
    const std::string& homographJsonPath)
{
    if (g_initialized) return;

    g_ezafeDetector = std::make_unique<EzafeDetector>(ezafeOnnxPath, ezafeSpiecePath);
    g_lemmatizer = std::make_unique<hazm::Lemmatizer>(hazmWordsPath, hazmVerbsPath, true);
    g_stopwords = hazm::stopwords_list(hazmStopwordsPath);
    g_homographDict = homograph::loadHomographDict(homographJsonPath);

    g_initialized = true;
}

// Helper: split string by whitespace
static std::vector<std::string> splitBySpace(const std::string& s) {
    std::vector<std::string> result;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

// Helper: remove Persian punctuation from word for dictionary lookup
static std::string stripPersianPunctuation(const std::string& word) {
    std::string result;
    for (size_t i = 0; i < word.size(); ) {
        unsigned char c = static_cast<unsigned char>(word[i]);
        if (c < 0x80) {
            // ASCII — skip punctuation
            if (std::isalpha(c) || std::isdigit(c)) {
                result += c;
            }
            ++i;
        } else {
            // Multi-byte UTF-8 — extract full codepoint
            size_t len = 1;
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;

            // Persian punctuation to strip: ، ؛ ؟ « » 
            // (U+060C, U+061B, U+061F, U+00AB, U+00BB)
            bool isPunct = false;
            if (len == 2) {
                uint16_t cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(word[i+1]) & 0x3F);
                if (cp == 0x060C || cp == 0x061B || cp == 0x061F || cp == 0x00AB || cp == 0x00BB)
                    isPunct = true;
            }

            if (!isPunct) {
                result.append(word, i, len);
            }
            i += len;
        }
    }
    return result;
}

std::string postprocessPersianIPA(
    const std::string& text,
    const std::string& ipaString)
{
    if (!g_initialized) return ipaString;

    // Split text and IPA into words
    auto textWords = splitBySpace(text);
    if (textWords.empty()) return ipaString;

    auto ipaWords = splitBySpace(ipaString);
    // Handle edge case: IPA may have fewer words due to punctuation collapse
    size_t minWords = std::min(textWords.size(), ipaWords.size());
    if (minWords == 0) return ipaString;

    // --- Step 1: Ezafe detection ---
    // Prepare clean words (no punctuation, for ezafe model)
    std::vector<std::string> cleanWords;
    std::vector<std::string> originalTextWords;
    for (size_t i = 0; i < minWords; ++i) {
        originalTextWords.push_back(textWords[i]);
        cleanWords.push_back(stripPersianPunctuation(textWords[i]));
    }

    // Predict ezafe for all Persian words
    std::vector<EzafeDetector::Result> ezafeResults;
    try {
        ezafeResults = g_ezafeDetector->predict(cleanWords);
    } catch (...) {
        // If ezafe prediction fails, continue without it
        return ipaString;
    }

    // --- Step 2: Homograph disambiguation ---
    // Build list of cleaned Persian words (stripped of punctuation) for context
    // and use original words for dictionary lookup
    std::vector<std::string> cleanedSentenceWords;
    for (const auto& w : originalTextWords) {
        std::string clean = stripPersianPunctuation(w);
        if (!clean.empty()) {
            cleanedSentenceWords.push_back(clean);
        }
    }

    // --- Step 3: Build corrected IPA ---
    std::ostringstream result;
    for (size_t i = 0; i < minWords; ++i) {
        if (i > 0) result << ' ';

        std::string ipaWord = (i < ipaWords.size()) ? ipaWords[i] : "";
        std::string textWord = originalTextWords[i];
        std::string cleanWord = cleanWords[i];

        if (cleanWord.empty()) {
            result << ipaWord;
            continue;
        }

        // Homograph disambiguation: check if this word is a homograph
        std::string currentIpa = ipaWord;
        if (g_homographDict.count(cleanWord) > 0) {
            std::string disambiguated = homograph::disambiguateHomograph(
                cleanWord, cleanedSentenceWords, g_homographDict,
                *g_lemmatizer, g_stopwords, ipaWord);
            if (!disambiguated.empty()) {
                currentIpa = disambiguated;
            }
        }

        // Ezafe suffix
        bool needsEzafe = false;
        if (i < ezafeResults.size()) {
            needsEzafe = ezafeResults[i].needs_ezafe &&
                         ezafeResults[i].confidence >= 0.7f;
        }

        if (needsEzafe) {
            // Check if word ends with 'i' or 'iː' → append 'je'
            // Otherwise append 'e' (if not already ending with 'e' or 'eː')
            // The IPA from eSpeak for Persian uses plain ASCII chars i/e plus
            // U+02D0 (ː) as length marker. Check the last visible char.
            size_t len = currentIpa.size();
            if (len >= 1) {
                // Find the last visible ASCII char before any length marker
                char lastAscii = 0;
                for (size_t pos = len; pos > 0; --pos) {
                    unsigned char c = static_cast<unsigned char>(currentIpa[pos-1]);
                    if (c < 0x80 && std::isalpha(c)) {
                        lastAscii = static_cast<char>(c);
                        break;
                    }
                }
                if (lastAscii == 'i') {
                    currentIpa += "je";
                } else if (lastAscii != 'e') {
                    currentIpa += "e";
                }
            }
        }

        result << currentIpa;
    }

    // Append any remaining IPA words beyond minWords (shouldn't normally happen)
    for (size_t i = minWords; i < ipaWords.size(); ++i) {
        result << ' ' << ipaWords[i];
    }

    return result.str();
}

} // namespace persian_phoneme