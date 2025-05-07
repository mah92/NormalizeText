#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include <string>
#include <vector>
#include "../language_detector/language_detector.h"

std::string performGeneralReplacements(Language mainlang, const std::string& input);
std::string performLanguageSpecificReplacements(Language language, Language mainlang, const std::string& input);

#endif // REPLACEMENT_H