#ifdef _WIN32
#include <windows.h>
#endif

#include <vector>
#include <string>
#include <iostream>

#include "./shakkelha.h"

using namespace std;

vector<string> textExamples = {
    "اَلسَّلامُ عَلَيكُم يَا صَدِيقِي.",
    "أتَاحَتْ لِلبَائِعِ المُتَجَوِّلِ أنْ يَكُونَ جَاذِباً لِلمُوَاطِنِ الأقَلِّ دَخْلاً",
    "أَحْرَزَتْ مُنْتَخَبَاتُ البَرَازِيلِ وَأَلمَانِيَا وَرُوسِيَا فَوْزاً فِي مُقَابَلَاتِهِم الإِعْدَادِيَّةِ الَّتِي أُقِيمَتْ اِسْتِعْدَاداً لِنِهَائِيَاتِ كَأْسِ العَالَم اَلَّتِي سَتَنْطَلِقُ بَعْدَ أَقَلِّ مِنْ أُسْبُوع",
    "أَخْفَقَ مَجْلِسُ النُّوَّابِ اللُّبْنَانِيُّ فِي اِخْتِيَارِ رَئِيسٍ جَدِيدٍ لِلبِلَادِ خَلَفاً لِلرَّئِيسِ الحَالِيِّ الَّذِي تَنْتَهِي وِلَايَتُهُ فِي الخَامِسِ وَالعِشْرِينْ مِنْ مَايُو أَيَارَ المُقْبِل",
};

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif

    string textExample = textExamples[0];

    int err = shakkelha_initialize("./shakkelha.onnx");
    if(err!=0) {
        std::cout << "Model not found!" << std::endl;
        return 0;
    }

    string diacritizedText = shakkelha_diacritize(textExample);
    std::cout << "Diacritized: " << diacritizedText << std::endl;
}