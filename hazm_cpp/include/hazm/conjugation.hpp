#pragma once

#include <string>
#include <vector>

namespace hazm {

class Conjugation {
public:
    // Past tense (perfective)
    std::vector<std::string> perfective_past(const std::string& ri) const;
    std::vector<std::string> negative_perfective_past(const std::string& ri) const;
    std::vector<std::string> passive_perfective_past(const std::string& ri) const;
    std::vector<std::string> negative_passive_perfective_past(const std::string& ri) const;
    
    // Past tense (imperfective)
    std::vector<std::string> imperfective_past(const std::string& ri) const;
    std::vector<std::string> negative_imperfective_past(const std::string& ri) const;
    std::vector<std::string> passive_imperfective_past(const std::string& ri) const;
    std::vector<std::string> negative_passive_imperfective_past(const std::string& ri) const;
    
    // Past progressive
    std::vector<std::string> past_progresive(const std::string& ri) const;
    std::vector<std::string> passive_past_progresive(const std::string& ri) const;
    
    // Present perfect
    std::vector<std::string> present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_present_perfect(const std::string& ri) const;
    std::vector<std::string> subjunctive_present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_subjunctive_present_perfect(const std::string& ri) const;
    std::vector<std::string> grammatical_present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_grammatical_present_perfect(const std::string& ri) const;
    std::vector<std::string> passive_present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_present_perfect(const std::string& ri) const;
    std::vector<std::string> passive_subjunctive_present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_subjunctive_present_perfect(const std::string& ri) const;
    std::vector<std::string> passive_grammatical_present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_grammatical_present_perfect(const std::string& ri) const;
    
    // Imperfective present perfect
    std::vector<std::string> imperfective_present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_imperfective_present_perfect(const std::string& ri) const;
    std::vector<std::string> subjunctive_imperfective_present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_subjunctive_imperfective_present_perfect(const std::string& ri) const;
    std::vector<std::string> passive_imperfective_present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_imperfective_present_perfect(const std::string& ri) const;
    std::vector<std::string> passive_subjunctive_imperfective_present_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_subjunctive_imperfective_present_perfect(const std::string& ri) const;
    
    // Present perfect progressive
    std::vector<std::string> present_perfect_progressive(const std::string& ri) const;
    std::vector<std::string> passive_present_perfect_progressive(const std::string& ri) const;
    
    // Past perfect (past precedent)
    std::vector<std::string> past_precedent(const std::string& ri) const;
    std::vector<std::string> negative_past_precedent(const std::string& ri) const;
    std::vector<std::string> passive_past_precedent(const std::string& ri) const;
    std::vector<std::string> negative_passive_past_precedent(const std::string& ri) const;
    std::vector<std::string> imperfective_past_precedent(const std::string& ri) const;
    std::vector<std::string> negative_imperfective_past_precedent(const std::string& ri) const;
    std::vector<std::string> passive_imperfective_past_precedent(const std::string& ri) const;
    std::vector<std::string> negative_passive_imperfective_past_precedent(const std::string& ri) const;
    std::vector<std::string> past_precedent_progressive(const std::string& ri) const;
    std::vector<std::string> passive_past_precedent_progressive(const std::string& ri) const;
    
    // Past perfect perfective
    std::vector<std::string> past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> subjunctive_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_subjunctive_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> grammatical_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_grammatical_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> passive_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> passive_subjunctive_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_subjunctive_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> passive_grammatical_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_grammatical_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> imperfective_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_imperfective_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> subjunctive_imperfective_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_subjunctive_imperfective_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> passive_imperfective_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_imperfective_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> passive_subjunctive_imperfective_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> negative_passive_subjunctive_imperfective_past_precedent_perfect(const std::string& ri) const;
    std::vector<std::string> past_precedent_perfect_progressive(const std::string& ri) const;
    std::vector<std::string> passive_past_precedent_perfect_progressive(const std::string& ri) const;
    
    // Present tense (perfective)
    std::vector<std::string> perfective_present(const std::string& rii) const;
    std::vector<std::string> negative_perfective_present(const std::string& rii) const;
    std::vector<std::string> subjunctive_perfective_present(const std::string& rii) const;
    std::vector<std::string> negative_subjunctive_perfective_present(const std::string& rii) const;
    std::vector<std::string> grammatical_perfective_present(const std::string& rii) const;
    std::vector<std::string> negative_grammatical_perfective_present(const std::string& rii) const;
    std::vector<std::string> passive_perfective_present(const std::string& ri) const;
    std::vector<std::string> negative_passive_perfective_present(const std::string& ri) const;
    std::vector<std::string> passive_subjunctive_perfective_present(const std::string& ri) const;
    std::vector<std::string> negative_passive_subjunctive_perfective_present(const std::string& ri) const;
    std::vector<std::string> passive_grammatical_perfective_present(const std::string& ri) const;
    std::vector<std::string> negative_passive_grammatical_perfective_present(const std::string& ri) const;
    
    // Present progressive (imperfective present)
    std::vector<std::string> imperfective_present(const std::string& rii) const;
    std::vector<std::string> negative_imperfective_present(const std::string& rii) const;
    std::vector<std::string> passive_imperfective_present(const std::string& ri) const;
    std::vector<std::string> negative_passive_imperfective_present(const std::string& ri) const;
    std::vector<std::string> present_progressive(const std::string& rii) const;
    std::vector<std::string> passive_present_progressive(const std::string& ri) const;
    
    // Future
    std::vector<std::string> perfective_future(const std::string& ri) const;
    std::vector<std::string> negative_perfective_future(const std::string& ri) const;
    std::vector<std::string> passive_perfective_future(const std::string& ri) const;
    std::vector<std::string> negative_passive_perfective_future(const std::string& ri) const;
    std::vector<std::string> imperfective_future(const std::string& ri) const;
    std::vector<std::string> negative_imperfective_future(const std::string& ri) const;
    std::vector<std::string> passive_imperfective_future(const std::string& ri) const;
    std::vector<std::string> negative_passive_imperfective_future(const std::string& ri) const;
    std::vector<std::string> future_precedent(const std::string& ri) const;
    std::vector<std::string> negative_future_precedent(const std::string& ri) const;
    std::vector<std::string> passive_future_precedent(const std::string& ri) const;
    std::vector<std::string> negative_passive_future_precedent(const std::string& ri) const;
    std::vector<std::string> future_precedent_imperfective(const std::string& ri) const;
    std::vector<std::string> negative_future_precedent_imperfective(const std::string& ri) const;
    std::vector<std::string> passive_future_precedent_imperfective(const std::string& ri) const;
    std::vector<std::string> negative_passive_future_precedent_imperfective(const std::string& ri) const;
    
    // Generate all forms
    std::vector<std::string> get_all(const std::string& verb) const;
};

} // namespace hazm