/*
 * grain_phones — NormalizeText -> GrainSpeech model phone format.
 *
 * The 16 kHz GrainSpeech model is trained on phoneme-level, language-tagged phone tokens
 * (e.g. "fa:ˈɑ", "en:iː"), where a token may hold several characters: stress (ˈ ˌ) attaches
 * to the FOLLOWING vowel, length (ː ˑ) and combining diacritics attach to the PRECEDING base,
 * and a tie bar (͡ ͜) joins two symbols into one affricate.
 *
 * Feeding the model the plain espeak/IPA character stream instead (different convention,
 * different word-level choices) makes synthesis unintelligible, so this module reproduces the
 * training convention exactly:
 *   - word-by-word lookup in the lexicon derived from the model's own alignments
 *     (infer_lex_fa.txt / infer_lex_en.txt, produced by extract_lexicon_from_alignments.py),
 *   - espeak IPA groups as the fallback for out-of-vocabulary words,
 *   - every token tagged with its language (fa: / en:).
 *
 * Port of phonemize_ipa.py + frontend_lexicon.py; keep the two in sync.
 */
#ifndef GRAIN_PHONES_H
#define GRAIN_PHONES_H

#include <map>
#include <string>
#include <vector>

namespace grain {

// word -> phone tokens (space separated, already tagged) from <dir>/infer_lex_<lang>.txt
std::map<std::string, std::string> loadInferenceLexicon(const std::string& dir,
                                                        const std::string& lang);

// One IPA group (e.g. "salˈɑm") -> phoneme tokens ({"s","a","l","ˈɑ","m"}).
std::vector<std::string> ipaGroupToPhonemes(const std::string& group);

// Normalized text + its IPA -> model-format phone string, e.g. "fa:s fa:a fa:l fa:ˈɑ fa:m".
std::string toModelPhones(const std::string& normalized, const std::string& ipa,
                          const std::map<std::string, std::string>& lexicon,
                          const std::string& langTag);

}  // namespace grain

#endif  // GRAIN_PHONES_H
