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
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <main_language: EN, FA, AR> <input_file>\n";
        return 1;
    }

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
    
    std::ofstream completeCsvOutput(completeCsvOutputFile);
    std::ofstream normalizedCsvOutput(normalizedCsvOutputFile);
    std::ofstream ipaCsvOutput(ipaCsvOutputFile);
    std::ofstream normalizedTxtOutput(normalizedTxtOutputFile);
    
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

        //normalize
        normalizeString(mainlang, ipa_mode, textToNormalize, normalizedString, ipaString);
        
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
    
    // Calculate and output timing information
    double avg_time_per_line_ns = static_cast<double>(total_processing_time_ns) / line_count;
    double avg_time_per_line_ms = avg_time_per_line_ns / 1'000'000.0;
    
    std::cout << "Processing complete. Output written to:\n";
    std::cout << " - " << completeCsvOutputFile << " (CSV with original, processed and ipa text)\n";
    std::cout << " - " << normalizedCsvOutputFile << " (CSV with normalized text)\n";
    std::cout << " - " << ipaCsvOutputFile << " (CSV with ipa text)\n";
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
