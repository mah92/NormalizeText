#ifndef NORMALIZE_H
#define NORMALIZE_H

#include <string>
#include <vector>
#include <memory>
#include "language_detector/language_detector.h"

void normalizeString(Language mainlang, const std::string& input,
    std::string &normalizedString, std::vector <std::vector <char32_t>> &phonemes, std::string &phonemeString, std::string &idString);

#endif // NORMALIZE_H