#ifndef PHOEMIZE_H_
#define PHOEMIZE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "./phoneme_ids.hpp"

#define CLAUSE_INTONATION_FULL_STOP 0x00000000
#define CLAUSE_INTONATION_COMMA 0x00001000
#define CLAUSE_INTONATION_QUESTION 0x00002000
#define CLAUSE_INTONATION_EXCLAMATION 0x00003000

#define CLAUSE_TYPE_CLAUSE 0x00040000
#define CLAUSE_TYPE_SENTENCE 0x00080000

#define CLAUSE_PERIOD (40 | CLAUSE_INTONATION_FULL_STOP | CLAUSE_TYPE_SENTENCE)
#define CLAUSE_COMMA (20 | CLAUSE_INTONATION_COMMA | CLAUSE_TYPE_CLAUSE)
#define CLAUSE_QUESTION (40 | CLAUSE_INTONATION_QUESTION | CLAUSE_TYPE_SENTENCE)
#define CLAUSE_EXCLAMATION                                                     \
  (45 | CLAUSE_INTONATION_EXCLAMATION | CLAUSE_TYPE_SENTENCE)
#define CLAUSE_COLON (30 | CLAUSE_INTONATION_FULL_STOP | CLAUSE_TYPE_CLAUSE)
#define CLAUSE_SEMICOLON (30 | CLAUSE_INTONATION_COMMA | CLAUSE_TYPE_CLAUSE)

namespace piper {

typedef std::map<char32_t, std::vector<char32_t>> PhonemeMap;

struct eSpeakPhonemeConfig {
  std::string voice = "en-us";

  char32_t period = U'.';      // CLAUSE_PERIOD
  char32_t comma = U',';       // CLAUSE_COMMA
  char32_t question = U'?';    // CLAUSE_QUESTION
  char32_t exclamation = U'!'; // CLAUSE_EXCLAMATION
  char32_t colon = U':';       // CLAUSE_COLON
  char32_t semicolon = U';';   // CLAUSE_SEMICOLON
  char32_t space = U' ';

  // Remove language switch flags like "(en)"
  bool keepLanguageFlags = false;

  std::shared_ptr<PhonemeMap> phonemeMap;
};

// Phonemizes text using espeak-ng.
// Returns phonemes for each sentence as a separate std::vector.
//
// Assumes espeak_Initialize has already been called.
//void
//phonemize_eSpeak(std::string text, eSpeakPhonemeConfig &config,
//                 std::vector<std::vector<char32_t>> &phonemes);

enum TextCasing {
  CASING_IGNORE = 0,
  CASING_LOWER = 1,
  CASING_UPPER = 2,
  CASING_FOLD = 3
};

// Configuration for phonemize_codepoints
struct CodepointsPhonemeConfig {
  TextCasing casing = CASING_FOLD;
  std::shared_ptr<PhonemeMap> phonemeMap;
};

int phonemize_eSpeak_Init(const char* espeak_data_path = NULL);
void phonemize_eSpeak_Terminate();

// "Phonemizes" text as a series of normalized UTF-8 codepoints.
// Returns a single std::vector of "phonemes".
//
// Does not detect sentence boundaries.
void phonemize_eSpeak(std::string text, eSpeakPhonemeConfig &config,
  std::vector<std::vector<char32_t>> &phonemes); 

//Helper functions for visualization
std::string getPhonemeString(const std::vector<std::vector<char32_t>> &phonemes);

std::string getIdString(const std::vector<std::vector<char32_t>> &phonemes, piper::PhonemeIdConfig &idConfig);

} // namespace piper

#endif // PHONEMIZE_H_





