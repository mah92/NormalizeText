#include "./vits2-tokenizer.h"

#include <map>
#include <cctype>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <cstdlib>
#include "uni_algo.h"

static std::map<std::string, uint8_t> replacement_map;

int readFileToMap(const char* fileAddress) 
{
    std::map<std::string, uint8_t> resultMap;
    std::ifstream inputFile(fileAddress);
    
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << fileAddress << std::endl;
        return -1;
    }
    
    std::string line;
    while (std::getline(inputFile, line)) {
        // Skip empty lines and lines starting with #, space or tab
        if (line.empty() || line[0] == '#' || line[0] == ' ' || line[0] == '\t') {
            continue;
        }
        
        size_t tabPos = line.find('\t');
        if (tabPos != std::string::npos && tabPos != 0 && tabPos != line.length() - 1) {
            std::string key = line.substr(0, tabPos);
            std::string value_str = line.substr(tabPos + 1);

            uint8_t value = static_cast<uint8_t>(std::stoi(value_str));
            resultMap[key] = value;
        }
    }
    
    replacement_map = resultMap;
    inputFile.close();
    return 0;
}

// Punctuation marks that trigger EOS/BOS
static const std::u32string punctuation = U"\t\n;:,.!?\"'(){}[]\u061B\u060C\u061F\u00AB\u00BB";

std::vector<uint8_t> string_to_id_vector(const std::string& text) {
    std::vector<uint8_t> result;

    // Add BOS token (2)
    result.push_back(replacement_map["<bos>"]);

    // Convert UTF-8 to UTF-32
    std::u32string u32_text;
    {
        auto conv_result = una::utf8to32(text);
        u32_text.assign(conv_result.begin(), conv_result.end());
    }
    
    for (size_t i = 0; i < u32_text.size(); ) {
        bool replaced = false;

        // Check for space character first
        if (u32_text[i] == U' ') {
            result.push_back(replacement_map["<space>"]);
            i++;
            continue;
        }

        // Check punctuation
        if (punctuation.find(u32_text[i]) != std::u32string::npos) {
            result.push_back(replacement_map["<eos>"]);
            result.push_back(replacement_map["<bos>"]);
            i++;
            continue;
        }

        for (const auto& [pattern, replacement] : replacement_map) {
            if (pattern == "<bos>" || pattern == "<eos>" || pattern == "<unk>" || pattern == "<space>") continue;

            // Convert pattern to UTF-32
            std::u32string u32_pattern;
            {
                auto conv_result = una::utf8to32(pattern);
                u32_pattern.assign(conv_result.begin(), conv_result.end());
            }

            if (i + u32_pattern.size() <= u32_text.size() &&
                std::equal(u32_pattern.begin(), u32_pattern.end(), u32_text.begin() + i)) {
                result.push_back(replacement);
                i += u32_pattern.size();
                replaced = true;
                break;
            }
        }

        if (!replaced) {
            // Get the UTF-8 representation of the unknown character
            std::string unknown_char;
            char32_t ch = u32_text[i];
            if (ch <= 0x7F) {
                unknown_char += static_cast<char>(ch);
            } else {
                // Convert UTF-32 character back to UTF-8 for display
                auto utf8_seq = una::utf32to8(std::u32string(1, ch));
                unknown_char.assign(utf8_seq.begin(), utf8_seq.end());
            }
            
            std::cerr << "Warning: Unknown character/sequence '" << unknown_char 
                      << "' (U+" << std::hex << std::uppercase << static_cast<uint32_t>(ch) 
                      << ") replaced with <unk> token" << std::endl;
            
            result.push_back(replacement_map["<unk>"]);
            i++;
        }
    }

    // Add EOS token (3)
    result.push_back(replacement_map["<eos>"]);
    return result;
}

std::string id_vector_to_id_string(const std::vector<uint8_t>& bytes) {
    std::string result;
    
    for (size_t i = 0; i < bytes.size(); ++i) {
        result += std::to_string(bytes[i]);
        if (i < bytes.size() - 1) {
            result += "\t";
        }
    }
    
    return result;
}