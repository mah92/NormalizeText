#include "hazm/stemmer.hpp"
#include "hazm/constants.hpp"
#include <algorithm>

namespace hazm {

Stemmer::Stemmer() {
    // Build sorted suffix list from constants
    // Python: self.ends = sorted(SUFFIXES | {"ٔ", "‌ا", "‌"}, key=len, reverse=True)
    ends = {
        "هایمان", "هایتان", "هایشان",
        "هایی",
        "های",
        "ترین",
        "تری",
        "ها",
        "تر",
        "گری",
        "گر",
        "اش",
        "یم",
        "ید",
        "ند",
        "ام",
        "ات",
        "مان",
        "تان",
        "شان",
        "ان",
        "ین",
        "ای",
        "ی",
        "م",
        "ت",
        "ش",
    };
    
    // Additional suffixes from Python: {"ٔ", "‌ا", "‌"}
    ends.push_back("\u0654");      // ARABIC HAMZA ABOVE (ٔ)
    ends.push_back("\u200c\u0627"); // ZWNJ + alef
    ends.push_back("\u200c");       // ZWNJ
    
    // Sort by length descending (longest first), matching Python's key=len reverse=True
    std::sort(ends.begin(), ends.end(), [](const std::string& a, const std::string& b) {
        return a.size() > b.size();
    });
}

std::string Stemmer::stem(const std::string& word) const {
    // Python implementation:
    //   for end in self.ends:
    //       if word.endswith(end):
    //           if len(end) == 1 and len(word) - len(end) < 3:
    //               continue
    //           word = word[:-len(end)]
    //           break
    //   if word.endswith("ۀ"):
    //       word = word[:-1] + "ه"
    //   if word.endswith("\u200c"):
    //       word = word[:-1]
    //   return word
    
    std::string result = word;
    
    for (const auto& end : ends) {
        if (result.size() >= end.size() && 
            result.compare(result.size() - end.size(), end.size(), end) == 0) {
            // If suffix is 1 char and remaining word < 3 chars, skip
            if (end.size() == 1 && (result.size() - end.size()) < 3) {
                continue;
            }
            result = result.substr(0, result.size() - end.size());
            break;
        }
    }
    
    // Convert ۀ (U+06C0) to ه (U+0647) at end
    if (!result.empty() && result.size() >= 2) {
        // Check for ۀ which is \u06c0 (ARABIC LETTER HEH WITH YEH ABOVE)
        // It's a 2-byte UTF-8: D9 80
        // But we'll check the last code point
        unsigned char c1 = static_cast<unsigned char>(result[result.size() - 2]);
        unsigned char c2 = static_cast<unsigned char>(result[result.size() - 1]);
        if (c1 == 0xDB && c2 == 0x80) { // U+06C0 = ۀ
            result = result.substr(0, result.size() - 2) + "\u0647"; // ه
        }
    }
    
    // Remove trailing ZWNJ
    if (!result.empty()) {
        auto zwnj = "\u200c";
        size_t zwnj_len = 3; // ZWNJ is 0xE2 0x80 0x8C
        if (result.size() >= zwnj_len && 
            result.compare(result.size() - zwnj_len, zwnj_len, zwnj) == 0) {
            result = result.substr(0, result.size() - zwnj_len);
        }
    }
    
    return result;
}

} // namespace hazm