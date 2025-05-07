//To work on espeaks before version 1.53

#ifndef TEXT_TO_PHONEMES_WITH_TERMINATOR_H
#define TEXT_TO_PHONEMES_WITH_TERMINATOR_H

#include "phonemize.hpp"
#include "uni_algo.h"
#include <vector>
#include <string>
#include <cstring> // for strncpy


static const std::vector<std::pair<char32_t, int> > clause_map = {
    {',', CLAUSE_COMMA},
    {0x060C, CLAUSE_COMMA}, // Arabic comma (،)
    {'\t', CLAUSE_COMMA},
    {'\'', CLAUSE_COMMA},
    {'"', CLAUSE_COMMA},
    {'`', CLAUSE_COMMA},
    {0x00AB, CLAUSE_COMMA},  // Left-pointing double angle quotation mark («)
    {0x00BB, CLAUSE_COMMA},  // Right-pointing double angle quotation mark (»)
    {'(', CLAUSE_COMMA},
    {')', CLAUSE_COMMA},
    {'{', CLAUSE_COMMA},
    {'}', CLAUSE_COMMA},
    {'[', CLAUSE_COMMA},
    {']', CLAUSE_COMMA},
    {'?', CLAUSE_QUESTION},
    {0x061F, CLAUSE_QUESTION}, // Arabic question mark (؟)
    {';', CLAUSE_SEMICOLON},
    {0x061B, CLAUSE_SEMICOLON}, // Arabic semicolon (؛)
    {'!', CLAUSE_EXCLAMATION},
    {':', CLAUSE_COLON},
    {'.', CLAUSE_PERIOD},
    {'\n', CLAUSE_TYPE_SENTENCE},
    //{'\0', CLAUSE_TYPE_SENTENCE},
};

static char text_buffer[10000];

struct SearchResult {
    size_t copy_len;
    int clause_type;
};

SearchResult search_special(const char* text, char* copied_text) {

    std::string_view sv(text);
    auto utf8_view = una::views::utf8(sv);

    int clause_type = 0; //None
    size_t copy_len = 0;
    bool found = false;

    for (auto it = utf8_view.begin(); it != utf8_view.end() && !found; ++it) {
        char32_t ch = *it;

        for (const auto& entry : clause_map) {
            if (ch == entry.first) {
                clause_type = entry.second;
                found = true;
                break;
            }
        }

        if(ch == '\0') {
            clause_type = CLAUSE_TYPE_SENTENCE;
            break;
        }
        
        //Add copy_len for all except null char
        auto utf8_char = una::utf32to8(std::u32string_view(&ch, 1));
        copy_len += utf8_char.size();
        
        if (found) {
            break;
        }
    }

    strncpy(copied_text, text, copy_len);
    copied_text[copy_len] = '\0';

    return {copy_len, clause_type};
}

const char *espeak_TextToPhonemesWithTerminator(const void **textptr, int textmode, int phonememode, int *terminator) {
    
    char* copied_text = (char*)&text_buffer[0];
    SearchResult sr = search_special((char*)*textptr, (char*)copied_text);
    // printf("copied_text:(%i)(%i)\n", sr.copy_len, strlen((char*)*textptr));
    // printf("%s\n", copied_text);

    if(sr.copy_len == 0) {
        *textptr = 0;
        if(terminator) *terminator |= CLAUSE_TYPE_SENTENCE;
        return "";
    }

    // static int aa = 0;
    // aa++;
    // if(aa == 5)
    //     exit(10);

    const char* phonemes = espeak_TextToPhonemes((const void **)&copied_text, textmode, phonememode);
    
    if(terminator) {
        *terminator = sr.clause_type; //0 == CLAUSE_TYPE_NONE
    }
    
    if(sr.copy_len == strlen((char*)*textptr)) {
        *textptr = 0;
        if(terminator) *terminator |= CLAUSE_TYPE_SENTENCE;
    } else {
        *textptr += sr.copy_len;
    }

    return phonemes;
}

#endif