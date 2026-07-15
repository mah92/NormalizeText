#include "homograph_dict.h"
#include "../include/json.hpp"

#include <fstream>
#include <stdexcept>

namespace homograph {

HomographDict loadHomographDict(const std::string& jsonPath) {
    HomographDict dict;

    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open homograph data: " + jsonPath);
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