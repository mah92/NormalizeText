#include "homograph_dict.h"
#include "../json_cpp/nlohmann_json.hpp"

#include <fstream>
#include <stdexcept>

namespace homograph {

HomographDict loadHomographDict(const std::string& jsonPath) {
    HomographDict dict;

    // Empty/disabled path → return an EMPTY dict (no homograph correction).
    // This optional NormalizeText component must never throw and take down
    // the whole TTS pipeline (2026-09-04, same fix as the ezafe detector).
    if (jsonPath.empty()) return dict;

    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        return dict;
    }

    nlohmann::json j;
    file >> j;

    for (auto& [grapheme, phonemeMap] : j.items()) {
        std::unordered_map<std::string, std::vector<std::string>> inner;
        for (auto& [phoneme, words] : phonemeMap.items()) {
            std::vector<std::string> wordList;
            for (auto& w : words) {
                wordList.push_back(w.get<std::string>());
            }
            inner[phoneme] = std::move(wordList);
        }
        dict[grapheme] = std::move(inner);
    }

    return dict;
}

} // namespace homograph