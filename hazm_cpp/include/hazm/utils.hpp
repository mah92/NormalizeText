#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace hazm {

// WordEntry matches Python: (count, tuple of POS tags)
struct WordEntry {
    int count;
    std::vector<std::string> tags;
};

// Load words.dat: each line is "word\tcount\ttag1,tag2,..."
// Returns unordered_map from word to WordEntry
inline std::unordered_map<std::string, WordEntry> load_words(const std::string& path) {
    std::unordered_map<std::string, WordEntry> words;
    std::ifstream file(path);
    if (!file.is_open()) return words;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Split by tab
        std::vector<std::string> parts;
        std::istringstream iss(line);
        std::string part;
        while (std::getline(iss, part, '\t')) {
            parts.push_back(part);
        }
        if (parts.size() != 3) continue;
        
        WordEntry entry;
        entry.count = std::stoi(parts[1]);
        
        // Split tags by comma
        std::istringstream tags_ss(parts[2]);
        std::string tag;
        while (std::getline(tags_ss, tag, ',')) {
            entry.tags.push_back(tag);
        }
        
        words[parts[0]] = entry;
    }
    return words;
}

// Load verbs.dat: each line is "past_root#present_root" or comment starting with #
// Returns vector of verb strings
inline std::vector<std::string> load_verbs_raw(const std::string& path) {
    std::vector<std::string> verbs;
    std::ifstream file(path);
    if (!file.is_open()) return verbs;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  // Skip comments
        verbs.push_back(line);
    }
    return verbs;
}

// Load stopwords.dat: each line is a stopword, may be empty lines
// Returns sorted unique set
inline std::unordered_set<std::string> load_stopwords(const std::string& path) {
    std::unordered_set<std::string> stopwords;
    std::ifstream file(path);
    if (!file.is_open()) return stopwords;
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            stopwords.insert(line);
        }
    }
    return stopwords;
}

// Character mapping similar to Python's maketrans
// Returns mapping from char32_t to char32_t
inline std::unordered_map<char32_t, char32_t> maketrans(const std::string& a, const std::string& b) {
    std::unordered_map<char32_t, char32_t> table;
    
    // Parse UTF-8 strings into code points
    auto next_codepoint = [](const std::string& s, size_t& pos) -> char32_t {
        if (pos >= s.size()) return 0;
        unsigned char c = static_cast<unsigned char>(s[pos]);
        char32_t cp;
        if (c < 0x80) {
            cp = c;
            pos += 1;
        } else if ((c & 0xE0) == 0xC0) {
            cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[pos + 1]) & 0x3F);
            pos += 2;
        } else if ((c & 0xF0) == 0xE0) {
            cp = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[pos + 1]) & 0x3F) << 6) | (static_cast<unsigned char>(s[pos + 2]) & 0x3F);
            pos += 3;
        } else if ((c & 0xF8) == 0xF0) {
            cp = ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[pos + 1]) & 0x3F) << 12) | ((static_cast<unsigned char>(s[pos + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(s[pos + 3]) & 0x3F);
            pos += 4;
        } else {
            cp = c;
            pos += 1;
        }
        return cp;
    };
    
    // Encode code point to UTF-8 (but we pair char32_t → char32_t)
    // Actually we need char → char mapping for use with std::string
    // Python's maketrans creates ordinal mappings, which work on code points
    // For C++, we'll work directly on the full string using find/replace
    
    size_t sa = 0, sb = 0;
    while (sa < a.size() && sb < b.size()) {
        char32_t ca = next_codepoint(a, sa);
        char32_t cb = next_codepoint(b, sb);
        table[ca] = cb;
    }
    return table;
}

// Apply maketrans-style translation to a string
inline std::string translate(const std::string& text, const std::unordered_map<char32_t, char32_t>& table) {
    if (table.empty()) return text;
    
    std::string result;
    result.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        size_t cp_len;
        char32_t cp;
        
        if (c < 0x80) {
            cp = c;
            cp_len = 1;
        } else if ((c & 0xE0) == 0xC0) {
            cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(text[i + 1]) & 0x3F);
            cp_len = 2;
        } else if ((c & 0xF0) == 0xE0) {
            cp = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 6) | (static_cast<unsigned char>(text[i + 2]) & 0x3F);
            cp_len = 3;
        } else if ((c & 0xF8) == 0xF0) {
            cp = ((c & 0x07) << 18) | ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 12) | ((static_cast<unsigned char>(text[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(text[i + 3]) & 0x3F);
            cp_len = 4;
        } else {
            cp = c;
            cp_len = 1;
        }
        
        auto it = table.find(cp);
        if (it != table.end()) {
            // Encode replacement code point to UTF-8
            char32_t rcp = it->second;
            if (rcp < 0x80) {
                result += static_cast<char>(rcp);
            } else if (rcp < 0x800) {
                result += static_cast<char>(0xC0 | (rcp >> 6));
                result += static_cast<char>(0x80 | (rcp & 0x3F));
            } else if (rcp < 0x10000) {
                result += static_cast<char>(0xE0 | (rcp >> 12));
                result += static_cast<char>(0x80 | ((rcp >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (rcp & 0x3F));
            } else {
                result += static_cast<char>(0xF0 | (rcp >> 18));
                result += static_cast<char>(0x80 | ((rcp >> 12) & 0x3F));
                result += static_cast<char>(0x80 | ((rcp >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (rcp & 0x3F));
            }
        } else {
            result.append(text, i, cp_len);
        }
        i += cp_len;
    }
    return result;
}

// Simple character-level string replace (for ASCII-range chars used in number mapping)
inline std::string simple_maketrans(const std::string& text, const std::string& src, const std::string& dst) {
    std::unordered_map<char, char> table;
    for (size_t i = 0; i < src.size() && i < dst.size(); ++i) {
        table[src[i]] = dst[i];
    }
    std::string result = text;
    for (auto& ch : result) {
        auto it = table.find(ch);
        if (it != table.end()) {
            ch = it->second;
        }
    }
    return result;
}

// Get data directory path
inline std::string data_path(const std::string& filename) {
#ifdef HAZM_DATA_DIR
    return std::string(HAZM_DATA_DIR) + filename;
#else
    return filename;
#endif
}

} // namespace hazm