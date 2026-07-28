#pragma once

#include <string>
#include <unordered_set>
#include <vector>

namespace hazm {

// Returns the set of Persian stopwords (read from data/stopwords.dat)
// Matches Python hazm.utils.stopwords_list()
std::unordered_set<std::string> stopwords_list(const std::string& path = "");

} // namespace hazm