#include "grain_phones.h"

#include <cctype>
#include <fstream>
#include <set>
#include <sstream>

namespace grain {
namespace {

// Same rule sets as phonemize_ipa.py — keep in sync.
const std::set<std::string> kStress = {"ˈ", "ˌ"};
const std::set<std::string> kLength = {"ː", "ˑ"};
const std::set<std::string> kTie = {"͡", "͜"};
const std::set<std::string> kDiacritics = {
    "ʰ", "ʲ", "ʷ", "ˤ", "ˠ", "ˀ", "ⁿ", "̩", "̯", "̃", "̥", "̊", "̪", "̺", "̻", "̼",
    "̽", "̾", "̿", "ͅ", "ꭜ", "ʱ", "ˡ"};

bool inSet(const std::set<std::string>& s, const std::string& cp) {
    return s.find(cp) != s.end();
}

// Decode UTF-8 into one string per code point.
std::vector<std::string> codePoints(const std::string& s) {
    std::vector<std::string> out;
    for (size_t i = 0; i < s.size();) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t len = 1;
        if ((c & 0x80) == 0x00) len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        if (i + len > s.size()) len = 1;
        out.push_back(s.substr(i, len));
        i += len;
    }
    return out;
}

bool isSpace(const std::string& cp) {
    return cp == " " || cp == "\t" || cp == "\n" || cp == "\r";
}

// Characters that are never phones: digits and punctuation.
bool isJunkOnly(const std::string& tok) {
    if (tok.empty()) return true;
    for (const std::string& cp : codePoints(tok)) {
        if (inSet(kStress, cp) || inSet(kLength, cp) || inSet(kTie, cp) ||
            inSet(kDiacritics, cp)) {
            continue;
        }
        if (cp.size() == 1 && std::isdigit(static_cast<unsigned char>(cp[0]))) continue;
        return false;   // any letter (ASCII or non-ASCII) counts as phone content
    }
    return true;
}

std::string trimPunct(const std::string& w) {
    std::vector<std::string> cps = codePoints(w);
    size_t a = 0, b = cps.size();
    auto punct = [](const std::string& cp) {
        if (cp.size() > 1) return false;
        unsigned char c = static_cast<unsigned char>(cp[0]);
        return !std::isalnum(c) && cp != "_";
    };
    while (a < b && punct(cps[a])) ++a;
    while (b > a && punct(cps[b - 1])) --b;
    std::string out;
    for (size_t i = a; i < b; ++i) out += cps[i];
    return out;
}

std::vector<std::string> splitWhitespace(const std::string& s) {
    std::vector<std::string> out;
    std::istringstream ss(s);
    std::string tok;
    while (ss >> tok) out.push_back(tok);
    return out;
}

}  // namespace

std::map<std::string, std::string> loadInferenceLexicon(const std::string& dir,
                                                        const std::string& lang) {
    std::map<std::string, std::string> lex;
    std::string path = dir + "/infer_lex_" + lang + ".txt";
    std::ifstream in(path);
    if (!in.is_open()) return lex;
    std::string line;
    while (std::getline(in, line)) {
        size_t tab = line.find('\t');
        if (tab == std::string::npos) continue;
        std::string word = line.substr(0, tab);
        std::string phones = line.substr(tab + 1);
        if (!word.empty() && lex.find(word) == lex.end()) lex[word] = phones;
    }
    return lex;
}

std::vector<std::string> ipaGroupToPhonemes(const std::string& group) {
    std::vector<std::string> cps = codePoints(group);
    std::vector<std::string> tokens;
    size_t i = 0;
    while (i < cps.size()) {
        const std::string& ch = cps[i];
        if (isSpace(ch)) {
            ++i;
            continue;
        }
        if (inSet(kStress, ch)) {
            // stress attaches to the following vowel
            std::string tok = ch;
            ++i;
            while (i < cps.size() && inSet(kStress, cps[i])) tok += cps[i++];
            if (i < cps.size()) {
                tok += cps[i++];
                while (i < cps.size() && (inSet(kLength, cps[i]) || inSet(kDiacritics, cps[i])))
                    tok += cps[i++];
            }
            tokens.push_back(tok);
            continue;
        }
        if (inSet(kLength, ch) || inSet(kDiacritics, ch)) {
            if (!tokens.empty()) tokens.back() += ch;
            else tokens.push_back(ch);
            ++i;
            continue;
        }
        if (inSet(kTie, ch)) {
            ++i;
            if (i < cps.size()) {
                if (!tokens.empty()) tokens.back() += cps[i];
                else tokens.push_back(cps[i]);
                ++i;
            }
            continue;
        }
        std::string tok = ch;
        ++i;
        if (i < cps.size() && inSet(kTie, cps[i])) {
            ++i;
            if (i < cps.size()) tok += cps[i++];
        }
        while (i < cps.size() && (inSet(kLength, cps[i]) || inSet(kDiacritics, cps[i])))
            tok += cps[i++];
        tokens.push_back(tok);
    }
    std::vector<std::string> out;
    for (const std::string& t : tokens)
        if (!isJunkOnly(t)) out.push_back(t);
    return out;
}

std::string toModelPhones(const std::string& normalized, const std::string& ipa,
                          const std::map<std::string, std::string>& lexicon,
                          const std::string& langTag) {
    std::vector<std::string> words, groups;
    for (const std::string& w : splitWhitespace(normalized)) {
        std::string t = trimPunct(w);
        if (!t.empty() && !isJunkOnly(t)) words.push_back(t);
    }
    for (const std::string& g : splitWhitespace(ipa)) {
        std::string t = trimPunct(g);
        if (!t.empty()) groups.push_back(t);
    }

    const bool aligned = (words.size() == groups.size());
    std::vector<std::string> out;
    for (size_t i = 0; i < words.size(); ++i) {
        auto it = lexicon.find(words[i]);
        if (it != lexicon.end() && !it->second.empty()) {
            std::istringstream ss(it->second);
            std::string tok;
            while (ss >> tok) out.push_back(tok);
            continue;
        }
        if (!aligned) continue;                     // unknown word, no group to fall back on
        for (const std::string& p : ipaGroupToPhonemes(groups[i]))
            out.push_back(langTag + ":" + p);
    }

    std::string joined;
    for (size_t i = 0; i < out.size(); ++i) {
        if (i) joined += " ";
        joined += out[i];
    }
    return joined;
}

}  // namespace grain
