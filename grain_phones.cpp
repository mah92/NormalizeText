#include "grain_phones.h"

#include <cctype>
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

// True when a code point is punctuation that must be trimmed off an IPA group.
//
// NOTE (2026-10-03): the first version of this helper returned false for every multi-byte
// code point (`if (cp.size() > 1) return false;`), so Persian punctuation (، ؛ ؟ ٪ ۔) and
// emoji were never trimmed. In the lexicon era that alone turned ordinary words such as
// «مقاومت،» into «unknown words». ZWNJ (U+200C) is deliberately kept — it is meaningful
// in Persian.
bool isPunctCp(const std::string& cp) {
    if (cp.size() == 1) {
        unsigned char c = static_cast<unsigned char>(cp[0]);
        return !std::isalnum(c) && cp != "_";
    }
    unsigned int v = 0;
    unsigned char c0 = static_cast<unsigned char>(cp[0]);
    if ((c0 & 0xE0) == 0xC0 && cp.size() >= 2) {
        v = ((c0 & 0x1Fu) << 6) | (static_cast<unsigned char>(cp[1]) & 0x3Fu);
    } else if ((c0 & 0xF0) == 0xE0 && cp.size() >= 3) {
        v = ((c0 & 0x0Fu) << 12) | ((static_cast<unsigned char>(cp[1]) & 0x3Fu) << 6) |
            (static_cast<unsigned char>(cp[2]) & 0x3Fu);
    } else if ((c0 & 0xF8) == 0xF0 && cp.size() >= 4) {
        v = ((c0 & 0x07u) << 18) | ((static_cast<unsigned char>(cp[1]) & 0x3Fu) << 12) |
            ((static_cast<unsigned char>(cp[2]) & 0x3Fu) << 6) |
            (static_cast<unsigned char>(cp[3]) & 0x3Fu);
    } else {
        return false;
    }
    if (v == 0x200C) return false;                       // ZWNJ stays
    if (v >= 0x2190) return true;                        // arrows, symbols, emoji
    if (v == 0x060C || v == 0x061B || v == 0x061F) return true;   // ، ؛ ؟
    if (v >= 0x066A && v <= 0x066D) return true;         // ٪ ٫ ٬ ٭
    if (v == 0x06D4) return true;                        // ۔
    if (v == 0x00AB || v == 0x00BB || v == 0x00A0) return true;
    if (v >= 0x2000 && v <= 0x206F) return true;         // general punctuation
    if (v >= 0x2E00 && v <= 0x2E7F) return true;
    if (v >= 0xFE50 && v <= 0xFE6F) return true;
    if (v >= 0xFF01 && v <= 0xFF65) return true;         // fullwidth forms
    return false;
}

std::string trimPunct(const std::string& w) {
    std::vector<std::string> cps = codePoints(w);
    size_t a = 0, b = cps.size();
    while (a < b && isPunctCp(cps[a])) ++a;
    while (b > a && isPunctCp(cps[b - 1])) --b;
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
                          const std::string& langTag) {
    // The IPA stream carries the content; `normalized` is kept in the signature because the
    // callers already have it (and for symmetry with phonemize_ipa.py's two outputs).
    (void)normalized;

    std::vector<std::string> out;
    for (const std::string& group : splitWhitespace(ipa)) {
        const std::string t = trimPunct(group);
        if (t.empty() || isJunkOnly(t)) continue;
        for (const std::string& p : ipaGroupToPhonemes(t))
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
