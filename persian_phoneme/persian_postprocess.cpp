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
            // U+02D0 (ː) as length marker. Check the last meaningful phoneme
            // codepoint (skip stress markers and length markers).
            size_t len = currentIpa.size();
            if (len >= 1) {
                // Find the last meaningful Unicode codepoint by scanning
                // backwards through full UTF-8 codepoints, skipping
                // diacritic/modifier characters (stress ˈˌ and length ː).
                // This correctly handles words ending with non-ASCII
                // phonemes like 'ʃ' (U+0283), 'ʒ' (U+0292), etc.
                char32_t lastPhoneme = 0;
                size_t pos = len;
                while (pos > 0) {
                    // Determine the start of the current UTF-8 codepoint
                    size_t cp_start = pos - 1;
                    unsigned char c = static_cast<unsigned char>(currentIpa[cp_start]);
                    // Find the beginning of this multi-byte sequence
                    while (cp_start > 0 && (c & 0xC0) == 0x80) {
                        --cp_start;
                        c = static_cast<unsigned char>(currentIpa[cp_start]);
                    }
                    // Decode the codepoint
                    size_t cp_len = pos - cp_start;
                    char32_t cp;
                    if (cp_len == 1) {
                        cp = static_cast<char32_t>(c);
                    } else if (cp_len == 2) {
                        cp = ((c & 0x1F) << 6) |
                             (static_cast<unsigned char>(currentIpa[cp_start + 1]) & 0x3F);
                    } else if (cp_len == 3) {
                        cp = ((c & 0x0F) << 12) |
                             ((static_cast<unsigned char>(currentIpa[cp_start + 1]) & 0x3F) << 6) |
                             (static_cast<unsigned char>(currentIpa[cp_start + 2]) & 0x3F);
                    } else {
                        cp = ((c & 0x07) << 18) |
                             ((static_cast<unsigned char>(currentIpa[cp_start + 1]) & 0x3F) << 12) |
                             ((static_cast<unsigned char>(currentIpa[cp_start + 2]) & 0x3F) << 6) |
                             (static_cast<unsigned char>(currentIpa[cp_start + 3]) & 0x3F);
                    }
                    // Skip stress markers (ˈ U+02C8, ˌ U+02CC) and
                    // length markers (ː U+02D0)
                    if (cp != 0x02C8 && cp != 0x02CC && cp != 0x02D0) {
                        lastPhoneme = cp;
                        break;
                    }
                    pos = cp_start;
                }

                if (lastPhoneme == U'i') {
                    currentIpa += "je";
                } else if (lastPhoneme != U'e') {
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