//Test by:
// g++ language_detector_test.cpp language_detector.cpp -o test && ./test

#include "language_detector.h"
#include <iostream>
#include <vector>

void print_tags(const std::vector<Language> &tags) {
    for (Language tag : tags) {
        std::cout << LanguageDetector::language_to_string(tag) << " ";
    }
    std::cout << std::endl;
}

void print_segments(const std::vector<DetectedSegment> &segments) {
    for (const auto &seg : segments) {
        std::cout << "[" << LanguageDetector::language_to_string(seg.language) << ": \"" << seg.text << "\"] ";
    }
    std::cout << std::endl;
}

void run_test_case(const std::string &input, Language main_lang, const std::string &description) {
    std::cout << "=== Test Case: " << description << " ===" << std::endl;
    std::cout << "Input: \"" << input << "\"" << std::endl;
    std::cout << "Main Language: " << LanguageDetector::language_to_string(main_lang) << std::endl;
    
    LanguageDetector detector(main_lang);
    std::vector<Language> tags = detector.detect(input);
    std::vector<DetectedSegment> segments = detector.detect_segments(input);
    
    std::cout << "Raw Tags: ";
    print_tags(tags);
    std::cout << "Segments: ";
    print_segments(segments);
    std::cout << std::endl;
}

int main() {
    // Test all combinations by cycling main languages for each test case
    
    std::locale::global(std::locale("en_US.UTF-8"));

    // Original: Pure Persian
    run_test_case("سلام دنیا!", Language::PERSIAN, "Pure Persian (FA context)");
    run_test_case("سلام دنیا!", Language::ARABIC, "Persian text in Arabic context");
    run_test_case("سلام دنیا!", Language::ENGLISH, "Persian text in English context");

    // Original: English in Persian context
    run_test_case("Hello world!", Language::PERSIAN, "English in Persian context");
    run_test_case("Hello world!", Language::ARABIC, "English in Arabic context");
    run_test_case("Hello world!", Language::ENGLISH, "English in English context");

    // Original: Mixed Persian-English
    run_test_case("این یک test مخلوط است", Language::PERSIAN, "Mixed FA-EN (FA context)");
    run_test_case("این یک test مخلوط است", Language::ARABIC, "Mixed FA-EN (AR context)");
    run_test_case("این یک test مخلوط است", Language::ENGLISH, "Mixed FA-EN (EN context)");

    // Original: Arabic in Persian context
    run_test_case("السلام علیکم", Language::PERSIAN, "Arabic in Persian context");
    run_test_case("السلام علیکم", Language::ARABIC, "Arabic in Arabic context");
    run_test_case("السلام علیکم", Language::ENGLISH, "Arabic in English context");

    // Original: Numbers in Persian
    run_test_case("عدد 123", Language::PERSIAN, "Numbers in FA context");
    run_test_case("عدد 123", Language::ARABIC, "Numbers in AR context");
    run_test_case("عدد 123", Language::ENGLISH, "Numbers in EN context");

    // Original: Attached numbers in Persian
    run_test_case("کلمه1 کلمه2", Language::PERSIAN, "Attached numbers (FA context)");
    run_test_case("کلمه1 کلمه2", Language::ARABIC, "Attached numbers (AR context)");
    run_test_case("کلمه1 کلمه2", Language::ENGLISH, "Attached numbers (EN context)");

    // Original: Pure Arabic
    run_test_case("مرحبا بالعالم", Language::ARABIC, "Pure Arabic (AR context)");
    run_test_case("مرحبا بالعالم", Language::PERSIAN, "Arabic in Persian context");
    run_test_case("مرحبا بالعالم", Language::ENGLISH, "Arabic in English context");

    // Original: English in Arabic context
    run_test_case("Hello world!", Language::ARABIC, "English in Arabic context");
    run_test_case("Hello world!", Language::PERSIAN, "English in Persian context");
    run_test_case("Hello world!", Language::ENGLISH, "English in English context");

    // Original: Non-English in English context
    run_test_case("سلام دنیا", Language::ENGLISH, "Persian in English context");
    run_test_case("سلام دنیا", Language::PERSIAN, "Persian in Persian context");
    run_test_case("سلام دنیا", Language::ARABIC, "Persian in Arabic context");

    // Extended: Mixed script without spaces
    run_test_case("Helloالسلام", Language::ENGLISH, "Mixed EN-AR (EN context)");
    run_test_case("Helloالسلام", Language::ARABIC, "Mixed EN-AR (AR context)");
    run_test_case("Helloالسلام", Language::PERSIAN, "Mixed EN-AR (FA context)");

    // Extended: Email with mixed scripts
    run_test_case("example@پارسی.com", Language::ENGLISH, "Email (EN context)");
    run_test_case("example@پارسی.com", Language::PERSIAN, "Email (FA context)");
    run_test_case("example@پارسی.com", Language::ARABIC, "Email (AR context)");

    // Extended: Bidirectional text
    run_test_case("(Hello) دنیا (World)", Language::ENGLISH, "Bidirectional (EN context)");
    run_test_case("(Hello) دنیا (World)", Language::PERSIAN, "Bidirectional (FA context)");
    run_test_case("(Hello) دنیا (World)", Language::ARABIC, "Bidirectional (AR context)");

    // Extended: Numbers with different numeral systems
    run_test_case("Number ٤٥۶۷", Language::ENGLISH, "Mixed numerals (EN context)");
    run_test_case("Number ٤٥۶۷", Language::PERSIAN, "Mixed numerals (FA context)");
    run_test_case("Number ٤٥۶۷", Language::ARABIC, "Mixed numerals (AR context)");

    // Extended: Long texts
    const std::string long_en = "This is a long English text with various linguistic elements...";
    run_test_case(long_en, Language::ENGLISH, "Long EN (EN context)");
    run_test_case(long_en, Language::PERSIAN, "Long EN (FA context)");
    run_test_case(long_en, Language::ARABIC, "Long EN (AR context)");

    const std::string long_fa = "این یک متن طولانی فارسی است که شامل موارد مختلف زبانی می‌باشد...";
    run_test_case(long_fa, Language::PERSIAN, "Long FA (FA context)");
    run_test_case(long_fa, Language::ENGLISH, "Long FA (EN context)");
    run_test_case(long_fa, Language::ARABIC, "Long FA (AR context)");

    const std::string long_ar = "أَمِتْنَا مُهْتَدِينَ غَيْرَ ضَالِّينَ‌َ. پنج";
    run_test_case(long_ar, Language::PERSIAN, "Long FA (FA context)");

    const std::string single_word_arabic = "العلق";
    run_test_case(single_word_arabic, Language::PERSIAN, "SINGLE WORD ARABIC");

    const std::string long_ar2 = "وَ أَسْتَحْمِلُكَ مِنْ‌َ ذُنُوبِي مَا قَدْ بَهَظَنِي حَمْلُهُ‌ُ ، وَ أَسْتَعِينُ بِكَ عَلَى مَا قَدْ فَدَحَنِي ثِقْلُهُ‌. , هشت , ";
    run_test_case(long_ar2, Language::PERSIAN, "Long AR 2 (FA context)");


    return 0;
}
