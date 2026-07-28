#pragma once

#include <string>
#include <vector>

namespace hazm {

class Stemmer {
public:
    Stemmer();
    
    // Stem a word by removing the longest matching suffix.
    // Matches Python hazm.stemmer.Stemmer.stem() exactly.
    std::string stem(const std::string& word) const;

private:
    std::vector<std::string> ends;
};

} // namespace hazm