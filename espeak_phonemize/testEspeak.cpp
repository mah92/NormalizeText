#include "./phonemize.hpp"

#include <espeak-ng/speak_lib.h>

#include <iostream>
#include <cstring>
#include <string>

int main()
{
  piper::phonemize_eSpeak_Init(NULL);

  // Check whitespace around punctuation ////////////////////////////
  std::vector<std::vector<char32_t>> phonemes;
  piper::eSpeakPhonemeConfig phonemeConfig;
  phonemeConfig.voice = "en-us";
  piper::phonemize_eSpeak("this, is: a; test.", phonemeConfig, phonemes);  
  std::string phonemeStr = piper::getPhonemeString(phonemes);
  std::cout << "punctuation test: " << phonemeStr << std::endl;
  if (phonemeStr != "ðˈɪs, ɪz: ˈeɪ; tˈɛst.\n") {
    std::cerr << "FAIL\n" << std::endl;
  } else {
    std::cout << "PASS\n" << std::endl;
  }

  // Capitalization is required to get espeak to split the sentences.
  phonemes.clear();
  phonemeConfig.voice = "en-us";
  piper::phonemize_eSpeak("Test 1. Test 2.", phonemeConfig, phonemes);  
  std::string phonemeStr2 = piper::getPhonemeString(phonemes);
  std::cout << "punctuation test: " << phonemeStr2 << std::endl;
  if (phonemeStr2 != "tˈɛst wˈʌn.\ntˈɛst tˈuː.\n") {
    std::cerr << "FAIL\n" << std::endl;
  } else {
    std::cout << "PASS\n" << std::endl;
  }

  // Check "licht" in German ///////////////////////////////////////
  phonemes.clear();
  piper::PhonemeIdConfig idConfig;
  phonemeConfig.voice = "de";

  // Should be "lˈɪçt!" where "ç" is decomposed into two codepoints
  piper::phonemize_eSpeak("licht!", phonemeConfig, phonemes);

  // 0 = pad
  // 1 = bos
  // 2 = eos
  // 4 = !
  std::string idStr = piper::getIdString(phonemes, idConfig);
  std::cout << "id test: licht: " << "1 0 24 0 120 0 74 0 16 0 140 0 32 0 4 0 2 " << std::endl;
  if (idStr != "1 0 24 0 120 0 74 0 16 0 140 0 32 0 4 0 2 ") {
    std::cerr << "FAIL\n" << std::endl;
  } else {
    std::cout << "PASS\n" << std::endl;
  }
  
  // --------------------------------------------------------------------------

  piper::phonemize_eSpeak_Terminate();
}

