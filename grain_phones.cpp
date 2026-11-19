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

// ---------------------------------------------------------------------------
// English diphthong merging — part of the front-end itself (Ali, 2026-10-08): espeak splits some
// English diphthongs into two symbols ("dˈeɪɾə" -> "ˈe" + "ɪ"), while the English training labels
// (the author's ARPAbet->IPA alignments) carry them as ONE token ("ˈeɪ"). Only the measured pairs
// are merged, and only English segments are affected: Persian has no diphthong inventory in the
// model's alphabet, so a bare merge there could mint a token the alphabet does not carry.
// ---------------------------------------------------------------------------
void mergeEnglishDiphthongs(std::vector<std::string>& tokens) {
    static const struct { const char* first; const char* second; const char* joined; } kPairs[] = {
        {"a", "\xC9\xAA", "a\xC9\xAA"},     // a + ɪ -> aɪ
        {"e", "\xC9\xAA", "e\xC9\xAA"},     // e + ɪ -> eɪ
        {"o", "\xCA\x8A", "o\xCA\x8A"},     // o + ʊ -> oʊ
        {"a", "\xCA\x8A", "a\xCA\x8A"},     // a + ʊ -> aʊ
        {"\xC9\x94", "\xC9\xAA", "\xC9\x94\xC9\xAA"},   // ɔ + ɪ -> ɔɪ
        {"\xCA\x8C", "\xC9\xAA", "a\xC9\xAA"},          // ʌ + ɪ -> aɪ
    };
    auto split_stress = [](const std::string& tok, std::string& stress, std::string& rest) {
        stress.clear();
        rest = tok;
        for (;;) {
            if (rest.size() >= 2 && static_cast<unsigned char>(rest[0]) == 0xCB
                && (static_cast<unsigned char>(rest[1]) == 0x88
                    || static_cast<unsigned char>(rest[1]) == 0x8C)) {
                stress += rest.substr(0, 2);
                rest.erase(0, 2);
            } else {
                break;
            }
        }
    };
    std::vector<std::string> out;
    for (size_t i = 0; i < tokens.size(); ++i) {
        bool merged = false;
        if (i + 1 < tokens.size()) {
            std::string s1, c1, s2, c2;
            split_stress(tokens[i], s1, c1);
            split_stress(tokens[i + 1], s2, c2);
            if (s2.empty() || s2 == s1) {
                for (const auto& pair : kPairs) {
                    if (c1 == pair.first && c2 == pair.second) {
                        out.push_back(s1 + pair.joined);   // stress stays on the merged token
                        ++i;                               // consume the second half
                        merged = true;
                        break;
                    }
                }
            }
        }
        if (!merged) out.push_back(tokens[i]);
    }
    tokens.swap(out);
}

// ---------------------------------------------------------------------------
std::vector<std::string> mapPhonesToInventory(
        const std::vector<std::string>& in,
        const std::set<std::string>& knownTokens) {
    // NOTE: symbol-table keys carry the leading '@' (phones_to_ids adds it), so
    // every lookup here must add it too — forgetting this made the whole
    // normalization a silent no-op on the first attempt.
    auto has = [&](const std::string& k) {
        std::string key = (!k.empty() && k[0] == '@') ? k : ("@" + k);
        return knownTokens.find(key) != knownTokens.end();
    };
    auto split_prefix = [](const std::string& p, std::string& prefix, std::string& core) {
        size_t c = p.find(':');
        if (c == std::string::npos) { prefix.clear(); core = p; }
        else { prefix = p.substr(0, c + 1); core = p.substr(c + 1); }
    };
    // strip leading primary ˈ (U+02C8 = CB 88) / secondary ˌ (U+02CC = CB 8C) marks
    auto split_stress = [](const std::string& core, std::string& stress, std::string& rest) {
        stress.clear();
        rest = core;
        for (;;) {
            if (rest.size() >= 2
                && static_cast<unsigned char>(rest[0]) == 0xCB
                && (static_cast<unsigned char>(rest[1]) == 0x88
                    || static_cast<unsigned char>(rest[1]) == 0x8C)) {
                stress += rest.substr(0, 2);
                rest.erase(0, 2);
            } else {
                break;
            }
        }
    };

    const std::string P_SCHWA = "\xC9\x99";         // ə
    const std::string P_LEN = "\xCB\x90";           // ː
    const std::string S_PRIMARY = "\xCB\x88";       // ˈ
    const std::string S_SECONDARY = "\xCB\x8C";     // ˌ

    std::vector<std::string> out;
    for (size_t i = 0; i < in.size(); ++i) {
        std::string prefix, core;
        split_prefix(in[i], prefix, core);
        std::string stress, rest;
        split_stress(core, stress, rest);

        // 1) already a symbol the model knows
        if (has(in[i])) { out.push_back(in[i]); continue; }

        // 2) espeak's split English diphthongs are already merged in the core path
        //    (mergeEnglishDiphthongs in toModelPhones), so nothing to do here — the merge belongs to
        //    the front-end, not to this alphabet-fitting layer.
        // 3) near-miss substitutions (stress-preserving form first)
        static const char* kSubs[][2] = {
            {"\xE1\xB5\xBB", "\xC9\xAA"},                    // ᵻ  -> ɪ
            {"\xC9\xA8",     "\xC9\xAA"},                    // ɨ  -> ɪ
            {"\xC9\xBE",     "t"},                            // ɾ  -> t  (the trained English labels write the
                                                              //          American flap as t: data/better/water)
            {"r",            "\xC9\xB9"},                    // r  -> ɹ
            {"i",            "\xC9\xAA"},                    // i  -> ɪ  (espeak's bare i; not in the alphabet)
            {"o\xCB\x90",     "o\xCA\x8A"},                    // oː -> oʊ  (espeak's long o; not in the alphabet)
            {"\xC9\x90",     "\xC9\x99"},                    // ɐ  -> ə
            {"\xC9\x92",     "\xC9\x94"},                    // ɒ  -> ɔ
            {"\xCA\x8C",     "\xC9\x99"},                    // ʌ  -> ə
            {"\xC9\x9C",     "\xC9\x9C\xCB\x90"},            // ɜ  -> ɜː
            {"\xC9\x9A",     "\xC9\x99"},                    // ɚ  -> ə
            {"\xC9\x9D",     "\xC9\x9C\xCB\x90"},            // ɝ  -> ɜː
        };
        if (rest.size() <= 3) {
            bool done = false;
            for (const auto& sub : kSubs) {
                if (rest == sub[0]) {
                    std::string cand = prefix + stress + sub[1];
                    if (has(cand)) { out.push_back(cand); done = true; break; }
                    std::string cand2 = prefix + sub[1];
                    if (has(cand2)) { out.push_back(cand2); done = true; break; }
                }
            }
            if (done) continue;
        }

        // 4) drop a trailing length mark (ɑː -> ɑ where only the short form exists)
        if (rest.size() > 2 && rest.compare(rest.size() - 2, 2, P_LEN) == 0) {
            std::string stripped = rest.substr(0, rest.size() - 2);
            std::string cand = prefix + stress + stripped;
            if (has(cand)) { out.push_back(cand); continue; }
            std::string cand2 = prefix + stripped;
            if (has(cand2)) { out.push_back(cand2); continue; }
            std::string cand3 = prefix + stripped + P_LEN;
            if (has(cand3)) { out.push_back(cand3); continue; }
        }

        // 5) a secondary-stressed vowel that only exists unstressed (ˌo -> o)
        if (stress == S_SECONDARY) {
            std::string bare = prefix + rest;
            if (has(bare)) { out.push_back(bare); continue; }
        }

        // 6) split a vowel+schwa sequence (ɪə -> ɪ + ə) when both halves exist
        if (rest.size() > 2 && rest.compare(rest.size() - 2, 2, P_SCHWA) == 0) {
            std::string head = rest.substr(0, rest.size() - 2);
            if (has(prefix + head) && has(prefix + P_SCHWA)) {
                out.push_back(prefix + stress + head);
                out.push_back(prefix + P_SCHWA);
                continue;
            }
        }

        // 7) STRIP_DIACRITICS: drop phonetic diacritics (palatalisation ʲ,
        //    aspiration ʰ, nasalisation ̃, syllabic ̩ …) and retry — espeak emits
        //    them for foreign words/URLs and the model's alphabet has none of them.
        {
            std::string stripped;
            for (size_t k = 0; k < rest.size();) {
                size_t len = 1;
                unsigned char c0 = static_cast<unsigned char>(rest[k]);
                if ((c0 & 0xE0) == 0xC0) len = 2;
                else if ((c0 & 0xF0) == 0xE0) len = 3;
                else if ((c0 & 0xF8) == 0xF0) len = 4;
                if (k + len > rest.size()) len = 1;
                std::string cp = rest.substr(k, len);
                k += len;
                static const char* kDiacritics[] = {
                    "\xCA\xB0",  // ʰ
                    "\xCA\xB2",  // ʲ
                    "\xCA\xB7",  // ʷ
                    "\xCB\xA4",  // ˤ
                    "\xCB\xA0",  // ˠ
                    "\xCB\x80",  // ˀ
                    "\xCC\xA9",  // ̩
                    "\xCC\xAF",  // ̯
                    "\xCC\x83",  // ̃
                    "\xCC\xA5",  // ̥
                    "\xCA\xB1",  // ʱ
                    "\xCA\xA1"   // ˡ
                };
                bool drop = false;
                for (const char* d : kDiacritics) {
                    if (cp == d) { drop = true; break; }
                }
                if (!drop) stripped += cp;
            }
            if (!stripped.empty() && stripped != rest) {
                std::string cand = prefix + stress + stripped;
                if (has(cand)) { out.push_back(cand); continue; }
                std::string cand2 = prefix + stripped;
                if (has(cand2)) { out.push_back(cand2); continue; }
            }
        }

        // 8) still unknown — keep it; phones_to_ids reports and skips it
        out.push_back(in[i]);
        (void)S_PRIMARY;
    }
    return out;
}

std::string toModelPhones(const std::string& normalized, const std::string& ipa,
                          const std::string& langTag) {
    // The IPA stream carries the content; `normalized` is kept in the signature because the
    // callers already have it (and for symmetry with phonemize_ipa.py's two outputs).
    (void)normalized;

    std::vector<std::string> out;
    const bool english = (langTag == "en" || langTag == "en-us" || langTag == "EN");
    for (const std::string& group : splitWhitespace(ipa)) {
        const std::string t = trimPunct(group);
        if (t.empty() || isJunkOnly(t)) continue;
        std::vector<std::string> toks = ipaGroupToPhonemes(t);
        if (english) mergeEnglishDiphthongs(toks);   // front-end rule, independent of any alphabet
        for (const std::string& p : toks)
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
