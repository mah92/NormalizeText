#ifndef SHAKKELHA_H
#define SHAKKELHA_H

#include <string>

int shakkelha_initialize(const std::string model_address);

std::wstring shakkelha_diacritize(const std::wstring& input);
std::string shakkelha_diacritize(const std::string& input);

//Utility functions
std::string wstring_to_string(const std::wstring& wstr);
std::wstring string_to_wstring(const std::string& str);

#endif