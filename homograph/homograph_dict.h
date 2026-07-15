#ifndef HOMOGRAPH_DICT_H
#define HOMOGRAPH_DICT_H

#include <string>
#include <unordered_map>
#include <vector>

namespace homograph {

// grapheme → {ipa_phoneme → [context_words]}
using HomographDict = std::unordered_map<std::string,
      std::unordered_map<std::string, std::vector<std::string>>>;

HomographDict loadHomographDict(const std::string& jsonPath);

} // namespace homograph

#endif // HOMOGRAPH_DICT_H