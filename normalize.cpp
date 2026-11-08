#include "./normalize.h"
#include "language_detector/language_detector.h"
#include "replacements/replacements.h"
#include "espeak_phonemize/phonemize.hpp"
#include "shakkelha/shakkelha.h"
#include "nawar-halabi/ar_phonemizer.h"
#include "persian_phoneme/persian_postprocess.h"

#include <iostream>
#include <chrono>
#include <cstdio>
#include <map>
#include <cctype>
#include <cstdlib>

static bool persianResourcesInitialized = false;

// Canonical pronunciation of a single Persian letter.  espeak's fa dictionary
// gets a few of these wrong (پ -> "pˈee", گ -> "ɡɡˈɑf", غ/ق -> "q1..."), so a
// lone letter is answered from this table instead.  Only applies when the whole
// segment is exactly one letter, so normal text is untouched.
static const std::map<std::string, std::string>& faLetterNames() {
    static const std::map<std::string, std::string> m = {
        {"ا", "ˈalef"},        {"ب", "bˈɑʔ"},        {"پ", "pˈe"},
        {"ت", "tˈe jˈe dˈo nˈoqteː"},
        {"ث", "sˈeː jˈe sˈeː nˈoqteː"},
        {"ج", "dʒˈim"},        {"چ", "tʃˈeː"},
        {"ح", "hˈe jˈe dʒˈimi"},  {"خ", "xˈɑʔ"},        {"د", "dˈɑl"},
        {"ذ", "zˈɑl"},        {"ر", "rˈɑʔ"},
        {"ز", "zˈe jˈe rˈeʔi"}, {"ژ", "ʒˈeː"},
        {"س", "sˈeː jˈe sˈeː dandˈuːneː"},
        {"ش", "ʃˈin"},        {"ص", "sˈɑd"},        {"ض", "zˈɑd"},
        {"ط", "tˈe jˈe dˈasteː dˈɑr"},
        {"ظ", "zˈe jˈe dˈasteː dˈɑr"},
        {"ع", "ˈejn"},        {"غ", "qˈejn"},       {"ف", "fˈeː"},
        {"ق", "qˈɑf"},        {"ک", "kˈɑf"},        {"گ", "ɡˈɑf"},
        {"ل", "lˈɑm"},        {"م", "mˈim"},        {"ن", "nˈun"},
        {"و", "vˈɑv"},        {"ه", "hˈe jˈe dˈo tʃˈeʃm"},
        {"ی", "jˈɑʔ"},
    };
    return m;
}

static std::string faLetterLookup(const std::string& text) {
    // trim surrounding spaces
    size_t a = text.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = text.find_last_not_of(" \t\r\n");
    std::string t = text.substr(a, b - a + 1);
    auto it = faLetterNames().find(t);
    return it == faLetterNames().end() ? "" : it->second;
}

// Canonical English letter names.  espeak spells a lone "A" as the letter it
// stands for only sometimes and reads "letter_a" as the word "lettera", so a
// lone letter is answered from this table.  Diphthongs are written with the
// phones our inventory actually has (eɪ -> "e ɪ", aɪ -> "a ɪ", oʊ -> "o ʊ"):
// @en:ˈeɪ and @en:ˈaɪ do not exist in the symbol table, they are dropped.
static const std::map<char, std::string>& enLetterNames() {
    static const std::map<char, std::string> m = {
        {'A', "ˈe ɪ"},   {'B', "bˈiː"},  {'C', "sˈiː"},  {'D', "dˈiː"},
        {'E', "ˈiː"},    {'F', "ˈɛ f"},  {'G', "dʒˈiː"}, {'H', "ˈe ɪ tʃ"},
        {'I', "ˈa ɪ"},   {'J', "dʒˈe ɪ"},{'K', "kˈe ɪ"}, {'L', "ˈɛ l"},
        {'M', "ˈɛ m"},   {'N', "ˈɛ n"},  {'O', "ˈo ʊ"},  {'P', "pˈiː"},
        {'Q', "k jˈuː"}, {'R', "ˈa r"},  {'S', "ˈɛ s"},  {'T', "tˈiː"},
        {'U', "jˈuː"},   {'V', "vˈiː"},  {'W', "dˈʌ b l j uː"},
        {'X', "ˈɛ k s"}, {'Y', "wˈa ɪ"}, {'Z', "zˈiː"},
    };
    return m;
}

static std::string enLetterLookup(const std::string& text) {
    size_t a = text.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = text.find_last_not_of(" \t\r\n");
    std::string t = text.substr(a, b - a + 1);
    // accept "A"/"a" and the corpus form "letter_a"
    const std::string prefix = "letter_";
    if (t.size() == prefix.size() + 1 && t.compare(0, prefix.size(), prefix) == 0)
        t = t.substr(prefix.size());
    if (t.size() != 1) return "";
    char c = static_cast<char>(std::toupper(static_cast<unsigned char>(t[0])));
    auto it = enLetterNames().find(c);
    return it == enLetterNames().end() ? "" : it->second;
}

// A run of single letters ("A B C", "R, E, D") is spelled letter by letter:
// espeak turns "B" into "bait" and "A" into "ɐ" once the input is more than one
// character, so every letter is answered from the table above instead.
static std::string enLetterSequenceLookup(const std::string& text) {
    std::string out;
    std::string tok;
    int count = 0;
    auto flush = [&]() -> bool {
        if (tok.empty()) return true;
        std::string name;
        if (tok.size() == 1) {
            char c = static_cast<char>(std::toupper(static_cast<unsigned char>(tok[0])));
            auto it = enLetterNames().find(c);
            if (it != enLetterNames().end()) name = it->second;
        }
        tok.clear();
        if (name.empty()) return false;
        if (!out.empty()) out += " ";
        out += name;
        ++count;
        return true;
    };
    for (char ch : text) {
        if (ch == ' ' || ch == '\t' || ch == ',' || ch == '.' || ch == ';' || ch == '-') {
            if (!flush()) return "";
        } else {
            tok += ch;
        }
    }
    if (!flush()) return "";
    return count >= 2 ? out : "";
}

// Original signature — wraps the config-based version with default paths
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
     std::string &normalizedString, std::string &ipaString) 
{
    NormalizeConfig config;
    config.espeak_data_path = "";         // empty → use default
    config.shakkelha_onnx = "./assets/shakkelha.onnx";
    config.ezafe_model_onnx = "./assets/ezafe_model.onnx";
    config.ezafe_model_spiece = "./assets/ezafe_spiece.model";
    config.hazm_words = "./assets/hazm_words.dat";
    config.hazm_verbs = "./assets/hazm_verbs.dat";
    config.hazm_stopwords = "./assets/hazm_stopwords.dat";
    config.homograph_data = "./assets/homograph_data.json";
    normalizeString(mainlang, ipa_mode, input, normalizedString, ipaString, config);
}

// Config-based version
void normalizeString(const Language mainlang, const int ipa_mode, const std::string& input,
     std::string &normalizedString, std::string &ipaString,
     const NormalizeConfig& config) 
{
    using clock = std::chrono::high_resolution_clock;
    using ms = std::chrono::duration<double, std::milli>;

    auto t_total = clock::now();

    // Whole input is a single Persian letter -> answer from the canonical table
    // before espeak sees it (its fa dictionary mangles پ/گ/غ/ق).
    if (mainlang == Language::PERSIAN) {
        std::string lone = faLetterLookup(input);
        if (!lone.empty()) {
            normalizedString = input;
            if (ipa_mode) ipaString = lone;
            return;
        }
    }
    if (mainlang == Language::ENGLISH && std::getenv("NT_LEGACY_LETTERS") == nullptr) {
        std::string lone = enLetterLookup(input);
        if (lone.empty()) lone = enLetterSequenceLookup(input);
        if (!lone.empty()) {
            normalizedString = input;
            if (ipa_mode) ipaString = lone;
            return;
        }
    }

    const char* data_path = config.espeak_data_path.empty() ? NULL : config.espeak_data_path.c_str();
    piper::eSpeakPhonemeConfig phonemeConfig;
    piper::PhonemeIdConfig idConfig;
    std::vector <std::vector <char32_t>> phonemes;

    // --- replacements + detection ---
    auto t0 = clock::now();
    normalizedString = performGeneralReplacements(mainlang, input);
    LanguageDetector detector(mainlang);
    std::vector<DetectedSegment> text_segments = detector.detect_segments(normalizedString);
    auto t1 = clock::now();

    double espeak_ms = 0, postprocess_ms = 0, espeak_init_ms = 0, pers_init_ms = 0, lang_repl_ms = 0;
    
    for(auto& text_segment : text_segments) {
        auto &segment_text = text_segment.text;
        auto &language = text_segment.language;
        std::string phoneme_segment;
        
        auto t_lr0 = clock::now();
        segment_text = performLanguageSpecificReplacements(language, mainlang, segment_text);
        lang_repl_ms += ms(clock::now() - t_lr0).count();

        if(language == Language::ARABIC && mainlang == Language::ARABIC) {
            int err = shakkelha_initialize(config.shakkelha_onnx.c_str());
            if(err!=0) {
                // Missing model must not kill the whole synthesis; continue
                // without diacritisation instead of returning empty audio.
                fprintf(stderr, "[NORM] shakkelha model missing, skipping diacritisation\n");
            } else {
                segment_text = shakkelha_diacritize(segment_text);
            }
            if(ipa_mode) {
                phoneme_segment = convertArabicToIPA(segment_text);
                ipaString += phoneme_segment;
            }
        } else {
            auto t_espeak_init0 = clock::now();
            int err = piper::phonemize_eSpeak_Init(data_path);
            espeak_init_ms += ms(clock::now() - t_espeak_init0).count();

            if(err!=0) { std::cout << "Espeak data folder not found!" << std::endl; return; }
            phonemes.clear();
            phonemeConfig.voice = (language == Language::PERSIAN || language == Language::ARABIC) ? "fa" : "en-us";

            if(ipa_mode) {
                auto t_esp = clock::now();
                piper::phonemize_eSpeak(segment_text, phonemeConfig, phonemes); 
                phoneme_segment = piper::getIpaString(phonemes);
                espeak_ms += ms(clock::now() - t_esp).count();

                if(language == Language::PERSIAN) {
                    auto t_pers_init0 = clock::now();
                    if(!persianResourcesInitialized) {
                        persian_phoneme::initPersianResources(
                            config.ezafe_model_onnx.c_str(),
                            config.ezafe_model_spiece.c_str(),
                            config.hazm_words.c_str(),
                            config.hazm_verbs.c_str(),
                            config.hazm_stopwords.c_str(),
                            config.homograph_data.c_str()
                        );
                        persianResourcesInitialized = true;
                    }
                    pers_init_ms += ms(clock::now() - t_pers_init0).count();

                    auto t_post0 = clock::now();
                    phoneme_segment = persian_phoneme::postprocessPersianIPA(segment_text, phoneme_segment);
                    postprocess_ms += ms(clock::now() - t_post0).count();

                    // espeak's fa output marks ق/غ as "q1"; the model phone set
                    // only knows "q", so normalise the marker away.
                    for (size_t p; (p = phoneme_segment.find("q1")) != std::string::npos; )
                        phoneme_segment.replace(p, 2, "q");

                    // espeak returns nothing when a clause STARTS with words such
                    // as "الف" (it phonemes fine mid-sentence).  A leading space
                    // changes the clause start without altering the phonemes.
                    bool blank = phoneme_segment.find_first_not_of(" \t\r\n") == std::string::npos;
                    bool hasText = segment_text.find_first_not_of(" \t\r\n") != std::string::npos;
                    if (blank && hasText) {
                        phonemes.clear();
                        piper::phonemize_eSpeak(" " + segment_text, phonemeConfig, phonemes);
                        std::string retry = piper::getIpaString(phonemes);
                        if (retry.find_first_not_of(" \t\r\n") != std::string::npos)
                            phoneme_segment = retry;
                    }
                }
                ipaString += phoneme_segment;
            }
        }
    }

    normalizedString = LanguageDetector::reunite_segments(text_segments);

    double total_ms = ms(clock::now() - t_total).count();
    double repl_detect_ms = ms(t1 - t0).count();
    double compute_ms = repl_detect_ms + lang_repl_ms + espeak_ms + postprocess_ms;
    double load_ms = espeak_init_ms + pers_init_ms;

    fprintf(stderr, "[NORM] total=%.1fms  load=%.1fms  compute=%.1fms  repl=%.1f  lang_repl=%.1f  espeak=%.1f  postproc=%.1f\n",
            total_ms, load_ms, compute_ms, repl_detect_ms, lang_repl_ms, espeak_ms, postprocess_ms);
}
