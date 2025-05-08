#ifndef VITS2_TOKENIZER_H
#define VITS2_TOKENIZER_H

#include <string>
#include <vector>
#include <stdint.h>

int readFileToMap(const char* fileAddress);

std::vector<uint8_t> string_to_id_vector(const std::string& text);

std::string id_vector_to_id_string(const std::vector<uint8_t>& bytes);

#endif