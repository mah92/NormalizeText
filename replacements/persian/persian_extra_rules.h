// persian_extra_rules.h — Persian number FORMATS that the language-neutral stage would destroy.
//
// Ali (2026-10-09): «۱۲:۳۰» came out as «دوازده سی» (the colon was flattened to a space),
// «۱۴۰۰/۰۵/۱۲» as «هزار و چهارصد پنج دوازده», «۵م» as «پنج م» (the letter mim!) and «۱/۲» as
// «یک دو». His chosen readings: dates keep a PAUSE between parts («۱۴۰۰، ۵، ۱۲»), the clock reads
// «دوازده و سی دقیقه», ordinals become the ordinal word (both «۵م» and «۵ام»), fractions read
// «یک دوم».
//
// Two rules of the road:
//  * std::regex cannot take multi-byte ranges in brackets («[۰-۹]» throws "Invalid range in bracket
//    expression"), so every class here is ASCII-only and the Persian-letter checks happen in code.
//  * Dates only get their separators swapped for «،» and keep the digits, so the language-aware
//    number converter downstream still reads them in the right language (a Latin date inside a
//    Persian document keeps its English numbers). Only the clock and the fraction, whose reading
//    NEEDS Persian words, check that the match sits next to Persian text.
//
// MUST run before seperateClock()/seperateDate() in performGeneralReplacements(), and only for
// Persian main language — English text keeps its previous behaviour.
#ifndef PERSIAN_EXTRA_RULES_H
#define PERSIAN_EXTRA_RULES_H

#include <algorithm>
#include <functional>
#include <regex>
#include <string>

#include "normalize_numbers_persian.h"

namespace persian_extra {

inline std::string cardinal(const std::string& digits) {
    return PersianNumberConverter::number_to_words(digits);
}

// 1..3 are irregular in Persian; from 4 up the ordinal is the cardinal + «م».
inline std::string ordinal(const std::string& digits) {
    try {
        long n = std::stol(digits);
        if (n == 1) return "اول";
        if (n == 2) return "دوم";
        if (n == 3) return "سوم";
    } catch (...) {
        // fall through to the general rule
    }
    return cardinal(digits) + "م";
}

// True when the byte at `pos` starts a multi-byte character (a Persian letter in our cases).
inline bool startsNonAscii(const std::string& s, size_t pos) {
    return pos < s.size() && static_cast<unsigned char>(s[pos]) >= 0x80;
}

// True when Persian text sits within `window` bytes on either side of `pos` — used to decide whether
// a clock/fraction match should be verbalised in Persian.
inline bool nearPersianText(const std::string& s, size_t pos, size_t window = 24) {
    const size_t from = pos > window ? pos - window : 0;
    const size_t to = std::min(s.size(), pos + window);
    for (size_t i = from; i < to; ++i) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c >= 0x80) {                 // any multi-byte char counts; Persian words dominate here
            return true;
        }
    }
    return false;
}

inline std::string replaceAll(const std::string& text, const std::regex& re,
                              const std::function<std::string(const std::smatch&)>& build,
                              const std::function<bool(const std::smatch&, size_t)>& keep) {
    std::string out;
    size_t last = 0;
    for (auto it = std::sregex_iterator(text.begin(), text.end(), re); it != std::sregex_iterator(); ++it) {
        const std::smatch& m = *it;
        const size_t after = m.position() + m.length();
        if (!keep(m, after)) continue;
        out += text.substr(last, m.position() - last);
        out += build(m);
        last = after;
    }
    if (last == 0) return text;          // nothing replaced (or a match at position 0 was rejected)
    out += text.substr(last);
    return out;
}

inline void apply(std::string& text) {
    // 1) dates: «1400/05/12» and «1400-05-12» -> «1400، 05، 12» (a PAUSE, Ali's choice). The digits
    //    are kept: the language-aware converter downstream reads them.
    {
        std::regex re(R"((^|[^0-9])([0-9]{1,4})([/\-.=])([0-9]{1,2})(\3)([0-9]{1,4})(?![0-9]))");
        text = replaceAll(text, re,
                          [](const std::smatch& m) {
                              return m.str(1) + m.str(2) + "، " + m.str(4) + "، " + m.str(6);
                          },
                          // only next to Persian text: a Latin date inside a Persian document must
                          // keep its English number reading (the «،» would flip the segment).
                          [&text](const std::smatch& m, size_t) {
                              return nearPersianText(text, m.position());
                          });
    }
    // 2) ordinals: «5م» / «5ام» -> «پنجم». Rejected when a non-ASCII char follows, so «12ماه»
    //    (months) is untouched, and «۵ م» (a standalone mim) stays a letter.
    {
        std::regex re(R"((^|[^0-9])([0-9]{1,4})\s*(اُم|ام|م)(?![A-Za-z0-9]))");
        text = replaceAll(text, re,
                          [](const std::smatch& m) { return m.str(1) + ordinal(m.str(2)); },
                          [&text](const std::smatch&, size_t after) { return !startsNonAscii(text, after); });
    }

    // 3) clock: «12:30» -> «دوازده و سی دقیقه» (only next to Persian text).
    {
        std::regex re(R"((^|[^0-9])([0-9]{1,2}):([0-9]{2})(?![0-9]))");
        text = replaceAll(text, re,
                          [](const std::smatch& m) {
                              return m.str(1) + cardinal(m.str(2)) + " و " + cardinal(m.str(3)) + " دقیقه";
                          },
                          [&text](const std::smatch& m, size_t) {
                              return nearPersianText(text, m.position());
                          });
    }

    // 4) fractions: «1/2» -> «یک دوم» for denominators 2..10 (only next to Persian text; anything
    //    else keeps the neutral flattening, which reads as two numbers).
    {
        std::regex re(R"((^|[^0-9])([0-9]{1,2})/([0-9]{1,2})(?![0-9/]))");
        text = replaceAll(text, re,
                          [](const std::smatch& m) {
                              long den = 0;
                              try { den = std::stol(m.str(3)); } catch (...) { den = 0; }
                              if (den >= 2 && den <= 10) {
                                  return m.str(1) + cardinal(m.str(2)) + " " + ordinal(m.str(3));
                              }
                              return m.str(1) + m.str(2) + " " + m.str(3);
                          },
                          [&text](const std::smatch& m, size_t) {
                              return nearPersianText(text, m.position());
                          });
    }

}

}  // namespace persian_extra

#endif  // PERSIAN_EXTRA_RULES_H
