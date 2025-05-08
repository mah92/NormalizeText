from collections import Counter

def process_utf8_file(input_file, output_file, top_n=100):
    # Read input file with UTF-8 encoding
    with open(input_file, 'r', encoding='utf-8') as f:
        text = f.read()
    
    # Count character frequencies
    char_counts = Counter(text)
    
    # Get top N most frequent characters
    top_chars = [char for char, count in char_counts.most_common(top_n)]
    
    # Sort by Unicode code point
    sorted_chars = sorted(top_chars, key=ord)
    
    # Write to output file
    with open(output_file, 'w', encoding='utf-8') as f:
        for char in sorted_chars:
            f.write(f"{char}\n")

if __name__ == "__main__":
    # Configuration
    INPUT_FILE = "./azarbaijani-turki.txt"       # Change to your input file path
    OUTPUT_FILE = "azarbaijani-turki_utf8_chars.txt"     # Change to desired output file path
    TOP_N = 110                    # Number of most frequent chars to keep
    
    # Process the file
    process_utf8_file(INPUT_FILE, OUTPUT_FILE, TOP_N)
    print(f"Processing complete. Results saved to {OUTPUT_FILE}")