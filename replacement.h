#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include <string>
#include <vector>
#include "language_detector/language_detector.h"

// Helper functions
std::vector<std::string> split(const std::string& s, char delimiter);
std::string join(const std::vector<std::string>& vec, char delimiter);

// Unicode conversion utilities
std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& str);

// Main replacement function
std::string performReplacements(Language mainlang, const std::string& input);

#endif // REPLACEMENT_H