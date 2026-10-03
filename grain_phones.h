/*
 * grain_phones — NormalizeText -> GrainSpeech model phone format.
 *
 * ONE general path, the same one the training front-end used (phonemize_ipa.py):
 *
 *   text -> NormalizeText (normalized + IPA) -> phone tokens -> language tag
 *
 * Phone conventions: a token may hold several characters — stress (ˈ ˌ) attaches to the
 * FOLLOWING vowel, length (ː ˑ) and combining diacritics attach to the PRECEDING base, a
 * tie bar (͡ ͜) joins two symbols into one affricate — and every token is tagged with the
 * utterance language (fa: / en:).
 *
 * There is deliberately NO lexicon here. An inference-side word list (infer_lex_fa.txt /
 * infer_lex_en.txt, derived from the training alignments) was tried in September 2026 and
 * removed on Ali's decision (2026-10-03: «واژهنامه باید حذف بشه»). That layer is what made
 * the engine silently drop words, invented the whole «unknown word» class (32% of one real
 * message was not in it) and, when patched with index-based group matching, read sentences
 * out of order. The IPA stream is used in order, so no word can be dropped or displaced.
 *
 * Keep in sync with phonemize_ipa.py (ipa_to_phonemes + tag_language).
 */
#ifndef GRAIN_PHONES_H
#define GRAIN_PHONES_H

#include <string>
#include <vector>

namespace grain {

// One IPA group (e.g. "salˈɑm") -> phoneme tokens ({"s","a","l","ˈɑ","m"}).
std::vector<std::string> ipaGroupToPhonemes(const std::string& group);

// Normalized text + its IPA -> model-format phone string, e.g. "fa:s fa:a fa:l fa:ˈɑ fa:m".
std::string toModelPhones(const std::string& normalized, const std::string& ipa,
                          const std::string& langTag);

}  // namespace grain

#endif  // GRAIN_PHONES_H
