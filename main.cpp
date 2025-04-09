#include "replacement.h"
#include "language_detector.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <algorithm>

// Helper function to remove all pipe characters from a string
void removeAllPipes(std::string& str) {
    str.erase(std::remove(str.begin(), str.end(), '|'), str.end());
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <main_language> <input_file> <output_file>\n";
        return 1;
    }

    // Set UTF-8 locale
    std::locale::global(std::locale("en_US.UTF-8"));
    
    std::string mainLanguage(argv[1]);
    Language mainlang = LanguageDetector::string_to_language(mainLanguage);

    std::ifstream inputFile(argv[2]);
    std::ofstream outputFile(argv[3]);
    
    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file: " << argv[2] << "\n";
        return 1;
    }
    
    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file: " << argv[3] << "\n";
        return 1;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
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
        
        // Write the modified line to output using pipe as delimiter
        outputFile << join(columns, '|') << "\n";
    }

    inputFile.close();
    outputFile.close();
    
    std::cout << "Processing complete. Output written to " << argv[3] << "\n";
    return 0;
}