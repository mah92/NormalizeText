#ifndef NORMALIZE_H
#define NORMALIZE_H

#include <string>
#include <vector>
#include <memory>
#include "language_detector/language_detector.h"

void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
    std::string &normalizedString, std::string &ipaString);

#endif // NORMALIZE_H