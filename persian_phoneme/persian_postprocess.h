#ifndef PERSIAN_POSTPROCESS_H
#define PERSIAN_POSTPROCESS_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "../homograph/homograph_dict.h"
#include "hazm/lemmatizer.hpp"
#include "ezafe_detector.hpp"

namespace persian_phoneme {

/**
 * Initialize all Persian processing resources once.
 * Call this once at startup before using postprocessPersianIPA.
 *
 * @param ezafeOnnxPath    Path to model_quantized.onnx
 * @param ezafeSpiecePath  Path to spiece.model
 * @param hazmWordsPath    Path to words.dat
 * @param hazmVerbsPath    Path to verbs.dat
 * @param hazmStopwordsPath Path to stopwords.dat
 * @param homographJsonPath Path to homograph_data.json
 */
void initPersianResources(
    const std::string& ezafeOnnxPath,
    const std::string& ezafeSpiecePath,
    const std::string& hazmWordsPath,
    const std::string& hazmVerbsPath,
    const std::string& hazmStopwordsPath,
    const std::string& homographJsonPath);

/**
 * Post-process an eSpeak IPA string for Persian:
 *  - Disambiguate homograph pronunciations using context
 *  - Add ezafe suffixes (-e / -je) where predicted
 *
 * @param text      Original Persian text (used for word boundaries and ezafe detection)
 * @param ipaString The IPA phoneme string produced by eSpeak
 * @return          Corrected IPA string
 */
std::string postprocessPersianIPA(
    const std::string& text,
    const std::string& ipaString);

} // namespace persian_phoneme

#endif // PERSIAN_POSTPROCESS_H