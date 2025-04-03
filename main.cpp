#include "replacement.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <algorithm>

// Helper function to remove all pipe characters from a string
void removeAllPipes(std::string& str) {
    str.erase(std::remove(str.begin(), str.end(), '|'), str.end());
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
        return 1;
    }

    // Set UTF-8 locale
    std::locale::global(std::locale("en_US.UTF-8"));
    
    std::ifstream inputFile(argv[1]);
    std::ofstream outputFile(argv[2]);
    
    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file: " << argv[1] << "\n";
        return 1;
    }
    
    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file: " << argv[2] << "\n";
        return 1;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        // First remove all pipes from the entire line
        removeAllPipes(line);
        
        // Now split into columns (using a different delimiter temporarily)
        std::vector<std::string> columns = split(line, '\t');  // Using tab as temporary delimiter
        
        if (!columns.empty()) {
            // Process the last column
            std::string processed = performReplacements(columns.back());
            
            // Remove any pipes that might have been in the original data
            removeAllPipes(processed);
            
            // Add the processed text as a new column
            columns.push_back(processed);
            
            // Write the modified line to output using pipe as delimiter
            outputFile << join(columns, '|') << "\n";
        }
    }

    inputFile.close();
    outputFile.close();
    
    std::cout << "Processing complete. Output written to " << argv[2] << "\n";
    return 0;
}