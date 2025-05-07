// Besm ALLAH
// Converted from https://github.com/nipponjo/tts-arabic-flutter/blob/main/lib/tts/ar_phonemizer.dart
// To test, uncomment main, then:
// g++ -std=c++11 ar_phonemizer.cpp -o ar_phonemizer /dev/null && ./ar_phonemizer

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <chrono>
#include <codecvt>
#include <locale>

#include "./ar_phonemizer.h"
using namespace std;

// Arabic to Buckwalter mapping (UTF-8 aware)
const unordered_map<string, string> arabicToBuckwMap = {
    {"\u0628", "b"},
    {"\u0630", "*"},
    {"\u0637", "T"},
    {"\u0645", "m"},
    {"\u062a", "t"},
    {"\u0631", "r"},
    {"\u0638", "Z"},
    {"\u0646", "n"},
    {"\u062b", "^"},
    {"\u0632", "z"},
    {"\u0639", "E"},
    {"\u0647", "h"},
    {"\u062c", "j"},
    {"\u0633", "s"},
    {"\u063a", "g"},
    {"\u062d", "H"},
    {"\u0642", "q"},
    {"\u0641", "f"},
    {"\u062e", "x"},
    {"\u0635", "S"},
    {"\u0634", "$"},
    {"\u062f", "d"},
    {"\u0636", "D"},
    {"\u0643", "k"},
    {"\u0623", ">"},
    {"\u0621", "'"},
    {"\u0626", "}"},
    {"\u0624", "&"},
    {"\u0625", "<"},
    {"\u0622", "|"},
    {"\u0627", "A"},
    {"\u0649", "Y"},
    {"\u0629", "p"},
    {"\u064a", "y"},
    {"\u0644", "l"},
    {"\u0648", "w"},
    {"\u064b", "F"},
    {"\u064c", "N"},
    {"\u064d", "K"},
    {"\u064e", "a"},
    {"\u064f", "u"},
    {"\u0650", "i"},
    {"\u0651", "~"},
    {"\u0652", "o"},
    {"\u0671", "{"},
    {"\u06cc", "y"}
};

// Helper function to convert Arabic to Buckwalter
string arabToBuckw(const string& arab) {
    string result;
    // Since Arabic characters in UTF-8 can be multiple bytes, we need to process them carefully
    for (size_t i = 0; i < arab.size(); ) {
        int cplen = 1;
        if ((arab[i] & 0xf8) == 0xf0) cplen = 4;
        else if ((arab[i] & 0xf0) == 0xe0) cplen = 3;
        else if ((arab[i] & 0xe0) == 0xc0) cplen = 2;
        if ((i + cplen) > arab.size()) cplen = 1;

        string charStr = arab.substr(i, cplen);
        auto it = arabicToBuckwMap.find(charStr);
        if (it != arabicToBuckwMap.end()) {
            result += it->second;
        } else {
            result += charStr;
        }
        i += cplen;
    }
    return result;
}

const unordered_map<string, string> unambiguousConsonantMap = {
    {"b", "b"}, {"*", "*"}, {"T", "T"}, {"m", "m"},
    {"t", "t"}, {"r", "r"}, {"Z", "Z"}, {"n", "n"},
    {"^", "^"}, {"z", "z"}, {"E", "E"}, {"h", "h"},
    {"j", "j"}, {"s", "s"}, {"g", "g"}, {"H", "H"},
    {"q", "q"}, {"f", "f"}, {"x", "x"}, {"S", "S"},
    {"$", "$"}, {"d", "d"}, {"D", "D"}, {"k", "k"},
    // Hamza
    {">", "<"}, {"'", "<"}, {"}", "<"}, {"&", "<"},
    {"<", "<"}
};

const string diacritics = "oauiFNK~";
const string diacriticsWithoutShadda = "oauiFNK";
const string emphatics = "DSTZgxq";
const string forwardEmphatics = "gx";
const string consonants = "><}&'bt^jHxd*rzs$SDTZEgfqklmnh|";
const string punctuation = ".:,!?";

//Do not use unordered_map, replacement order is important
const map<string, string> preprocessingReplacements = {
    {"AF", "F"},
    {"\u0640", ""},
    {"o", ""},
    {"aA", "A"},
    {"aY", "Y"},
    {" A", " "},
    {"F", "an"},
    {"N", "un"},
    {"K", "in"},
    {"|", ">A"},
    {"i~", "~i"},
    {"a~", "~a"},
    {"u~", "~u"},
    {"Ai", "<i"},
    {"Aa", ">a"},
    {"Au", ">u"}
};

const unordered_map<string, vector<string>> fixedWords = {
    {"h*A", {"hA*A", "hA*a"}},
    {"h*h", {"hA*ihi", "hA*ih"}},
    {"h*An", {"hA*Ani", "hA*An"}},
    {"h&lA'", {"hA<ulA<i", "hA<ulA<"}},
    {"*lk", {"*Alika", "*Alik"}},
    {"k*lk", {"ka*Alika", "ka*Alik"}},
    {"*lkm", {"*Alikum"}},
    {">wl}k", {"<ulA<ika", "<ulA<ik"}},
    {"Th", {"TAha"}},
    {"lkn", {"lAkinna", "lAkin"}},
    {"lknh", {"lAkinnahu"}},
    {"lknhm", {"lAkinnahum"}},
    {"lknk", {"lAkinnaka", "lAkinnaki"}},
    {"lknkm", {"lAkinnakum"}},
    {"lknkmA", {"lAkinnakumA"}},
    {"lknnA", {"lAkinnanA"}},
    {"AlrHmn", {"rraHmAni", "rraHmAn"}},
    {"Allh", {"llAhi", "llAh", "llAhu", "llAha", "llAh", "llA"}},
    {"h*yn", {"hA*ayni", "hA*ayn"}},
    {"nt", {"nit"}},
    {"fydyw", {"vidyU"}},
    {"lndn", {"landun"}}
};

string isFixedWord(const string& word) {
    if (word.empty()) return "";
    
    char lastLetter = word.back();
    string wordConsonants;
    
    // Remove all non-consonant characters from the word
    for (char c : word) {
        string s(1, c);
        if (s.find_first_not_of("h*Ahn'>wl}kmyTtfd") == string::npos) {
            wordConsonants += c;
        }
    }
    
    auto it = fixedWords.find(wordConsonants);
    if (it != fixedWords.end()) {
        const vector<string>& pronunciations = it->second;
        if (pronunciations.size() > 1) {
            for (const string& pronunciation : pronunciations) {
                if (!pronunciation.empty() && pronunciation.back() == lastLetter) {
                    return pronunciation;
                }
            }
        } else if (!pronunciations.empty()) {
            return pronunciations[0];
        }
    }
    
    return "";
}

vector<string> preprocessUtterance(string utterance) {
    // Replace using the translation map
    for (const auto& replacement : preprocessingReplacements) {
        size_t pos = 0;
        while ((pos = utterance.find(replacement.first, pos)) != string::npos) {
            utterance.replace(pos, replacement.first.length(), replacement.second);
            pos += replacement.second.length();
        }
    }

    // Handle Hamza types and vowel modifications
    regex pattern1("^>([^auAw])");
    utterance = regex_replace(utterance, pattern1, ">a$1");
    
    regex pattern2(" >([^auAw ])");
    utterance = regex_replace(utterance, pattern2, " >a$1");
    
    regex pattern3("<([^i])");
    utterance = regex_replace(utterance, pattern3, "<i$1");

    // Add space between punctuation and preceding non-whitespace character
    regex pattern4("(\\S)([.?,!])");
    utterance = regex_replace(utterance, pattern4, "$1 $2");

    // Split the utterance into words
    vector<string> words;
    size_t start = 0;
    size_t end = utterance.find(' ');
    while (end != string::npos) {
        words.push_back(utterance.substr(start, end - start));
        start = end + 1;
        end = utterance.find(' ', start);
    }
    words.push_back(utterance.substr(start));
    
    return words;
}

string processWord(string word) {
    if (word.empty() || punctuation.find(word) != string::npos) {
        return word;
    }

    string fixedRes = isFixedWord(word);
    if (!fixedRes.empty()) {
        return fixedRes;
    }

    bool emphaticContext = false;
    word = "bb" + word + "ee";

    string phones;

    // MAIN LOOP: Iterate over the word characters
    for (size_t index = 2; index < word.length() - 2; index++) {
        char letter_2 = word[index - 2];
        char letter_1 = word[index - 1];
        char letter = word[index];
        char letter1 = word[index + 1];
        char letter2 = word[index + 2];

        // Update emphatic context
        if (consonants.find(letter) != string::npos && emphatics.find(letter) == string::npos) {
            emphaticContext = false;
        }
        if (emphatics.find(letter) != string::npos) {
            emphaticContext = true;
        }
        if (emphatics.find(letter1) != string::npos && forwardEmphatics.find(letter1) == string::npos) {
            emphaticContext = true;
        }

        // Handle unambiguous consonant phones
        string s(1, letter);
        auto it = unambiguousConsonantMap.find(s);
        if (it != unambiguousConsonantMap.end()) {
            phones += it->second;
        }
        // Special handling for 'l' (Lam)
        else if (letter == 'l') {
            if (diacritics.find(letter1) == string::npos &&
                string("AYwyaui").find(letter1) == string::npos &&
                letter2 == '~') {
                // omit
            } else {
                phones += 'l';
            }
        }
        // Shadda doubles the previous letter
        else if (letter == '~' && string("wy").find(letter_1) == string::npos && !phones.empty()) {
            phones += phones.back();
        }
        // Madda (|)
        else if (letter == '|') {
            phones += emphaticContext ? 'A' : '<';
        }
        // Ta' marboota (p)
        else if (letter == 'p') {
            phones += diacritics.find(letter1) != string::npos ? 't' : '\0';
        }
        // Handle vowels and complex cases for Waw and Ya'
        else if (string("AYwyaui").find(letter) != string::npos) {
            if (string("wy").find(letter) != string::npos) {
                if ((diacriticsWithoutShadda + "AY").find(letter1) != string::npos ||
                    (string("wy").find(letter1) != string::npos && 
                     string(diacritics + "Awy").find(letter2) == string::npos) ||
                    (diacriticsWithoutShadda.find(letter_1) != string::npos &&
                     (consonants + "e").find(letter1) != string::npos)) {
                    if (letter == 'w' && letter_1 == 'u' && string("aiAY").find(letter1) == string::npos) {
                        phones += 'U';
                    } else if (letter == 'y' && letter_1 == 'i' && string("auAY").find(letter1) == string::npos) {
                        phones += 'I';
                    } else if (letter == 'w' && letter1 == 'A' && letter2 == 'e') {
                        phones += 'w';
                    } else {
                        phones += letter;
                    }
                } else if (letter1 == '~') {
                    if (letter_1 == 'a' ||
                        (letter == 'w' && string("iy").find(letter_1) != string::npos) ||
                        (letter == 'y' && string("wu").find(letter_1) != string::npos)) {
                        phones += string(1, letter) + string(1, letter);
                    } else {
                        phones += string(1, (letter == 'w' ? 'U' : 'I')) + string(1, letter);
                    }
                } else if ((consonants + "ui").find(letter_1) != string::npos && letter1 == 'e') {
                    phones += letter == 'w' ? 'U' : 'I';
                } else {
                    phones += letter == 'w' ? 'U' : 'I';
                }
            } else if (string("ui").find(letter) != string::npos) {
                phones += letter;
            } else {
                if (letter == 'A' && string("wk").find(letter_1) != string::npos && letter_2 == 'b') {
                    phones += 'a';
                } else if (letter == 'A' && string("ui").find(letter_1) != string::npos) {
                    continue;
                }
                // Waw al jama3a: The Alif after is optional
                else if (letter == 'A' && letter_1 == 'w' && letter1 == 'e') {
                    phones += 'A';
                } else if (string("AY").find(letter) != string::npos && letter1 == 'e') {
                    phones += 'A';
                } else {
                    if (letter == 'a') {
                        phones += 'a';
                    } else {
                        phones += 'A';
                    }
                }
            }
        }
    }

    return phones;
}

string postprocessUtterance(string phonemes) {
    regex pattern1("aA");
    phonemes = regex_replace(phonemes, pattern1, "A");
    
    regex pattern2("iI");
    phonemes = regex_replace(phonemes, pattern2, "I");
    
    regex pattern3("uU");
    phonemes = regex_replace(phonemes, pattern3, "U");
    
    regex pattern4("aa");
    phonemes = regex_replace(phonemes, pattern4, "A");
    
    return phonemes;
}

string phonemize(const string& text) {
    string buckw = arabToBuckw(text);
    vector<string> words = preprocessUtterance(buckw);
    string phonemesStr;
    
    for (const string& word : words) {
        phonemesStr += processWord(word) + " ";
    }
    
    if (!phonemesStr.empty()) {
        phonemesStr.pop_back(); // Remove trailing space
    }
    
    return postprocessUtterance(phonemesStr);
}

const unordered_map<string, string> nawarToIPA = {
    {"<", "ʔ"},
    {"^", "θ"},
    {"j", "d͡ʒ"},
    {"H", "ħ"},
    {"*", "ð"},
    {"$", "ʃ"},
    {"S", "sˤ"},
    {"D", "dˤ"},
    {"T", "tˤ"},
    {"Z", "ðˤ"},
    {"E", "ʕ"},
    {"g", "ɣ"},
    {"U", "uː"},
    {"I", "iː"},
    {"A", "aː"},
    {"y", "j"}
};

string convertNawarToIPA(const string& phonemesStr) {
    string phonemesIPA;
    for (char c : phonemesStr) {
        string s(1, c);
        auto it = nawarToIPA.find(s);
        if (it != nawarToIPA.end()) {
            phonemesIPA += it->second;
        } else {
            phonemesIPA += c;
        }
    }
    return phonemesIPA;
}

string convertArabicToIPA(const string& inputStr) {    
    string phonemesStr = phonemize(inputStr);
    string phonemesIPA = convertNawarToIPA(phonemesStr);

    return phonemesIPA;
}

#if 0
vector<string> textExamples = {
    "اَلسَّلامُ عَلَيكُم يَا صَدِيقِي.",
    "أتَاحَتْ لِلبَائِعِ المُتَجَوِّلِ أنْ يَكُونَ جَاذِباً لِلمُوَاطِنِ الأقَلِّ دَخْلاً",
    "أَحْرَزَتْ مُنْتَخَبَاتُ البَرَازِيلِ وَأَلمَانِيَا وَرُوسِيَا فَوْزاً فِي مُقَابَلَاتِهِم الإِعْدَادِيَّةِ الَّتِي أُقِيمَتْ اِسْتِعْدَاداً لِنِهَائِيَاتِ كَأْسِ العَالَم اَلَّتِي سَتَنْطَلِقُ بَعْدَ أَقَلِّ مِنْ أُسْبُوع",
    "أَخْفَقَ مَجْلِسُ النُّوَّابِ اللُّبْنَانِيُّ فِي اِخْتِيَارِ رَئِيسٍ جَدِيدٍ لِلبِلَادِ خَلَفاً لِلرَّئِيسِ الحَالِيِّ الَّذِي تَنْتَهِي وِلَايَتُهُ فِي الخَامِسِ وَالعِشْرِينْ مِنْ مَايُو أَيَارَ المُقْبِل",

    // The dart version converts these words to buckwalter correctly when they are single, but drops initial hamzah when they are in above sentences
    // The c++ version always converts correctly
    "اِسْتِعْدَاداً", 
    "اَلَّتِي", 
    "اِخْتِيَارِ",

    "أُقِيمَتْ اِسْتِعْدَاداً", //7
    "العَالَم اَلَّتِي",    //8
    "فِي اِخْتِيَارِ",      //9
};

int main() {
    string textExample = textExamples[3];
    
    auto start = chrono::high_resolution_clock::now();
    string phonemesStr = phonemize(textExample);
    
    /*for (int i = 0; i < 1000; i++) {
        phonemesStr = phonemize(textExample);
    }*/
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    string phonemesIPA;
    phonemesIPA = convertNawarToIPA(phonemesStr);
    
    cout << phonemesStr << endl;
    cout << phonemesIPA << endl;
    cout << "Time: " << duration << " ms" << endl;
    
    return 0;
}
#endif