#include "normalize.h"
#include "language_detector.h"
#include "vits2-tokenizer/vits2-tokenizer.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <locale>
#include <algorithm>
#include <chrono>  // For time measurement
#include <iomanip>
#include <sstream>

// Helper functions
void removeAllPipes(std::string& str);
void removeAllSpaces(std::string& str);

static std::string join(const std::vector<std::string>& vec, char delimiter);

int main(int argc, char* argv[]) {
    // Optional: --grain also writes <input>-grain.csv with the tagged phone stream the
    // 16 kHz GrainSpeech model consumes. --symbols <file> additionally maps the phones onto a
    // model symbol table (the app passes its symbols file); without it the raw stream is written.
    bool grainFlag = false;
    std::string symbolsFile;
    std::vector<std::string> positional;
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--grain" || arg == "-g") {
            grainFlag = true;
        } else if (arg == "--symbols" && i + 1 < argc) {
            symbolsFile = argv[++i];
        } else {
            positional.push_back(arg);
        }
    }
    if (positional.size() != 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <main_language: EN, FA, AR> <input_file> [--grain] [--symbols <model-symbols-file>]\n";
        return 1;
    }
    argc = static_cast<int>(positional.size()) + 1;
    argv[1] = const_cast<char*>(positional[0].c_str());
    argv[2] = const_cast<char*>(positional[1].c_str());

    // Set UTF-8 locale
    std::locale::global(std::locale("en_US.UTF-8"));
    
    std::string mainLanguage(argv[1]);
    Language mainlang = LanguageDetector::string_to_language(mainLanguage);

    int ipa_mode = 1;
    //int ipa_mode = 0; //RAW
    //if(strcmp(argv[2], "IPA") == 0)
    //    ipa_mode = 1; //IPA

    
    std::ifstream inputFile(argv[2]);
    
    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file: " << argv[2] << "\n";
        return 1;
    }
    
    // Create output filenames based on input filename
    std::string inputFileName(argv[2]);
    std::string completeCsvOutputFile, vits2CsvOutputFile, normalizedCsvOutputFile, ipaCsvOutputFile;
    completeCsvOutputFile = inputFileName + "-complete.csv";
    normalizedCsvOutputFile = inputFileName + "-normalized.csv";
    ipaCsvOutputFile = inputFileName + "-ipa.csv";
    std::string normalizedTxtOutputFile = inputFileName + "-normalized.txt";
    std::string grainCsvOutputFile = inputFileName + "-grain.csv";
    
    std::ofstream completeCsvOutput(completeCsvOutputFile);
    std::ofstream normalizedCsvOutput(normalizedCsvOutputFile);
    std::ofstream ipaCsvOutput(ipaCsvOutputFile);
    std::ofstream normalizedTxtOutput(normalizedTxtOutputFile);

    // Optional GrainSpeech-format output: one path only (ADR-050). The tagged phone stream comes
    // from normalizeToTaggedPhones — the SAME API the app's engine calls — so per-word language
    // tags and the CSV's IPA/normalized columns can never diverge, and the pipeline runs once per
    // line instead of twice.
    std::ofstream grainCsvOutput;
    const bool grainEnabled = grainFlag;
    NormalizeConfig grainConfig = defaultNormalizeConfig();
    grainConfig.symbols_file = symbolsFile;   // empty -> raw phones (no inventory mapping)
    std::string grainPhones;
    if (grainEnabled) {
        grainCsvOutput.open(grainCsvOutputFile);
        if (!grainCsvOutput.is_open()) {
            std::cerr << "Error opening CSV output file: " << grainCsvOutputFile << "\n";
            return 1;
        }
        std::cout << "GrainSpeech phone output enabled: " << grainCsvOutputFile
                  << " (per-word language tags"
                  << (symbolsFile.empty() ? ", no inventory mapping)" : ", mapped to " + symbolsFile + ")")
                  << "\n";
    }
    
    if (!completeCsvOutput.is_open()) {
        std::cerr << "Error opening CSV output file: " << completeCsvOutputFile << "\n";
        return 1;
    }

    if (!normalizedCsvOutput.is_open()) {
        std::cerr << "Error opening CSV output file: " << normalizedCsvOutputFile << "\n";
        return 1;
    }

    if (!ipaCsvOutput.is_open()) {
        std::cerr << "Error opening CSV output file: " << ipaCsvOutputFile << "\n";
        return 1;
    }
    
    if (!normalizedTxtOutput.is_open()) {
        std::cerr << "Error opening TXT output file: " << normalizedTxtOutputFile << "\n";
        return 1;
    }

    /*int err;
    
    if(ipa_mode == 0)
        err = readFileToMap("../vocab-raw.txt");
    else
        err = readFileToMap("../vocab-ipa.txt");
    if (err!= 0) {
        std::cerr << "Error opening vocab file" << "\n";
        return 1;
    }*/

    // Variables for time measurement
    auto total_start = std::chrono::high_resolution_clock::now();
    int line_count = 0;
    long long total_processing_time_ns = 0;

    std::string inputLine;
    while (std::getline(inputFile, inputLine)) {
        auto line_start = std::chrono::high_resolution_clock::now();
        line_count++;
        
        // Parse pipe-delimited fields: file_name|text1|text2|...
        std::vector<std::string> fields;
        std::stringstream ss(inputLine);
        std::string field;
        while (std::getline(ss, field, '|')) {
            fields.push_back(field);
        }
        
        if (fields.empty()) {
            continue; // Skip empty lines
        }
        
        // First field is the file name (e.g., LJ043-0084)
        std::string originalFileName = fields[0];
        
        // Process the inputLine content
        std::string filePath;
        std::string speakerID;
        std::string normalizedString;
        std::string ipaString;
        //std::vector<uint8_t> idVector;
        //std::string idString;

        // Use the original file name (without extension) in the output path
        char filepath[200];
        sprintf(filepath, "DUMMY1/%s.wav", originalFileName.c_str());
        filePath = std::string(filepath);

        speakerID = "0";

        // Concatenate the remaining fields (text columns) for normalization
        std::string textToNormalize;
        for (size_t i = 1; i < fields.size(); ++i) {
            textToNormalize += fields[i];
        }

        // normalize — ONE path: with --grain the tagged phone stream is produced by the same call
        // (per-word language tags), and the IPA/normalized columns come from it too.
        if (grainEnabled) {
            normalizeToTaggedPhones(mainlang, textToNormalize, grainConfig, grainPhones,
                                    &ipaString, &normalizedString);
        } else {
            normalizeString(mainlang, ipa_mode, textToNormalize, normalizedString, ipaString);
        }
        
        //if(ipa_mode)
        //    idVector = string_to_id_vector(ipaString);
        //else
        //    idVector = string_to_id_vector(normalizedString);

        //idString = id_vector_to_id_string(idVector);

        // Remove any pipes that might have been in the processed data
        removeAllPipes(normalizedString);
        if(ipa_mode)
            removeAllSpaces(ipaString);
        
        // Create a vector with original content (without pipes) and processed version
        std::vector<std::string> completeColumns;
        completeColumns.push_back(filePath);
        completeColumns.push_back(speakerID);
        completeColumns.push_back(textToNormalize);
        completeColumns.push_back(normalizedString);
        completeColumns.push_back(ipaString);
        //completeColumns.push_back(idString);

        std::vector<std::string> normalizedColumns;
        normalizedColumns.push_back(filePath);
        //normalizedColumns.push_back(speakerID);
        normalizedColumns.push_back(normalizedString);

        std::vector<std::string> ipaColumns;
        ipaColumns.push_back(filePath);
        //ipaColumns.push_back(speakerID);
        ipaColumns.push_back(ipaString);
        
        // Optional: model-format phone stream, produced above by the single normalize call
        if (grainEnabled) {
            std::vector<std::string> grainColumns;
            grainColumns.push_back(filePath);
            grainColumns.push_back(grainPhones);
            grainCsvOutput << join(grainColumns, '|') << "\n";
        }

        // Write to CSV file with pipe delimiter
        completeCsvOutput << join(completeColumns, '|') << "\n";
        normalizedCsvOutput << join(normalizedColumns, '|') << "\n";
        ipaCsvOutput << join(ipaColumns, '|') << "\n";
        
        // Write just the processed content to the text file
        normalizedTxtOutput << normalizedString << "\n";
        //normalizedTxtOutput << ipaString << "\n";
        //normalizedTxtOutput << idString << "\n";

        auto line_end = std::chrono::high_resolution_clock::now();
        total_processing_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(line_end - line_start).count();
    }

    auto total_end = std::chrono::high_resolution_clock::now();
    auto total_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(total_end - total_start).count();

    inputFile.close();
    completeCsvOutput.close();
    normalizedCsvOutput.close();
    ipaCsvOutput.close();
    normalizedTxtOutput.close();
    if (grainCsvOutput.is_open()) grainCsvOutput.close();
    
    // Calculate and output timing information
    double avg_time_per_line_ns = static_cast<double>(total_processing_time_ns) / line_count;
    double avg_time_per_line_ms = avg_time_per_line_ns / 1'000'000.0;
    
    std::cout << "Processing complete. Output written to:\n";
    std::cout << " - " << completeCsvOutputFile << " (CSV with original, processed and ipa text)\n";
    std::cout << " - " << normalizedCsvOutputFile << " (CSV with normalized text)\n";
    std::cout << " - " << ipaCsvOutputFile << " (CSV with ipa text)\n";
    if (grainEnabled)
        std::cout << " - " << grainCsvOutputFile
                  << " (CSV with phones in the GrainSpeech model format)\n";
    std::cout << " - " << normalizedTxtOutputFile << " (processed text only)\n";
    std::cout << "\nPerformance metrics:\n";
    std::cout << " - Total lines processed: " << line_count << "\n";
    std::cout << " - Total processing time: " << (total_time_ns / 1000000.0) << " ms\n";
    std::cout << " - Average time per line: " << avg_time_per_line_ms << " ms\n";
    
    return 0;
}

void removeAllPipes(std::string& str) {
    str.erase(std::remove(str.begin(), str.end(), '|'), str.end());
}

void removeAllSpaces(std::string& str) {
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

static std::string join(const std::vector<std::string>& vec, char delimiter) {
    std::string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) result += delimiter;
        result += vec[i];
    }
    return result;
}
