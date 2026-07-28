#include "hazm/conjugation.hpp"

namespace hazm {

// ri = بن ماضی (past root), rii = بن مضارع (present root)

// Past suffixes for 6 persons
static const std::vector<std::string> PAST_SUFFIXES = {"\u0645", "\u06CC", "", "\u06CC\u0645", "\u06CC\u062F", "\u0646\u062F"};
static const std::vector<std::string> PRESENT_SUFFIXES = {"\u0645", "\u06CC", "\u062F", "\u06CC\u0645", "\u06CC\u062F", "\u0646\u062F"};

// ------ Past Perfective ------

std::vector<std::string> Conjugation::perfective_past(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& s : PAST_SUFFIXES) result.push_back(ri + s);
    return result;
}

std::vector<std::string> Conjugation::negative_perfective_past(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_past(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_perfective_past(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_past("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_perfective_past(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_perfective_past("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

// ------ Past Imperfective ------

std::vector<std::string> Conjugation::imperfective_past(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_past(ri)) result.push_back("\u0645\u06CC\u200C" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_imperfective_past(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_past(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_imperfective_past(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_past("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_imperfective_past(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_imperfective_past("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

// ------ Past Progressive ------

std::vector<std::string> Conjugation::past_progresive(const std::string& ri) const {
    auto d1 = perfective_past("\u062F\u0627\u0634\u062A"); // داشت
    auto d2 = imperfective_past(ri);
    std::vector<std::string> result;
    for (size_t i = 0; i < 6; ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

std::vector<std::string> Conjugation::passive_past_progresive(const std::string& ri) const {
    auto d1 = perfective_past("\u062F\u0627\u0634\u062A");
    auto d2 = passive_imperfective_past(ri);
    std::vector<std::string> result;
    for (size_t i = 0; i < 6; ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

// ------ Present Perfect ------

std::vector<std::string> Conjugation::present_perfect(const std::string& ri) const {
    // ri + ["ه‌ام", "ه‌ای", "ه است", "ه", "ه‌ایم", "ه‌اید", "ه‌اند"]
    return {
        ri + "\u0647\u200C\u0627\u0645",       // ه‌ام
        ri + "\u0647\u200C\u0627\u06CC",       // ه‌ای
        ri + "\u0647 \u0627\u0633\u062A",      // ه است
        ri + "\u0647",                           // ه
        ri + "\u0647\u200C\u0627\u06CC\u0645",  // ه‌ایم
        ri + "\u0647\u200C\u0627\u06CC\u062F",  // ه‌اید
        ri + "\u0647\u200C\u0627\u0646\u062F",  // ه‌اند
    };
}

std::vector<std::string> Conjugation::negative_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : present_perfect(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::subjunctive_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_present("\u0628\u0627\u0634")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_subjunctive_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_present_perfect(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::grammatical_present_perfect(const std::string& ri) const {
    auto base = subjunctive_present_perfect(ri);
    base[1] = ri + "\u0647 \u0628\u0627\u0634"; // replace باشی with باش
    return base;
}

std::vector<std::string> Conjugation::negative_grammatical_present_perfect(const std::string& ri) const {
    auto base = grammatical_present_perfect(ri);
    std::vector<std::string> result;
    for (const auto& x : base) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : present_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_present_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::passive_subjunctive_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_present_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_subjunctive_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_subjunctive_present_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::passive_grammatical_present_perfect(const std::string& ri) const {
    auto base = grammatical_present_perfect("\u0634\u062F");
    std::vector<std::string> result;
    for (const auto& x : base) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_grammatical_present_perfect(const std::string& ri) const {
    auto base = negative_grammatical_present_perfect("\u0634\u062F");
    std::vector<std::string> result;
    for (const auto& x : base) result.push_back(ri + "\u0647 " + x);
    return result;
}

// ------ Imperfective Present Perfect ------

std::vector<std::string> Conjugation::imperfective_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : present_perfect(ri)) result.push_back("\u0645\u06CC\u200C" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_imperfective_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_present_perfect(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::subjunctive_imperfective_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_present_perfect(ri)) result.push_back("\u0645\u06CC\u200C" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_subjunctive_imperfective_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_imperfective_present_perfect(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_imperfective_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_present_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_imperfective_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_imperfective_present_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::passive_subjunctive_imperfective_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_imperfective_present_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_subjunctive_imperfective_present_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_subjunctive_imperfective_present_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

// ------ Present Perfect Progressive ------

std::vector<std::string> Conjugation::present_perfect_progressive(const std::string& ri) const {
    auto d1 = present_perfect("\u062F\u0627\u0634\u062A");
    auto d2 = imperfective_present_perfect(ri);
    std::vector<std::string> result;
    for (size_t i = 0; i < d1.size(); ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

std::vector<std::string> Conjugation::passive_present_perfect_progressive(const std::string& ri) const {
    auto d1 = present_perfect("\u062F\u0627\u0634\u062A");
    auto d2 = passive_imperfective_present_perfect(ri);
    std::vector<std::string> result;
    for (size_t i = 0; i < d1.size(); ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

// ------ Past Precedent (Past Perfect) ------

std::vector<std::string> Conjugation::past_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_past("\u0628\u0648\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_past_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : past_precedent(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_past_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : past_precedent("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_past_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_past_precedent("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::imperfective_past_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : past_precedent(ri)) result.push_back("\u0645\u06CC\u200C" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_imperfective_past_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_past_precedent(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_imperfective_past_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_past_precedent("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_imperfective_past_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_imperfective_past_precedent("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::past_precedent_progressive(const std::string& ri) const {
    auto d1 = perfective_past("\u062F\u0627\u0634\u062A");
    auto d2 = imperfective_past_precedent(ri);
    std::vector<std::string> result;
    for (size_t i = 0; i < 6; ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

std::vector<std::string> Conjugation::passive_past_precedent_progressive(const std::string& ri) const {
    auto d1 = perfective_past("\u062F\u0627\u0634\u062A");
    auto d2 = passive_imperfective_past_precedent(ri);
    std::vector<std::string> result;
    for (size_t i = 0; i < 6; ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

// ------ Past Precedent Perfect ------

std::vector<std::string> Conjugation::past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : present_perfect("\u0628\u0648\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : past_precedent_perfect(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::subjunctive_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_present_perfect("\u0628\u0648\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_subjunctive_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_past_precedent_perfect(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::grammatical_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : grammatical_present_perfect("\u0628\u0648\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_grammatical_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : grammatical_past_precedent_perfect(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : past_precedent_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_past_precedent_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::passive_subjunctive_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_past_precedent_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_subjunctive_past_precedent_perfect(const std::string& ri) const {
    auto base = subjunctive_past_precedent_perfect("\u0634\u062F");
    std::vector<std::string> result;
    for (const auto& x : base) result.push_back(ri + "\u0647 \u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_grammatical_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : grammatical_past_precedent_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_grammatical_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_grammatical_past_precedent_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::imperfective_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : past_precedent_perfect(ri)) result.push_back("\u0645\u06CC\u200C" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_imperfective_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_past_precedent_perfect(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::subjunctive_imperfective_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_past_precedent_perfect(ri)) result.push_back("\u0645\u06CC\u200C" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_subjunctive_imperfective_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_imperfective_past_precedent_perfect(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_imperfective_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_past_precedent_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_imperfective_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_imperfective_past_precedent_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::passive_subjunctive_imperfective_past_precedent_perfect(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_imperfective_past_precedent_perfect("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_subjunctive_imperfective_past_precedent_perfect(const std::string& ri) const {
    auto base = subjunctive_imperfective_past_precedent_perfect("\u0634\u062F");
    std::vector<std::string> result;
    for (const auto& x : base) result.push_back(ri + "\u0647 \u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::past_precedent_perfect_progressive(const std::string& ri) const {
    auto d1 = present_perfect("\u062F\u0627\u0634\u062A");
    auto d2 = imperfective_past_precedent_perfect(ri);
    std::vector<std::string> result;
    for (size_t i = 0; i < d1.size(); ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

std::vector<std::string> Conjugation::passive_past_precedent_perfect_progressive(const std::string& ri) const {
    auto d1 = present_perfect("\u062F\u0627\u0634\u062A");
    auto d2 = passive_imperfective_past_precedent_perfect(ri);
    std::vector<std::string> result;
    for (size_t i = 0; i < d1.size(); ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

// ------ Present Perfective ------

std::vector<std::string> Conjugation::perfective_present(const std::string& rii) const {
    std::vector<std::string> result;
    for (const auto& s : PRESENT_SUFFIXES) result.push_back(rii + s);
    return result;
}

std::vector<std::string> Conjugation::negative_perfective_present(const std::string& rii) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_present(rii)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::subjunctive_perfective_present(const std::string& rii) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_present(rii)) result.push_back("\u0628" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_subjunctive_perfective_present(const std::string& rii) const {
    // Python: ["ن" + x for x in self.perfective_present(rii)]
    std::vector<std::string> result;
    for (const auto& x : perfective_present(rii)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::grammatical_perfective_present(const std::string& rii) const {
    // Python: replace "ببینی" with "ببین" (second form)
    auto base = subjunctive_perfective_present(rii);
    if (base.size() >= 2) {
        base[1] = base[1].substr(0, base[1].size() - 2); // remove final "ی"
    }
    return base;
}

std::vector<std::string> Conjugation::negative_grammatical_perfective_present(const std::string& rii) const {
    auto base = perfective_present(rii);
    // For negative grammatical: نبینم, نبین, نبیند, ...
    std::vector<std::string> result;
    for (size_t i = 0; i < base.size(); ++i) {
        if (i == 1) {
            result.push_back("\u0646" + base[i].substr(0, base[i].size() - 2));
        } else {
            result.push_back("\u0646" + base[i]);
        }
    }
    return result;
}

std::vector<std::string> Conjugation::passive_perfective_present(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_present("\u0634\u0648")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_perfective_present(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_perfective_present("\u0634\u0648")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::passive_subjunctive_perfective_present(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : subjunctive_perfective_present("\u0634\u0648")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_subjunctive_perfective_present(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_subjunctive_perfective_present("\u0634\u0648")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::passive_grammatical_perfective_present(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : grammatical_perfective_present("\u0634\u0648")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_grammatical_perfective_present(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_grammatical_perfective_present("\u0634\u0648")) result.push_back(ri + "\u0647 " + x);
    return result;
}

// ------ Present Imperfective/Progressive ------

std::vector<std::string> Conjugation::imperfective_present(const std::string& rii) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_present(rii)) result.push_back("\u0645\u06CC\u200C" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_imperfective_present(const std::string& rii) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_present(rii)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_imperfective_present(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_present("\u0634\u0648")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_imperfective_present(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_imperfective_present("\u0634\u0648")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::present_progressive(const std::string& rii) const {
    auto d1 = perfective_present("\u062F\u0627\u0631"); // دار
    auto d2 = imperfective_present(rii);
    std::vector<std::string> result;
    for (size_t i = 0; i < 6; ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

std::vector<std::string> Conjugation::passive_present_progressive(const std::string& ri) const {
    auto d1 = perfective_present("\u062F\u0627\u0631");
    auto d2 = passive_imperfective_present(ri);
    std::vector<std::string> result;
    for (size_t i = 0; i < 6; ++i) result.push_back(d1[i] + " " + d2[i]);
    return result;
}

// ------ Future ------

std::vector<std::string> Conjugation::perfective_future(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_present("\u062E\u0648\u0627\u0647")) result.push_back(x + " " + ri);
    return result;
}

std::vector<std::string> Conjugation::negative_perfective_future(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_future(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_perfective_future(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_future("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_perfective_future(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_perfective_future("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::imperfective_future(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_future(ri)) result.push_back("\u0645\u06CC\u200C" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_imperfective_future(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_future(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_imperfective_future(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : imperfective_future("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_imperfective_future(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_imperfective_future("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::future_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : perfective_future("\u0628\u0648\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_future_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : future_precedent(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_future_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : future_precedent("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_future_precedent(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_future_precedent("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::future_precedent_imperfective(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : future_precedent(ri)) result.push_back("\u0645\u06CC\u200C" + x);
    return result;
}

std::vector<std::string> Conjugation::negative_future_precedent_imperfective(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : future_precedent_imperfective(ri)) result.push_back("\u0646" + x);
    return result;
}

std::vector<std::string> Conjugation::passive_future_precedent_imperfective(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : future_precedent_imperfective("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

std::vector<std::string> Conjugation::negative_passive_future_precedent_imperfective(const std::string& ri) const {
    std::vector<std::string> result;
    for (const auto& x : negative_future_precedent_imperfective("\u0634\u062F")) result.push_back(ri + "\u0647 " + x);
    return result;
}

// ------ get_all ------

std::vector<std::string> Conjugation::get_all(const std::string& verb) const {
    auto pos = verb.find('#');
    if (pos == std::string::npos) return {};
    
    std::string ri = verb.substr(0, pos);
    std::string rii = verb.substr(pos + 1);
    
    std::vector<std::string> result;
    auto add_all = [&](const std::vector<std::string>& forms) {
        result.insert(result.end(), forms.begin(), forms.end());
    };
    
    // Infinitive
    result.push_back(ri + "\u0646");
    
    add_all(perfective_past(ri));
    add_all(negative_perfective_past(ri));
    add_all(passive_perfective_past(ri));
    add_all(negative_passive_perfective_past(ri));
    add_all(imperfective_past(ri));
    add_all(negative_imperfective_past(ri));
    add_all(passive_imperfective_past(ri));
    add_all(negative_passive_imperfective_past(ri));
    add_all(past_progresive(ri));
    add_all(passive_past_progresive(ri));
    add_all(present_perfect(ri));
    add_all(negative_present_perfect(ri));
    add_all(subjunctive_present_perfect(ri));
    add_all(negative_subjunctive_present_perfect(ri));
    add_all(grammatical_present_perfect(ri));
    add_all(negative_grammatical_present_perfect(ri));
    add_all(passive_present_perfect(ri));
    add_all(negative_passive_present_perfect(ri));
    add_all(passive_subjunctive_present_perfect(ri));
    add_all(negative_passive_subjunctive_present_perfect(ri));
    add_all(passive_grammatical_present_perfect(ri));
    add_all(negative_passive_grammatical_present_perfect(ri));
    add_all(imperfective_present_perfect(ri));
    add_all(negative_imperfective_present_perfect(ri));
    add_all(subjunctive_imperfective_present_perfect(ri));
    add_all(negative_subjunctive_imperfective_present_perfect(ri));
    add_all(passive_imperfective_present_perfect(ri));
    add_all(negative_passive_imperfective_present_perfect(ri));
    add_all(passive_subjunctive_imperfective_present_perfect(ri));
    add_all(negative_passive_subjunctive_imperfective_present_perfect(ri));
    add_all(present_perfect_progressive(ri));
    add_all(passive_present_perfect_progressive(ri));
    add_all(past_precedent(ri));
    add_all(negative_past_precedent(ri));
    add_all(passive_past_precedent(ri));
    add_all(negative_passive_past_precedent(ri));
    add_all(imperfective_past_precedent(ri));
    add_all(negative_imperfective_past_precedent(ri));
    add_all(passive_imperfective_past_precedent(ri));
    add_all(negative_passive_imperfective_past_precedent(ri));
    add_all(past_precedent_progressive(ri));
    add_all(passive_past_precedent_progressive(ri));
    add_all(past_precedent_perfect(ri));
    add_all(negative_past_precedent_perfect(ri));
    add_all(subjunctive_past_precedent_perfect(ri));
    add_all(negative_subjunctive_past_precedent_perfect(ri));
    add_all(grammatical_past_precedent_perfect(ri));
    add_all(negative_grammatical_past_precedent_perfect(ri));
    add_all(passive_past_precedent_perfect(ri));
    add_all(negative_passive_past_precedent_perfect(ri));
    add_all(passive_subjunctive_past_precedent_perfect(ri));
    add_all(negative_passive_subjunctive_past_precedent_perfect(ri));
    add_all(passive_grammatical_past_precedent_perfect(ri));
    add_all(negative_passive_grammatical_past_precedent_perfect(ri));
    add_all(imperfective_past_precedent_perfect(ri));
    add_all(negative_imperfective_past_precedent_perfect(ri));
    add_all(subjunctive_imperfective_past_precedent_perfect(ri));
    add_all(negative_subjunctive_imperfective_past_precedent_perfect(ri));
    add_all(passive_imperfective_past_precedent_perfect(ri));
    add_all(negative_passive_imperfective_past_precedent_perfect(ri));
    add_all(passive_subjunctive_imperfective_past_precedent_perfect(ri));
    add_all(negative_passive_subjunctive_imperfective_past_precedent_perfect(ri));
    add_all(past_precedent_perfect_progressive(ri));
    add_all(passive_past_precedent_perfect_progressive(ri));
    add_all(perfective_present(rii));
    add_all(negative_perfective_present(rii));
    add_all(subjunctive_perfective_present(rii));
    add_all(negative_subjunctive_perfective_present(rii));
    add_all(grammatical_perfective_present(rii));
    add_all(negative_grammatical_perfective_present(rii));
    add_all(passive_perfective_present(ri));
    add_all(negative_passive_perfective_present(ri));
    add_all(passive_subjunctive_perfective_present(ri));
    add_all(negative_passive_subjunctive_perfective_present(ri));
    add_all(passive_grammatical_perfective_present(ri));
    add_all(negative_passive_grammatical_perfective_present(ri));
    add_all(imperfective_present(rii));
    add_all(negative_imperfective_present(rii));
    add_all(passive_imperfective_present(ri));
    add_all(negative_passive_imperfective_present(ri));
    add_all(present_progressive(rii));
    add_all(passive_present_progressive(ri));
    add_all(perfective_future(ri));
    add_all(negative_perfective_future(ri));
    add_all(passive_perfective_future(ri));
    add_all(negative_passive_perfective_future(ri));
    add_all(imperfective_future(ri));
    add_all(negative_imperfective_future(ri));
    add_all(passive_imperfective_future(ri));
    add_all(negative_passive_imperfective_future(ri));
    add_all(future_precedent(ri));
    add_all(negative_future_precedent(ri));
    add_all(passive_future_precedent(ri));
    add_all(negative_passive_future_precedent(ri));
    add_all(future_precedent_imperfective(ri));
    add_all(negative_future_precedent_imperfective(ri));
    add_all(passive_future_precedent_imperfective(ri));
    add_all(negative_passive_future_precedent_imperfective(ri));
    
    return result;
}

} // namespace hazm