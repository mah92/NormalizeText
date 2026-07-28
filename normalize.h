#ifndef NORMALIZE_H
#define NORMALIZE_H

#include <string>
#include <vector>
#include <memory>
#include "language_detector/language_detector.h"

struct NormalizeConfig {
    std::string espeak_data_path;       // espeak-ng-data directory (empty = default)
    std::string shakkelha_onnx;         // shakkelha.onnx for Arabic diacritization
    std::string ezafe_model_onnx;       // model_quantized.onnx for Persian ezafe
    std::string ezafe_model_spiece;     // spiece.model for Persian ezafe tokenizer
    std::string hazm_words;             // words.dat for Persian processing
    std::string hazm_verbs;             // verbs.dat for Persian processing
    std::string hazm_stopwords;         // stopwords.dat for Persian processing
    std::string homograph_data;         // homograph_data.json for Persian homograph
};

// Original signature (kept for backward compatibility, uses default paths)
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
    std::string &normalizedString, std::string &ipaString);

// New signature with config (all paths from caller, no hardcoded paths)
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
    std::string &normalizedString, std::string &ipaString,
    const NormalizeConfig& config);

#endif // NORMALIZE_H