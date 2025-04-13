#include "replacement.h"
#include "language_detector.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <algorithm>
#include <chrono>  // For time measurement

// Helper function to remove all pipe characters from a string
void removeAllPipes(std::string& str) {
    str.erase(std::remove(str.begin(), str.end(), '|'), str.end());
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <main_language> <input_file>\n";
        return 1;
    }

    // Set UTF-8 locale
    std::locale::global(std::locale("en_US.UTF-8"));
    
    std::string mainLanguage(argv[1]);
    Language mainlang = LanguageDetector::string_to_language(mainLanguage);

    std::ifstream inputFile(argv[2]);
    
    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file: " << argv[2] << "\n";
        return 1;
    }
    
    // Create output filenames based on input filename
    std::string inputFileName(argv[2]);
    std::string csvOutputFile = inputFileName + "-normalized.csv";
    std::string txtOutputFile = inputFileName + "-normalized.txt";
    
    std::ofstream csvOutput(csvOutputFile);
    std::ofstream txtOutput(txtOutputFile);
    
    if (!csvOutput.is_open()) {
        std::cerr << "Error opening CSV output file: " << csvOutputFile << "\n";
        return 1;
    }
    
    if (!txtOutput.is_open()) {
        std::cerr << "Error opening TXT output file: " << txtOutputFile << "\n";
        return 1;
    }

    // Variables for time measurement
    auto total_start = std::chrono::high_resolution_clock::now();
    size_t line_count = 0;
    long long total_processing_time_ns = 0;

    std::string line;
    while (std::getline(inputFile, line)) {
        auto line_start = std::chrono::high_resolution_clock::now();
        line_count++;
        
        // First remove all pipes from the entire line
        removeAllPipes(line);
        
        // Process the line content
        std::string processed = performReplacements(mainlang, line);
        
        // Remove any pipes that might have been in the processed data
        removeAllPipes(processed);
        
        // Create a vector with original content (without pipes) and processed version
        std::vector<std::string> columns;
        columns.push_back(line);
        columns.push_back(processed);
        
        // Write to CSV file with pipe delimiter
        csvOutput << join(columns, '|') << "\n";
        
        // Write just the processed content to the text file
        txtOutput << processed << "\n";

        auto line_end = std::chrono::high_resolution_clock::now();
        total_processing_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(line_end - line_start).count();
    }

    auto total_end = std::chrono::high_resolution_clock::now();
    auto total_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(total_end - total_start).count();

    inputFile.close();
    csvOutput.close();
    txtOutput.close();
    
    // Calculate and output timing information
    double avg_time_per_line_ns = static_cast<double>(total_processing_time_ns) / line_count;
    double avg_time_per_line_ms = avg_time_per_line_ns / 1'000'000.0;
    
    std::cout << "Processing complete. Output written to:\n";
    std::cout << " - " << csvOutputFile << " (CSV with original and processed text)\n";
    std::cout << " - " << txtOutputFile << " (processed text only)\n";
    std::cout << "\nPerformance metrics:\n";
    std::cout << " - Total lines processed: " << line_count << "\n";
    std::cout << " - Total processing time: " << (total_time_ns / 1000000.0) << " ms\n";
    std::cout << " - Average time per line: " << avg_time_per_line_ms << " ms\n";
    
    return 0;
}