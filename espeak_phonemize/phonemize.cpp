#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <espeak-ng/speak_lib.h>

#include "phonemize.hpp"
#include "phoneme_ids.hpp"
#include "uni_algo.h"

#if 1
//To work on espeaks before version 1.53
#include "text_to_phonemes_with_terminator.h"
#endif

namespace piper {

// language -> phoneme -> [phoneme, ...]
std::map<std::string, PhonemeMap> DEFAULT_PHONEME_MAP = {
    {"pt-br", {{U'c', {U'k'}}}}};

static bool espeak_inited = false;
int phonemize_eSpeak_Init(const char* espeak_data_path) 
{
  if(espeak_inited)
    return 0;

  //path: The directory which contains the espeak-ng-data directory, or NULL for the default location.
  int result = espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, 0, espeak_data_path, 0);
  if (result < 0) {
    std::cerr << "Failed to initialize eSpeak" << std::endl;
    return 1;
  }
  espeak_inited = true;
  return 0;
}

void phonemize_eSpeak_Terminate()
{  
  if(espeak_inited) {
    espeak_Terminate();
    espeak_inited = false;
  }
}

void phonemize_eSpeak(std::string text, eSpeakPhonemeConfig &config,
                 std::vector<std::vector<char32_t>> &phonemes) 
{
  if(!espeak_inited) {
    return;
  }

  auto voice = config.voice;
  int result = espeak_SetVoiceByName(voice.c_str());
  if (result != 0) {
    throw std::runtime_error("Failed to set eSpeak-ng voice");
  }

  std::shared_ptr<PhonemeMap> phonemeMap;
  if (config.phonemeMap) {
    phonemeMap = config.phonemeMap;
  } else if (DEFAULT_PHONEME_MAP.count(voice) > 0) {
    phonemeMap = std::make_shared<PhonemeMap>(DEFAULT_PHONEME_MAP[voice]);
  }

  // Modified by eSpeak
  std::string textCopy(text);

  std::vector<char32_t> *sentencePhonemes = nullptr;
  const char *inputTextPointer = textCopy.c_str();
  int terminator = 0;

  while (inputTextPointer != NULL) {
    // Modified espeak-ng API to get access to clause terminator
    std::string clausePhonemes(espeak_TextToPhonemesWithTerminator(
        (const void **)&inputTextPointer,
        /*textmode*/ espeakCHARS_AUTO,
        /*phonememode = IPA*/ 0x02, &terminator));

    // Decompose, e.g. "ç" -> "c" + "̧"
    auto phonemesNorm = una::norm::to_nfd_utf8(clausePhonemes);
    auto phonemesRange = una::ranges::utf8_view{phonemesNorm};

    if (!sentencePhonemes) {
      // Start new sentence
      phonemes.emplace_back();
      sentencePhonemes = &phonemes[phonemes.size() - 1];
    }

    // Maybe use phoneme map
    std::vector<char32_t> mappedSentPhonemes;
    if (phonemeMap) {
      for (auto phoneme : phonemesRange) {
        if (phonemeMap->count(phoneme) < 1) {
          // No mapping for phoneme
          mappedSentPhonemes.push_back(phoneme);
        } else {
          // Mapping for phoneme
          auto mappedPhonemes = &(phonemeMap->at(phoneme));
          mappedSentPhonemes.insert(mappedSentPhonemes.end(),
                                    mappedPhonemes->begin(),
                                    mappedPhonemes->end());
        }
      }
    } else {
      // No phoneme map
      mappedSentPhonemes.insert(mappedSentPhonemes.end(), phonemesRange.begin(),
                                phonemesRange.end());
    }

    auto phonemeIter = mappedSentPhonemes.begin();
    auto phonemeEnd = mappedSentPhonemes.end();

    if (config.keepLanguageFlags) {
      // No phoneme filter
      sentencePhonemes->insert(sentencePhonemes->end(), phonemeIter,
                               phonemeEnd);
    } else {
      // Filter out (lang) switch (flags).
      // These surround words from languages other than the current voice.
      bool inLanguageFlag = false;

      while (phonemeIter != phonemeEnd) {
        if (inLanguageFlag) {
          if (*phonemeIter == U')') {
            // End of (lang) switch
            inLanguageFlag = false;
          }
        } else if (*phonemeIter == U'(') {
          // Start of (lang) switch
          inLanguageFlag = true;
        } else {
          sentencePhonemes->push_back(*phonemeIter);
        }

        phonemeIter++;
      }
    }

    // Add appropriate punctuation depending on terminator type
    int punctuation = terminator & 0x000FFFFF;
    if (punctuation == CLAUSE_PERIOD) {
      sentencePhonemes->push_back(config.period);
    } else if (punctuation == CLAUSE_QUESTION) {
      sentencePhonemes->push_back(config.question);
    } else if (punctuation == CLAUSE_EXCLAMATION) {
      sentencePhonemes->push_back(config.exclamation);
    } else if (punctuation == CLAUSE_COMMA) {
      sentencePhonemes->push_back(config.comma);
      sentencePhonemes->push_back(config.space);
    } else if (punctuation == CLAUSE_COLON) {
      sentencePhonemes->push_back(config.colon);
      sentencePhonemes->push_back(config.space);
    } else if (punctuation == CLAUSE_SEMICOLON) {
      sentencePhonemes->push_back(config.semicolon);
      sentencePhonemes->push_back(config.space);
    }

    if ((terminator & CLAUSE_TYPE_SENTENCE) == CLAUSE_TYPE_SENTENCE) {
      // End of sentence
      sentencePhonemes = nullptr;
    }

  } // while inputTextPointer != NULL

} /* phonemize_eSpeak */


//////////////////////////////////////////////////////////////////////////////////
#if 0
void phonemize_codepoints(std::string text, CodepointsPhonemeConfig &config,
                     std::vector<std::vector<char32_t>> &phonemes) {

  if (config.casing == CASING_LOWER) {
    text = una::cases::to_lowercase_utf8(text);
  } else if (config.casing == CASING_UPPER) {
    text = una::cases::to_uppercase_utf8(text);
  } else if (config.casing == CASING_FOLD) {
    text = una::cases::to_casefold_utf8(text);
  }

  // Decompose, e.g. "ç" -> "c" + "̧"
  auto phonemesNorm = una::norm::to_nfd_utf8(text);
  auto phonemesRange = una::ranges::utf8_view{phonemesNorm};

  // No sentence boundary detection
  phonemes.emplace_back();
  auto sentPhonemes = &phonemes[phonemes.size() - 1];

  if (config.phonemeMap) {
    for (auto phoneme : phonemesRange) {
      if (config.phonemeMap->count(phoneme) < 1) {
        // No mapping for phoneme
        sentPhonemes->push_back(phoneme);
      } else {
        // Mapping for phoneme
        auto mappedPhonemes = &(config.phonemeMap->at(phoneme));
        sentPhonemes->insert(sentPhonemes->end(), mappedPhonemes->begin(),
                             mappedPhonemes->end());
      }
    }
  } else {
    // No phoneme map
    sentPhonemes->insert(sentPhonemes->end(), phonemesRange.begin(),
                         phonemesRange.end());
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////

std::string getPhonemeString(const std::vector<std::vector<char32_t>> &phonemes) {
  std::stringstream phonemeStr;
  for (auto &sentencePhonemes : phonemes) {
    for (auto phoneme : sentencePhonemes) {
      // Convert to UTF-8 string
      std::u32string phonemeU32Str;
      phonemeU32Str += phoneme;

      phonemeStr << una::utf32to8(phonemeU32Str);
    }

    phonemeStr << "\n";
  }

  return phonemeStr.str();
}

std::string getIdString(const std::vector<std::vector<char32_t>> &phonemes,
  piper::PhonemeIdConfig &idConfig) {
  std::map<char32_t, std::size_t> missingPhonemes;
  std::stringstream idStr;
  for (auto sentPhonemes : phonemes) {
    std::vector<piper::PhonemeId> sentIds;
    piper::phonemes_to_ids(sentPhonemes, idConfig, sentIds, missingPhonemes);

    for (auto id : sentIds) {
      idStr << id << " ";
    }
  }

  return idStr.str();
}

} // namespace piper


