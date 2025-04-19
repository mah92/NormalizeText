#include "replacement.h"
#include "language_detector/language_detector.h"
#include "language-specific-codes/arabic/normalize_numbers_arabic.h"
#include "language-specific-codes/english/normalize_numbers_english.h"
#include "language-specific-codes/persian/normalize_numbers_persian.h"

#include "language-specific-codes/arabic/arabic_symbols.h"
#include "language-specific-codes/persian/persian_symbols.h"

#include <regex>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <vector>

// UTF-8 character replacements
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_NO_SPACE_ALL = {
    // Arabic digits (٠-٩)
    {"\xD9\xA0", "0"}, {"\xD9\xA1", "1"}, {"\xD9\xA2", "2"}, 
    {"\xD9\xA3", "3"}, {"\xD9\xA4", "4"}, {"\xD9\xA5", "5"},
    {"\xD9\xA6", "6"}, {"\xD9\xA7", "7"}, {"\xD9\xA8", "8"},
    {"\xD9\xA9", "9"},

    // Persian digits (۰-۹)
    {"\xDB\xB0", "0"}, {"\xDB\xB1", "1"}, {"\xDB\xB2", "2"},
    {"\xDB\xB3", "3"}, {"\xDB\xB4", "4"}, {"\xDB\xB5", "5"},
    {"\xDB\xB6", "6"}, {"\xDB\xB7", "7"}, {"\xDB\xB8", "8"},
    {"\xDB\xB9", "9"},

    // Decimal separator
    {"\xD9\xAB", "."},

    //Mathematical Bold
    {"𝐀", "A"}, {"𝐚", "a"}, {"𝐁", "B"}, {"𝐛", "b"}, {"𝐂", "C"}, {"𝐜", "c"}, {"𝐃", "D"}, {"𝐝", "d"}, {"𝐄", "E"}, {"𝐞", "e"}, {"𝐅", "F"}, {"𝐟", "f"}, {"𝐆", "G"}, {"𝐠", "g"}, {"𝐇", "H"}, {"𝐡", "h"}, {"𝐈", "I"}, {"𝐢", "i"}, {"𝐉", "J"}, {"𝐣", "j"}, {"𝐊", "K"}, {"𝐤", "k"}, {"𝐋", "L"}, {"𝐥", "l"}, {"𝐌", "M"}, {"𝐦", "m"}, {"𝐍", "N"}, {"𝐧", "n"}, {"𝐎", "O"}, {"𝐨", "o"}, {"𝐏", "P"}, {"𝐩", "p"}, {"𝐐", "Q"}, {"𝐪", "q"}, {"𝐑", "R"}, {"𝐫", "r"}, {"𝐒", "S"}, {"𝐬", "s"}, {"𝐓", "T"}, {"𝐭", "t"}, {"𝐔", "U"}, {"𝐮", "u"}, {"𝐕", "V"}, {"𝐯", "v"}, {"𝐖", "W"}, {"𝐰", "w"}, {"𝐗", "X"}, {"𝐱", "x"}, {"𝐘", "Y"}, {"𝐲", "y"}, {"𝐙", "Z"}, {"𝐳", "z"},

    //Mathematical Italic
    {"𝐴", "A"}, {"𝑎", "a"}, {"𝐵", "B"}, {"𝑏", "b"}, {"𝐶", "C"}, {"𝑐", "c"}, {"𝐷", "D"}, {"𝑑", "d"}, {"𝐸", "E"}, {"𝑒", "e"}, {"𝐹", "F"}, {"𝑓", "f"}, {"𝐺", "G"}, {"𝑔", "g"}, {"𝐻", "H"}, {"ℎ", "h"}, {"𝐼", "I"}, {"𝑖", "i"}, {"𝐽", "J"}, {"𝑗", "j"}, {"𝐾", "K"}, {"𝑘", "k"}, {"𝐿", "L"}, {"𝑙", "l"}, {"𝑀", "M"}, {"𝑚", "m"}, {"𝑁", "N"}, {"𝑛", "n"}, {"𝑂", "O"}, {"𝑜", "o"}, {"𝑃", "P"}, {"𝑝", "p"}, {"𝑄", "Q"}, {"𝑞", "q"}, {"𝑅", "R"}, {"𝑟", "r"}, {"𝑆", "S"}, {"𝑠", "s"}, {"𝑇", "T"}, {"𝑡", "t"}, {"𝑈", "U"}, {"𝑢", "u"}, {"𝑉", "V"}, {"𝑣", "v"}, {"𝑊", "W"}, {"𝑤", "w"}, {"𝑋", "X"}, {"𝑥", "x"}, {"𝑌", "Y"}, {"𝑦", "y"}, {"𝑍", "Z"}, {"𝑧", "z"},

    //Mathematical Bold Italic
    {"𝑨", "A"}, {"𝒂", "a"}, {"𝑩", "B"}, {"𝒃", "b"}, {"𝑪", "C"}, {"𝒄", "c"}, {"𝑫", "D"}, {"𝒅", "d"}, {"𝑬", "E"}, {"𝒆", "e"}, {"𝑭", "F"}, {"𝒇", "f"}, {"𝑮", "G"}, {"𝒈", "g"}, {"𝑯", "H"}, {"𝒉", "h"}, {"𝑰", "I"}, {"𝒊", "i"}, {"𝑱", "J"}, {"𝒋", "j"}, {"𝑲", "K"}, {"𝒌", "k"}, {"𝑳", "L"}, {"𝒍", "l"}, {"𝑴", "M"}, {"𝒎", "m"}, {"𝑵", "N"}, {"𝒏", "n"}, {"𝑶", "O"}, {"𝒐", "o"}, {"𝑷", "P"}, {"𝒑", "p"}, {"𝑸", "Q"}, {"𝒒", "q"}, {"𝑹", "R"}, {"𝒓", "r"}, {"𝑺", "S"}, {"𝒔", "s"}, {"𝑻", "T"}, {"𝒕", "t"}, {"𝑼", "U"}, {"𝒖", "u"}, {"𝑽", "V"}, {"𝒗", "v"}, {"𝑾", "W"}, {"𝒘", "w"}, {"𝑿", "X"}, {"𝒙", "x"}, {"𝒀", "Y"}, {"𝒚", "y"}, {"𝒁", "Z"}, {"𝒛", "z"},

    //Mathematical Script
    {"𝒜", "A"}, {"𝒶", "a"}, {"ℬ", "B"}, {"𝒷", "b"}, {"𝒞", "C"}, {"𝒸", "c"}, {"𝒟", "D"}, {"𝒹", "d"}, {"ℰ", "E"}, {"ℯ", "e"}, {"ℱ", "F"}, {"𝒻", "f"}, {"𝒢", "G"}, {"ℊ", "g"}, {"ℋ", "H"}, {"𝒽", "h"}, {"ℐ", "I"}, {"𝒾", "i"}, {"𝒥", "J"}, {"𝒿", "j"}, {"𝒦", "K"}, {"𝓀", "k"}, {"ℒ", "L"}, {"𝓁", "l"}, {"ℳ", "M"}, {"𝓂", "m"}, {"𝒩", "N"}, {"𝓃", "n"}, {"𝒪", "O"}, {"ℴ", "o"}, {"𝒫", "P"}, {"𝓅", "p"}, {"𝒬", "Q"}, {"𝓆", "q"}, {"ℛ", "R"}, {"𝓇", "r"}, {"𝒮", "S"}, {"𝓈", "s"}, {"𝒯", "T"}, {"𝓉", "t"}, {"𝒰", "U"}, {"𝓊", "u"}, {"𝒱", "V"}, {"𝓋", "v"}, {"𝒲", "W"}, {"𝓌", "w"}, {"𝒳", "X"}, {"𝓍", "x"}, {"𝒴", "Y"}, {"𝓎", "y"}, {"𝒵", "Z"}, {"𝓏", "z"},

    // Mathematical Bold Script (𝓐-𝓩, 𝓪-𝔃)
    {"𝓐", "A"}, {"𝓑", "B"}, {"𝓒", "C"}, {"𝓓", "D"}, {"𝓔", "E"}, {"𝓕", "F"}, {"𝓖", "G"}, {"𝓗", "H"}, {"𝓘", "I"}, {"𝓙", "J"}, {"𝓚", "K"}, {"𝓛", "L"}, {"𝓜", "M"}, {"𝓝", "N"}, {"𝓞", "O"}, {"𝓟", "P"}, {"𝓠", "Q"}, {"𝓡", "R"}, {"𝓢", "S"}, {"𝓣", "T"}, {"𝓤", "U"}, {"𝓥", "V"}, {"𝓦", "W"}, {"𝓧", "X"}, {"𝓨", "Y"}, {"𝓩", "Z"}, {"𝓪", "a"}, {"𝓫", "b"}, {"𝓬", "c"}, {"𝓭", "d"}, {"𝓮", "e"}, {"𝓯", "f"}, {"𝓰", "g"}, {"𝓱", "h"}, {"𝓲", "i"}, {"𝓳", "j"}, {"𝓴", "k"}, {"𝓵", "l"}, {"𝓶", "m"}, {"𝓷", "n"}, {"𝓸", "o"}, {"𝓹", "p"}, {"𝓺", "q"}, {"𝓻", "r"}, {"𝓼", "s"}, {"𝓽", "t"}, {"𝓾", "u"}, {"𝓿", "v"}, {"𝔀", "w"}, {"𝔁", "x"}, {"𝔂", "y"}, {"𝔃", "z"},

    //Mathematical Fraktur
    {"𝔄", "A"}, {"𝔞", "a"}, {"𝔅", "B"}, {"𝔟", "b"}, {"ℭ", "C"}, {"𝔠", "c"}, {"𝔇", "D"}, {"𝔡", "d"}, {"𝔈", "E"}, {"𝔢", "e"}, {"𝔉", "F"}, {"𝔣", "f"}, {"𝔊", "G"}, {"𝔤", "g"}, {"ℌ", "H"}, {"𝔥", "h"}, {"ℑ", "I"}, {"𝔦", "i"}, {"𝔍", "J"}, {"𝔧", "j"}, {"𝔎", "K"}, {"𝔨", "k"}, {"𝔏", "L"}, {"𝔩", "l"}, {"𝔐", "M"}, {"𝔪", "m"}, {"𝔑", "N"}, {"𝔫", "n"}, {"𝔒", "O"}, {"𝔬", "o"}, {"𝔓", "P"}, {"𝔭", "p"}, {"𝔔", "Q"}, {"𝔮", "q"}, {"ℜ", "R"}, {"𝔯", "r"}, {"𝔖", "S"}, {"𝔰", "s"}, {"𝔗", "T"}, {"𝔱", "t"}, {"𝔘", "U"}, {"𝔲", "u"}, {"𝔙", "V"}, {"𝔳", "v"}, {"𝔚", "W"}, {"𝔴", "w"}, {"𝔛", "X"}, {"𝔵", "x"}, {"𝔜", "Y"}, {"𝔶", "y"}, {"ℨ", "Z"}, {"𝔷", "z"},

    //Mathematical Double-Struck
    {"𝔸", "A"}, {"𝕒", "a"}, {"𝔹", "B"}, {"𝕓", "b"}, {"ℂ", "C"}, {"𝕔", "c"}, {"𝔻", "D"}, {"𝕕", "d"}, {"𝔼", "E"}, {"𝕖", "e"}, {"𝔽", "F"}, {"𝕗", "f"}, {"𝔾", "G"}, {"𝕘", "g"}, {"ℍ", "H"}, {"𝕙", "h"}, {"𝕀", "I"}, {"𝕚", "i"}, {"𝕁", "J"}, {"𝕛", "j"}, {"𝕂", "K"}, {"𝕜", "k"}, {"𝕃", "L"}, {"𝕝", "l"}, {"𝕄", "M"}, {"𝕞", "m"}, {"ℕ", "N"}, {"𝕟", "n"}, {"𝕆", "O"}, {"𝕠", "o"}, {"ℙ", "P"}, {"𝕡", "p"}, {"ℚ", "Q"}, {"𝕢", "q"}, {"ℝ", "R"}, {"𝕣", "r"}, {"𝕊", "S"}, {"𝕤", "s"}, {"𝕋", "T"}, {"𝕥", "t"}, {"𝕌", "U"}, {"𝕦", "u"}, {"𝕍", "V"}, {"𝕧", "v"}, {"𝕎", "W"}, {"𝕨", "w"}, {"𝕏", "X"}, {"𝕩", "x"}, {"𝕐", "Y"}, {"𝕪", "y"}, {"ℤ", "Z"}, {"𝕫", "z"},

    //Sans-Serif Bold
    {"𝗔", "A"}, {"𝗮", "a"}, {"𝗕", "B"}, {"𝗯", "b"}, {"𝗖", "C"}, {"𝗰", "c"}, {"𝗗", "D"}, {"𝗱", "d"}, {"𝗘", "E"}, {"𝗲", "e"}, {"𝗙", "F"}, {"𝗳", "f"}, {"𝗚", "G"}, {"𝗴", "g"}, {"𝗛", "H"}, {"𝗵", "h"}, {"𝗜", "I"}, {"𝗶", "i"}, {"𝗝", "J"}, {"𝗷", "j"}, {"𝗞", "K"}, {"𝗸", "k"}, {"𝗟", "L"}, {"𝗹", "l"}, {"𝗠", "M"}, {"𝗺", "m"}, {"𝗡", "N"}, {"𝗻", "n"}, {"𝗢", "O"}, {"𝗼", "o"}, {"𝗣", "P"}, {"𝗽", "p"}, {"𝗤", "Q"}, {"𝗾", "q"}, {"𝗥", "R"}, {"𝗿", "r"}, {"𝗦", "S"}, {"𝘀", "s"}, {"𝗧", "T"}, {"𝘁", "t"}, {"𝗨", "U"}, {"𝘂", "u"}, {"𝗩", "V"}, {"𝘃", "v"}, {"𝗪", "W"}, {"𝘄", "w"}, {"𝗫", "X"}, {"𝘅", "x"}, {"𝗬", "Y"}, {"𝘆", "y"}, {"𝗭", "Z"}, {"𝘇", "z"},

    //"Bold Italic Sans-Serif": {
    {"𝘈", "A"}, {"𝘢", "a"}, {"𝘽", "B"}, {"𝘣", "b"}, {"𝘾", "C"}, {"𝘤", "c"}, {"𝘋", "D"}, {"𝘥", "d"}, {"𝙀", "E"}, {"𝘦", "e"}, {"𝘍", "F"}, {"𝘧", "f"}, {"𝘎", "G"}, {"𝘨", "g"}, {"𝘏", "H"}, {"𝘩", "h"}, {"𝘐", "I"}, {"𝘪", "i"}, {"𝘑", "J"}, {"𝘫", "j"}, {"𝘒", "K"}, {"𝘬", "k"}, {"𝙇", "L"}, {"𝘭", "l"}, {"𝘔", "M"}, {"𝘮", "m"}, {"𝙉", "N"}, {"𝘯", "n"}, {"𝘖", "O"}, {"𝘰", "o"}, {"𝙋", "P"}, {"𝘱", "p"}, {"𝘘", "Q"}, {"𝘲", "q"}, {"𝙍", "R"}, {"𝘳", "r"}, {"𝙎", "S"}, {"𝘴", "s"}, {"𝙏", "T"}, {"𝘵", "t"}, {"𝙐", "U"}, {"𝘶", "u"}, {"𝙑", "V"}, {"𝘷", "v"}, {"𝘞", "W"}, {"𝘸", "w"}, {"𝘟", "X"}, {"𝘹", "x"}, {"𝙔", "Y"}, {"𝘺", "y"}, {"𝙕", "Z"}, {"𝘻", "z"},


    //"Italic Sans-Serif": {
    {"𝘼", "A"}, {"𝙖", "a"}, {"𝘽", "B"}, {"𝙗", "b"}, {"𝘾", "C"}, {"𝙘", "c"}, {"𝘿", "D"}, {"𝙙", "d"}, {"𝙀", "E"}, {"𝙚", "e"}, {"𝙁", "F"}, {"𝙛", "f"}, {"𝙂", "G"}, {"𝙜", "g"}, {"𝙃", "H"}, {"𝙝", "h"}, {"𝙄", "I"}, {"𝙞", "i"}, {"𝙅", "J"}, {"𝙟", "j"}, {"𝙆", "K"}, {"𝙠", "k"}, {"𝙇", "L"}, {"𝙡", "l"}, {"𝙈", "M"}, {"𝙢", "m"}, {"𝙉", "N"}, {"𝙣", "n"}, {"𝙊", "O"}, {"𝙤", "o"}, {"𝙋", "P"}, {"𝙥", "p"}, {"𝙌", "Q"}, {"𝙦", "q"}, {"𝙍", "R"}, {"𝙧", "r"}, {"𝙎", "S"}, {"𝙨", "s"}, {"𝙏", "T"}, {"𝙩", "t"}, {"𝙐", "U"}, {"𝙪", "u"}, {"𝙑", "V"}, {"𝙫", "v"}, {"𝙒", "W"}, {"𝙬", "w"}, {"𝙓", "X"}, {"𝙭", "x"}, {"𝙔", "Y"}, {"𝙮", "y"}, {"𝙕", "Z"}, {"𝙯", "z"},
            
    //"Mathematical Bold Italic Sans-Serif": 
    {"𝑨", "A"}, {"𝒂", "a"}, {"𝑩", "B"}, {"𝒃", "b"}, {"𝑪", "C"}, {"𝒄", "c"}, {"𝑫", "D"}, {"𝒅", "d"}, {"𝑬", "E"}, {"𝒆", "e"}, {"𝑭", "F"}, {"𝒇", "f"}, {"𝑮", "G"}, {"𝒈", "g"}, {"𝑯", "H"}, {"𝒉", "h"}, {"𝑰", "I"}, {"𝒊", "i"}, {"𝑱", "J"}, {"𝒋", "j"}, {"𝑲", "K"}, {"𝒌", "k"}, {"𝑳", "L"}, {"𝒍", "l"}, {"𝑴", "M"}, {"𝒎", "m"}, {"𝑵", "N"}, {"𝒏", "n"}, {"𝑶", "O"}, {"𝒐", "o"}, {"𝑷", "P"}, {"𝒑", "p"}, {"𝑸", "Q"}, {"𝒒", "q"}, {"𝑹", "R"}, {"𝒓", "r"}, {"𝑺", "S"}, {"𝒔", "s"}, {"𝑻", "T"}, {"𝒕", "t"}, {"𝑼", "U"}, {"𝒖", "u"}, {"𝑽", "V"}, {"𝒗", "v"}, {"𝑾", "W"}, {"𝒘", "w"}, {"𝑿", "X"}, {"𝒙", "x"}, {"𝒀", "Y"}, {"𝒚", "y"}, {"𝒁", "Z"}, {"𝒛", "z"},     
          
    //Monospace
    {"𝙰", "A"}, {"𝚊", "a"}, {"𝙱", "B"}, {"𝚋", "b"}, {"𝙲", "C"}, {"𝚌", "c"}, {"𝙳", "D"}, {"𝚍", "d"}, {"𝙴", "E"}, {"𝚎", "e"}, {"𝙵", "F"}, {"𝚏", "f"}, {"𝙶", "G"}, {"𝚐", "g"}, {"𝙷", "H"}, {"𝚑", "h"}, {"𝙸", "I"}, {"𝚒", "i"}, {"𝙹", "J"}, {"𝚓", "j"}, {"𝙺", "K"}, {"𝚔", "k"}, {"𝙻", "L"}, {"𝚕", "l"}, {"𝙼", "M"}, {"𝚖", "m"}, {"𝙽", "N"}, {"𝚗", "n"}, {"𝙾", "O"}, {"𝚘", "o"}, {"𝙿", "P"}, {"𝚙", "p"}, {"𝚀", "Q"}, {"𝚚", "q"}, {"𝚁", "R"}, {"𝚛", "r"}, {"𝚂", "S"}, {"𝚜", "s"}, {"𝚃", "T"}, {"𝚝", "t"}, {"𝚄", "U"}, {"𝚞", "u"}, {"𝚅", "V"}, {"𝚟", "v"}, {"𝚆", "W"}, {"𝚠", "w"}, {"𝚇", "X"}, {"𝚡", "x"}, {"𝚈", "Y"}, {"𝚢", "y"}, {"𝚉", "Z"}, {"𝚣", "z"},
    
    //???
    {"Ａ", "A"}, {"Ｂ", "B"}, {"Ｃ", "C"}, {"Ｄ", "D"}, {"Ｅ", "E"}, {"Ｆ", "F"}, {"Ｇ", "G"}, {"Ｈ", "H"}, {"Ｉ", "I"}, {"Ｊ", "J"}, {"Ｋ", "K"}, {"Ｌ", "L"}, {"Ｍ", "M"}, {"Ｎ", "N"}, {"Ｏ", "O"}, {"Ｐ", "P"}, {"Ｑ", "Q"}, {"Ｒ", "R"}, {"Ｓ", "S"}, {"Ｔ", "T"}, {"Ｕ", "U"}, {"Ｖ", "V"}, {"Ｗ", "W"}, {"Ｘ", "X"}, {"Ｙ", "Y"}, {"Ｚ", "Z"},
    {"ａ", "a"}, {"ｂ", "b"}, {"ｃ", "c"}, {"ｄ", "d"}, {"ｅ", "e"}, {"ｆ", "f"}, {"ｇ", "g"}, {"ｈ", "h"}, {"ｉ", "i"}, {"ｊ", "j"}, {"ｋ", "k"}, {"ｌ", "l"}, {"ｍ", "m"}, {"ｎ", "n"}, {"ｏ", "o"}, {"ｐ", "p"}, {"ｑ", "q"}, {"ｒ", "r"}, {"ｓ", "s"}, {"ｔ", "t"}, {"ｕ", "u"}, {"ｖ", "v"}, {"ｗ", "w"}, {"ｘ", "x"}, {"ｙ", "y"}, {"ｚ", "z"},

    //Mathematical Bold Digits 
    {"𝟎", "0"}, {"𝟏", "1"}, {"𝟐", "2"}, {"𝟑", "3"}, {"𝟒", "4"}, {"𝟓", "5"}, {"𝟔", "6"}, {"𝟕", "7"}, {"𝟖", "8"}, {"𝟗", "9"},

    // Double-struck (ℙ𝕝𝕒𝕚𝕟)
    {"𝟘", "0"}, {"𝟙", "1"}, {"𝟚", "2"}, {"𝟛", "3"}, {"𝟜", "4"}, {"𝟝", "5"}, {"𝟞", "6"}, {"𝟟", "7"}, {"𝟠", "8"}, {"𝟡", "9"},

    // Sans-serif (𝖯𝖺𝗍𝗁)
    {"𝟢", "0"}, {"𝟣", "1"}, {"𝟤", "2"}, {"𝟥", "3"}, {"𝟦", "4"}, {"𝟧", "5"}, {"𝟨", "6"}, {"𝟩", "7"}, {"𝟪", "8"}, {"𝟫", "9"},
};

// Characters to remove
const std::vector<std::string> UNWANTED_UNICODE_CHARS = {
    "\xE2\x80\x8F", "\xE2\x81\xA8", "\xE2\x81\xA9", "\xE2\x80\xAE",
    "\xE2\x80\xAD", "\xE2\x80\xAB", "\xE2\x80\xAA", "\xE2\x80\xAC",
    "\xEF\xBB\xBF", "\xE2\x80\x8E"
};

// =============== REPLACEMENT TABLES ===============

// Whole word replacements for all languages
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_ALL = {

};

// Whole word replacements for Arabic
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_ARABIC = {
    {"أ", "ألف"},
    {"ا", "ألف"},
    {"ب", "باء"},
    {"ت", "تاء"},
    {"ث", "ثاء"},
    {"ج", "جيم"},
    {"ح", "حاء"},
    {"خ", "خاء"},
    {"د", "دال"},
    {"ذ", "ذال"},
    {"ر", "راء"},
    {"ز", "زاي"},
    {"س", "سين"},
    {"ش", "شين"},
    {"ص", "صاد"},
    {"ض", "ضاد"},
    {"ط", "طاء"},
    {"ظ", "ظاء"},
    {"ع", "عين"},
    {"غ", "غين"},
    {"ف", "فاء"},
    {"ق", "قاف"},
    {"ک", "كاف"},
    {"ك", "كاف"},
    {"ک", "گاف"},
    {"ل", "لام"},
    {"م", "ميم"},
    {"ن", "نون"},
    {"و", "واو"},
    {"ه", "هاء"},
    {"ة", "تاء مربوطة"},
    {"ی", "ياء"},
    {"ي", "ياء"},
};

// Whole word replacements for English
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_ENGLISH = {
    {"approx", "approximately"},
    {"etc", "et cetera"},
    {"e.g", "for example"},
    {"i.e", "that is"},

    //Honorifics
    {"mrs", "misess"},
    {"mr", "mister"},
    {"dr", "doctor"},
    {"st", "saint"},
    {"co", "company"},
    {"jr", "junior"},
    {"maj", "major"},
    {"gen", "general"},
    {"drs", "doctors"},
    {"rev", "reverend"},
    {"lt", "lieutenant"},
    {"hon", "honorable"},
    {"sgt", "sergeant"},
    {"capt", "captain"},
    {"esq", "esquire"},
    {"ltd", "limited"},
    {"col", "colonel"},
    {"ft", "fort"},

    {"bit/s" , "bits per second"},
    {"kbit/s", "kilo bits per second"},
    {"mbit/s", "mega bits per second"},
    {"gbit/s", "giga bits per second"},
    {"tbit/s", "tera bits per second"},
   
    {"b/s" , "bits per second"},
    {"kb/s", "kilo bits per second"},
    {"mb/s", "mega bits per second"},
    {"gb/s", "giga bits per second"},
    {"tb/s", "tera bits per second"},

    {"bps" , "bits per second"},
    {"kbps", "kilo bits per second"},
    {"mbps", "mega bits per second"},
    {"gbps", "giga bits per second"},
    {"tbps", "tera bits per second"},

    //{"am", "A M"},
    {"pm", "p m"},

    {"c++", "c plus plus"},
    {"c#", "c sharp"},

    // File Extensions
    {"txt" , " t x t"},
    {"doc" , " duck"},
    {"docx", " duck x"},
    {"pdf" , " p d f"},
    {"rtf" , " r t f"},
    {"odt" , " o d t"},
    {"ppt" , " p p t"},
    {"pptx", " p p t x"},
    {"xls" , " x l s"},
    {"xlsx", " x l s x"},
    {"csv" , " c s v"},
    {"jpg" , " j p g"},
    {"jpeg", " j peg"},
    {"png" , " p n g"},
    {"bmp" , " b m p"},
    {"svg" , " s v g"},
    {"webp", " web p"},
    {"aac" , " a a c"},
    {"ogg" , " o g g"},
    {"m4a" , " m 4 a"},
    {"mp3" , " m p 3"},
    {"mp4" , " m p 4"},
    {"avi" , " a v i"},
    {"mkv" , " m k v"},
    {"wmv" , " w m v"},
    {"flv" , " f l v"},
    {"webm", " web m"},
    {"7z"  , " seven zip"},
    {"gz"  , " g z"},
    {"msi" , " m s i"},
    {"dll" , " d l l"},
    {"sh"  , " s h"},
    {"apk" , " a p k"},
    {"html", " h t m l"},
    {"htm" , " h t m"},
    {"css" , " c s s"},
    {"js"  , " j s"},
    {"cpp" , " c p p"},
    {"php" , " p h p"},
    {"json", " j son"},
    {"xml" , " x m l"},
    {"sql" , " s q l"},
    {"db"  , " d b"},
    {"mdb" , " m d b"},
    {"sqlite", " s q lite"},
    {"accdb", " a c c d b"},

    {"qwerty", " q verty"},
    
    //Internet
    {"http", "h t t p"},
    {"https", "h t t p s"},
    {"www", "w w w"},
    {"ftp", "f t p"},
};

// Whole word replacements for Persian
static const std::unordered_map<std::string, std::string> WHOLE_WORD_REPLACEMENTS_PERSIAN = {
    //{"ص", "صفحه"},
    //{"ش", "شماره"},
    //{"ج", "جلد"},
    //{"ک", "کوچه"},
    //{"خ", "خیابان"},
};

// Normal replacements (anywhere in string) for all languages
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_ALL = {
    {"ﷻ", "جل جلاله"},
    {"(ص)", "صلى الله عليه و آله"},
    {"(ع)", "علیه السلام"},

    // Lowercase Greek
    {"α", "alpha"}, {"ά", "alpha"}, {"β", "beta"}, {"γ", "gamma"}, {"δ", "delta"}, {"ε", "epsilon"}, {"έ", "epsilon"}, {"ζ", "zeta"}, {"η", "eta"}, {"ή", "eta"}, {"θ", "theta"}, {"ι", "iota"}, {"ί", "iota"}, {"κ", "kappa"}, {"λ", "lambda"}, {"μ", "mu"}, {"ν", "nu"}, {"ξ", "xi"}, {"ο", "omicron"}, {"ό", "omicron"}, {"π", "pi"}, {"ρ", "rho"}, {"σ", "sigma"}, {"ς", "sigma"}, {"τ", "tau"}, {"υ", "upsilon"}, {"ύ", "upsilon"}, {"φ", "phi"}, {"χ", "chi"}, {"ψ", "psi"}, {"ω", "omega"}, {"ώ", "omega"},

    // Uppercase Greek
    {"Α", "Alpha"}, {"Ά", "Alpha"}, {"Β", "Beta"}, {"Γ", "Gamma"}, {"Δ", "Delta"}, {"Ε", "Epsilon"}, {"Έ", "Epsilon"}, {"Ζ", "Zeta"}, {"Η", "Eta"}, {"Ή", "Eta"}, {"Θ", "Theta"}, {"Ι", "Iota"}, {"Ί", "Iota"}, {"Κ", "Kappa"}, {"Λ", "Lambda"}, {"Μ", "Mu"}, {"Ν", "Nu"}, {"Ξ", "Xi"}, {"Ο", "Omicron"}, {"Ό", "Omicron"}, {"Π", "Pi"}, {"Ρ", "Rho"}, {"Σ", "Sigma"}, {"Τ", "Tau"}, {"Υ", "Upsilon"}, {"Ύ", "Upsilon"}, {"Φ", "Phi"}, {"Χ", "Chi"}, {"Ψ", "Psi"}, {"Ω", "Omega"}, {"Ώ", "Omega"},
};

// Normal replacements for Arabic
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_ARABIC = {
    {"@", "في"},
    {"#", "شباك"},
    {"%", "بالمئة"},
    {"^", "مرفوع للقوة"},
    {"&", "و"},
    {"*", "مضروب في"},
    {"+", "زائد"},
    {"/", "مقسوم على"},
    {"\\", "بَک اِسلَش"},
    {"=", "يساوي"},
};

// Normal replacements for English
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_ENGLISH = {
    {"@", "at sign"},
    {"#", "hash"},
    {"%", "percent"},
    {"^", "power"},
    {"&", "and"},
    {"*", "multiply"},
    {"+", "plus"},
    {"/", "slash"},
    {"\\", "backslash"},
    {"=", "equals"},

    {"w/", "with"},
    {"w/o", "without"},
    {"b/c", "because"},
};

// Normal replacements for Persian
static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_PERSIAN_NO_SPACE = {
    {"ۀ", "ه ی"},
    {"ة", "ت"},
    {"ك", "ک"},
};

static const std::unordered_map<std::string, std::string> NORMAL_REPLACEMENTS_PERSIAN = {
    {"@", "اَت ساین"},
    {"#", "هَش تگ"},
    {"%", "درصد"},
    {"^", "به توان"},
    {"&", "اَند"},
    {"*", "ضرب در"},
    {"+", "به علاوه ی"},
    {"/", "تقسیم بر"},
    {"\\", "بَک اِسلَش"},
    {"=", "مساوی"},  
};

void doArabicSpecificReplacements(std::string &segment_text)
{
    // Matches: number → whitespace → "ص" → followed by non-Arabic OR end-of-string
    std::regex pattern1(R"((\d+\s)ص([^\wء-ي]|$))");  // [^\wء-ي] = not a word char or Arabic letter
    std::string replacement1 = "$1صباحاً ";
    segment_text = std::regex_replace(segment_text, pattern1, replacement1);

    // Matches: number → whitespace → "م" → followed by non-Arabic OR end-of-string
    std::regex pattern2(R"((\d+\s)م([^\wء-ي]|$))");  // [^\wء-ي] = not a word char or Arabic letter
    std::string replacement2 = "$1مساءً ";
    segment_text = std::regex_replace(segment_text, pattern2, replacement2);
}

// =============== HELPER FUNCTIONS ===============
std::vector<std::string> split(const std::string& s, char delimiter);
std::string join(const std::vector<std::string>& vec, char delimiter);
std::string toLower(const std::string& str);
std::string factorizeChineseLetters(const std::string& input);
bool is_cjk(uint32_t cp);
std::string replace_cjk_with_placeholder(const std::string& input);
void applyWholeWordReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements);
void applyWholeWordReplacementsArabic(std::string& result, const std::unordered_map<std::string, std::string>& replacements);
void applyNormalReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements);
void applyNormalReplacementsWithSpace(std::string& result, const std::unordered_map<std::string, std::string>& replacements);

// =============== MAIN REPLACEMENT FUNCTION ===============

std::string performReplacements(Language mainlang, const std::string& input) {
    std::string result = input;

    // Remove unwanted characters
    for (const auto& seq : UNWANTED_UNICODE_CHARS) {
        size_t pos = 0;
        while ((pos = result.find(seq, pos)) != std::string::npos) {
            result.erase(pos, seq.length());
        }
    }

    // Replace Arabic numbers and different forms of english letters to simple english
    applyNormalReplacements(result, NORMAL_REPLACEMENTS_NO_SPACE_ALL);

    // Apply universal normal replacements
    applyNormalReplacementsWithSpace(result, NORMAL_REPLACEMENTS_ALL);

    // Apply universal whole word replacements
    applyWholeWordReplacements(result, WHOLE_WORD_REPLACEMENTS_ALL);

    // Byte unit replacements
    result = std::regex_replace(result, std::regex("(^|\\s)B(\\b|\\s)"), "$1Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])B(\\b|\\s)"), "$1 Byte$2");
    result = std::regex_replace(result, std::regex("(^|\\s)KB(\\b|\\s)"), "$1Kilo Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])KB(\\b|\\s)"), "$1 Kilo Byte$2");
    result = std::regex_replace(result, std::regex("(^|\\s)MB(\\b|\\s)"), "$1Mega Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])MB(\\b|\\s)"), "$1 Mega Byte$2");
    result = std::regex_replace(result, std::regex("(^|\\s)GB(\\b|\\s)"), "$1Giga Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])GB(\\b|\\s)"), "$1 Giga Byte$2");
    result = std::regex_replace(result, std::regex("(^|\\s)TB(\\b|\\s)"), "$1Tera Byte$2");
    result = std::regex_replace(result, std::regex("([0-9])TB(\\b|\\s)"), "$1 Tera Byte$2");

    result = std::regex_replace(result, std::regex("(^|\\s)b(\\b|\\s)"), "$1bit$2");
    result = std::regex_replace(result, std::regex("([0-9])b(\\b|\\s)"), "$1 bit$2");
    result = std::regex_replace(result, std::regex("(^|\\s)Kb(\\b|\\s)"), "$1Kilo bit$2");
    result = std::regex_replace(result, std::regex("([0-9])Kb(\\b|\\s)"), "$1 Kilo bit$2");
    result = std::regex_replace(result, std::regex("(^|\\s)Mb(\\b|\\s)"), "$1Mega bit$2");
    result = std::regex_replace(result, std::regex("([0-9])Mb(\\b|\\s)"), "$1 Mega bit$2");
    result = std::regex_replace(result, std::regex("(^|\\s)Gb(\\b|\\s)"), "$1Giga bit$2");
    result = std::regex_replace(result, std::regex("([0-9])Gb(\\b|\\s)"), "$1 Giga bit$2");
    result = std::regex_replace(result, std::regex("(^|\\s)Tb(\\b|\\s)"), "$1Tera bit$2");
    result = std::regex_replace(result, std::regex("([0-9])Tb(\\b|\\s)"), "$1 Tera bit$2");

    // Handle camelCase and abbreviations
    // "doTheABC" -> "do The ABC"
    result = std::regex_replace(result, std::regex("([a-z])([A-Z])"), "$1 $2");
    result = std::regex_replace(result, std::regex("([a-z])([A-Z]{2,})(\\b|$)"), "$1 $2$3");

    // Process capital sequences (2-4 letters only, longer sequences stay unchanged)
    // "ABCD" -> "A B C D"

    std::regex capitalLetters(R"(\b([A-Z]{2,4})(?![A-Z]))");  // Uses word boundary instead of lookbehind
    std::string temp;
    std::sregex_iterator it(result.begin(), result.end(), capitalLetters);
    std::sregex_iterator end;
    size_t last_pos = 0;

    for (; it != end; ++it) {
        temp += result.substr(last_pos, it->position() - last_pos);
        std::string letters = (*it)[1].str();
        for (size_t i = 0; i < letters.size(); ++i) {
            if (i != 0) temp += " ";
            temp += letters[i];
        }
        last_pos = it->position() + it->length();
    }
    temp += result.substr(last_pos);
    result = temp;

    // Remove number separators
    // 204:20 -> 04 20
    bool changed;
    do {
        changed = false;
        std::regex slashBetweenNumbers("([0-9]+)/([0-9]+)");
        std::string new_result = std::regex_replace(result, slashBetweenNumbers, "$1 $2");
        if (new_result != result) {
            changed = true;
            result = new_result;
        }
    } while (changed);

    // 02/28/2025 -> 02 28 2025
    do {
        changed = false;
        std::regex colonBetweenNumbers("([0-9]+):([0-9]+)");
        std::string new_result = std::regex_replace(result, colonBetweenNumbers, "$1 $2");
        if (new_result != result) {
            changed = true;
            result = new_result;
        }
    } while (changed);

    // CJK character replacement
    // "指事"-> "chinese letter  chinese letter"
    result = replace_cjk_with_placeholder(result);

    // Factorize Chinese letters
    // " chinese letter  chinese letter " -> "2 chinese letters"
    result = factorizeChineseLetters(result);

    // Main Language specific
    switch (mainlang) {
        case Language::ARABIC:
        // 🔑 -> مفتاح
        applyNormalReplacementsWithSpace(result, ARABIC_SYMBOL_REPLACEMENTS);
        break;

        case Language::PERSIAN:
        // 🔑 -> کلید
        applyNormalReplacementsWithSpace(result, PERSIAN_SYMBOL_REPLACEMENTS);
        break;

        default:
        case Language::ENGLISH:
        // 🔑 -> key
        //applyNormalReplacementsWithSpace(result, PERSIAN_SYMBOL_REPLACEMENTS);
        break;
    }

    // CONVERT TO LOWERCASE ///////////////////////////////////
    // ABCDEFG -> abcdefg
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });

    // Language-specific processing
    LanguageDetector detector(mainlang);

    std::vector<DetectedSegment> text_segments = detector.detect_segments(result);
    
    for(auto& text_segment : text_segments) {
        auto &segment_text = text_segment.text;
        auto &language = text_segment.language;

        switch (language) {
            case Language::ARABIC:
            // 8:54 م ->
            //   8:54 مساءً
            if(mainlang == Language::ARABIC) doArabicSpecificReplacements(segment_text); // Should be before ArabicNumberConverter and applyWholeWordReplacements, Causes Artifacts if called on english(?)
            // @ -> فی
            applyNormalReplacementsWithSpace(segment_text, NORMAL_REPLACEMENTS_ARABIC);
            // ا -> الف
            applyWholeWordReplacements(segment_text, WHOLE_WORD_REPLACEMENTS_ARABIC);

            if(mainlang != Language::PERSIAN) //Persian users don't work with arabic numbers
                segment_text = ArabicNumberConverter::normalize_text(segment_text);
            else
                segment_text = PersianNumberConverter::normalize_text(segment_text);
            break;

            case Language::PERSIAN:
            // ة
            applyNormalReplacements(segment_text, NORMAL_REPLACEMENTS_PERSIAN_NO_SPACE);
            // @
            applyNormalReplacementsWithSpace(segment_text, NORMAL_REPLACEMENTS_PERSIAN);
            // Empty
            applyWholeWordReplacements(segment_text, WHOLE_WORD_REPLACEMENTS_PERSIAN);

            segment_text = PersianNumberConverter::normalize_text(segment_text);
            break;

            default:
            case Language::ENGLISH:
            applyNormalReplacementsWithSpace(segment_text, NORMAL_REPLACEMENTS_ENGLISH);
            // @
            applyWholeWordReplacements(segment_text, WHOLE_WORD_REPLACEMENTS_ENGLISH);

            segment_text = EnglishNumberConverter::normalize_text(segment_text);
            break;
        }
    }


    result = LanguageDetector::reunite_segments(text_segments);

    return result;
}

// =============== HELPER FUNCTIONS ===============

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string join(const std::vector<std::string>& vec, char delimiter) {
    std::string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) result += delimiter;
        result += vec[i];
    }
    return result;
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return lowerStr;
}

std::string factorizeChineseLetters(const std::string& input) {
    std::string result;
    size_t pos = 0;
    const std::string pattern = " chinese letter ";
    const size_t pattern_length = pattern.length();
    
    while (pos < input.length()) {
        size_t found = input.find(pattern, pos);
        if (found == std::string::npos) {
            result += input.substr(pos);
            break;
        }
        
        result += input.substr(pos, found - pos);
        size_t count = 0;
        
        while (found != std::string::npos && 
               input.substr(found, pattern_length) == pattern) {
            count++;
            found += pattern_length;
        }
        
        if (count > 1) {
            result += " " + std::to_string(count) + " chinese letters ";
        } else {
            result += pattern;
        }
        
        pos = found;
    }
    
    return result;
}

void applyWholeWordReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements) {
    for (const auto& pair : replacements) {
        std::regex pattern("\\b" + pair.first + "\\b");
        result = std::regex_replace(result, pattern, pair.second);
    }
}

void applyWholeWordReplacementsArabic(std::string& result, const std::unordered_map<std::string, std::string>& replacements) {
    for (const auto& pair : replacements) {
        std::regex pattern("\\b" + pair.first + "\\b");
        result = std::regex_replace(result, pattern, pair.second);
    }
}

void applyNormalReplacements(std::string& result, const std::unordered_map<std::string, std::string>& replacements) {
    for (const auto& pair : replacements) {
        size_t pos = 0;
        while ((pos = result.find(pair.first, pos)) != std::string::npos) {
            result.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length();
        }
    }
}

void applyNormalReplacementsWithSpace(std::string& result, const std::unordered_map<std::string, std::string>& replacements) {
    for (const auto& pair : replacements) {
        size_t pos = 0;
        while ((pos = result.find(pair.first, pos)) != std::string::npos) {
            // Always add space before and after
            std::string replacement = " " + pair.second + " ";
            
            // Perform the replacement
            result.replace(pos, pair.first.length(), replacement);
            
            // Move position forward (skip past the replacement + spaces)
            pos += replacement.length();
        }
    }
}

bool is_cjk(uint32_t cp) {
    return (cp >= 0x4E00 && cp <= 0x9FFF);
}

std::string replace_cjk_with_placeholder(const std::string& input) {
    std::string output;
    size_t i = 0;
    while (i < input.size()) {
        unsigned char c = input[i];
        uint32_t codepoint = 0;
        int bytes = 0;

        // Decode UTF-8 to codepoint
        if (c < 0x80) {
            codepoint = c;
            bytes = 1;
        } else if ((c & 0xE0) == 0xC0) {
            codepoint = ((c & 0x1F) << 6) | (input[i + 1] & 0x3F);
            bytes = 2;
        } else if ((c & 0xF0) == 0xE0) {
            codepoint = ((c & 0x0F) << 12) |
                        ((input[i + 1] & 0x3F) << 6) |
                        (input[i + 2] & 0x3F);
            bytes = 3;
        } else if ((c & 0xF8) == 0xF0) {
            codepoint = ((c & 0x07) << 18) |
                        ((input[i + 1] & 0x3F) << 12) |
                        ((input[i + 2] & 0x3F) << 6) |
                        (input[i + 3] & 0x3F);
            bytes = 4;
        }

        if (is_cjk(codepoint)) {
            output += " chinese letter ";
        } else {
            output += input.substr(i, bytes);
        }

        i += bytes;
    }

    return output;
}