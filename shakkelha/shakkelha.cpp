#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <onnxruntime_cxx_api.h>
#include <codecvt>
#include <locale>

#ifdef _WIN32
#include <windows.h>
#endif

// Include your mappings here
std::unordered_map<std::wstring, int> RNN_BIG_CHARACTERS_MAPPING = {
    {L"<PAD>", 0}, {L"<SOS>", 1}, {L"<EOS>", 2}, {L"<UNK>", 3}, {L"\n", 4},
    {L" ", 5}, {L"!", 6}, {L"\"", 7}, {L"&", 8}, {L"'", 9}, {L"(", 10},
    {L")", 11}, {L"*", 12}, {L"+", 13}, {L",", 14}, {L"-", 15}, {L".", 16},
    {L"/", 17}, {L"0", 18}, {L"1", 19}, {L"2", 20}, {L"3", 21}, {L"4", 22},
    {L"5", 23}, {L"6", 24}, {L"7", 25}, {L"8", 26}, {L"9", 27}, {L":", 28},
    {L";", 29}, {L"=", 30}, {L"[", 31}, {L"]", 32}, {L"_", 33}, {L"`", 34},
    {L"{", 35}, {L"}", 36}, {L"~", 37}, {L"«", 38}, {L"»", 39}, {L"،", 40},
    {L"؛", 41}, {L"؟", 42}, {L"ء", 43}, {L"آ", 44}, {L"أ", 45}, {L"ؤ", 46},
    {L"إ", 47}, {L"ئ", 48}, {L"ا", 49}, {L"ب", 50}, {L"ة", 51}, {L"ت", 52},
    {L"ث", 53}, {L"ج", 54}, {L"ح", 55}, {L"خ", 56}, {L"د", 57}, {L"ذ", 58},
    {L"ر", 59}, {L"ز", 60}, {L"س", 61}, {L"ش", 62}, {L"ص", 63}, {L"ض", 64},
    {L"ط", 65}, {L"ظ", 66}, {L"ع", 67}, {L"غ", 68}, {L"ف", 69}, {L"ق", 70},
    {L"ك", 71}, {L"ل", 72}, {L"م", 73}, {L"ن", 74}, {L"ه", 75}, {L"و", 76},
    {L"ى", 77}, {L"ي", 78}
};

std::unordered_map<int, std::wstring> RNN_REV_CLASSES_MAPPING = {
    {0, L""}, {1, L"َ"}, {2, L"ً"}, {3, L"ُ"}, {4, L"ٌ"}, {5, L"ِ"},
    {6, L"ٍ"}, {7, L"ْ"}, {8, L"ّ"}, {9, L"َّ"}, {10, L"ًّ"}, {11, L"ُّ"},
    {12, L"ٌّ"}, {13, L"ِّ"}, {14, L"ٍّ"}, {15, L"<PAD>"}, {16, L"<SOS>"},
    {17, L"<EOS>"}, {18, L"<N/A>"}
};

std::wstring remove_diacritics(const std::wstring& input) {
    const std::wstring diacritics = L"ًٌٍَُِّْ";
    std::wstring result;
    for (wchar_t ch : input) {
        if (diacritics.find(ch) == std::wstring::npos)
            result += ch;
    }
    return result;
}

std::vector<int64_t> encode(const std::wstring& input) {
    std::wstring cleaned = remove_diacritics(input);  // 🔥 important
    std::vector<int64_t> encoded = {RNN_BIG_CHARACTERS_MAPPING[L"<SOS>"]};
    for (wchar_t ch : cleaned) {
        std::wstring chStr(1, ch);
        if (RNN_BIG_CHARACTERS_MAPPING.count(chStr))
            encoded.push_back(RNN_BIG_CHARACTERS_MAPPING[chStr]);
        else
            encoded.push_back(RNN_BIG_CHARACTERS_MAPPING[L"<UNK>"]);
    }
    encoded.push_back(RNN_BIG_CHARACTERS_MAPPING[L"<EOS>"]);
    return encoded;
}

std::wstring decode_aligned(
    const std::vector<std::vector<std::vector<float>>>& probs,
    const std::wstring& inputText
) {
    const std::wstring diacritics = L"ًٌٍَُِّْ";
    const std::wstring arabicLetters = L"ءآأؤإئابةتثجحخدذرزسشصضطظعغفقكلمنهوىي";

    std::wstring cleanInput;
    std::vector<size_t> cleanToOrigIndex;

    // Step 1: Build clean version of input and mapping to original positions
    for (size_t i = 0; i < inputText.size(); ++i) {
        wchar_t ch = inputText[i];
        if (diacritics.find(ch) == std::wstring::npos) {
            cleanToOrigIndex.push_back(i);
            cleanInput += ch;
        }
    }

    std::wstring output;
    size_t modelIndex = 1; // Start after <SOS>

    for (size_t i = 0; i < cleanToOrigIndex.size(); ++i) {
        size_t origIndex = cleanToOrigIndex[i];
        wchar_t ch = inputText[origIndex];
        output += ch;

        // Collect all diacritics immediately following this base letter
        bool hasOriginalDiacritic = false;
        size_t j = origIndex + 1;
        while (j < inputText.size() && diacritics.find(inputText[j]) != std::wstring::npos) {
            output += inputText[j];
            hasOriginalDiacritic = true;
            ++j;
        }

        if (!hasOriginalDiacritic) {
            // Use model prediction
            const auto& prob = probs[0][modelIndex];
            int maxIdx = std::distance(prob.begin(), std::max_element(prob.begin(), prob.end()));
            const std::wstring& predicted = RNN_REV_CLASSES_MAPPING[maxIdx];
            if (predicted.find(L"<") == std::wstring::npos)
                output += predicted;
        }

        ++modelIndex;
    }

    return output;
}

std::string wstring_to_string(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

static std::unique_ptr<Ort::Env> env;
static std::unique_ptr<Ort::Session> session;
static bool initialized = false;

int shakkelha_initialize(const std::string model_address) {
    if (initialized) {
        std::cerr << "Warning: Model already initialized." << std::endl;
        return 0; // or an error code if reinitialization is not allowed
    }

    try {
        // Initialize ONNX Runtime environment
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "shakkelha");

        // Try to load the model (throws if file not found or invalid)
        Ort::SessionOptions session_options;
        session = std::make_unique<Ort::Session>(*env, model_address.c_str(), session_options);

        initialized = true; // Only set if model loads successfully
        return 0; // Success
    } 
    catch (const std::exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        initialized = false; // Ensure it remains false on failure
        return -1; // Error code
    }
}

std::wstring shakkelha_diacritize(const std::wstring& input) {

    if(!initialized) {
        std::wstring input = L"نَمُوذَجُ شَكِّلْهَا لَمْ يُوجَد";
        return input;
    }

    std::vector<int64_t> input_ids = encode(input);
    std::array<int64_t, 2> input_shape = {1, static_cast<int64_t>(input_ids.size())};

    Ort::AllocatorWithDefaultOptions allocator;
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<int64_t>(
        memory_info, input_ids.data(), input_ids.size(), input_shape.data(), 2);

    const char* input_names[] = {"input"};
    const char* output_names[] = {"output"};
    auto output_tensors = session->Run(Ort::RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, 1);

    float* output_data = output_tensors.front().GetTensorMutableData<float>();
    auto output_shape = output_tensors.front().GetTensorTypeAndShapeInfo().GetShape();
    size_t timesteps = output_shape[1];
    size_t classes = output_shape[2];

    std::vector<std::vector<std::vector<float>>> probs(1, std::vector<std::vector<float>>(timesteps, std::vector<float>(classes)));
    for (size_t t = 0; t < timesteps; ++t)
        for (size_t c = 0; c < classes; ++c)
            probs[0][t][c] = output_data[t * classes + c];

    std::wstring result = decode_aligned(probs, input);
    //std::cout << "Diacritized: " << wstring_to_string(result) << std::endl;

    return result;
}

std::string shakkelha_diacritize(const std::string& input) {
    std::wstring winput = string_to_wstring(input);
    std::wstring woutput = shakkelha_diacritize(winput);
    return wstring_to_string(woutput);
}

#if 0

std::vector<std::wstring> textExamples = {
    L"اَلسَّلامُ عَلَيكُم يَا صَدِيقِي.",
    L"مرحبا بكم",
    L"بسم الله الرحمن الرحیم",
    L"بِسِم اللَّهَ الَّرِحمن الرحیم",
    L"وَرَجَّحَ التَّقْرِيرُ الَّذِي أَعَدَّهُ مَعْهَدُ أَبْحَاثِ هَضَبَةِ التِّبِتِ فِي الْأَكَادِيمِيَّةِ الصِّينِيَّةِ لِلْعُلُومِ - أَنْ تَسْتَمِرَّ دَرَجَاتُ الْحَرَارَةِ وَمُسْتَوَيَاتُ الرُّطُوبَةِ فِي الْإِرْتِفَاعِ طَوَالَ هَذَا الْقَرْنْ",
    L"ورجح التقرير الذي أعده معهد أبحاث هضبة التبت في الأكاديمية الصينية للعلوم أن تستمر درجات الحرارة ومستويات الرطوبة في الارتفاع طوال هذا القرن",
};

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::wstring textExample = textExamples[0];

    int err = shakkelha_initialize("../shakkelha.onnx");
    if(err!=0) {
        std::cout << "Model not found!" << std::endl;
        return 0;
    }

    std::wstring diacritizedText = shakkelha_diacritize(textExample);
    std::cout << "Diacritized: " << wstring_to_string(diacritizedText) << std::endl;

    std::string diacritizedText2 = shakkelha_diacritize(wstring_to_string(textExample));
    std::cout << "Diacritized2: " << diacritizedText2 << std::endl;


    return 0;
}
#endif

