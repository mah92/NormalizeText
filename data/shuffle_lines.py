import random

def shuffle_lines(input_file, output_file=None):
    # If no output file is provided, overwrite the input file
    if output_file is None:
        output_file = input_file

    # Read all lines from the input file
    with open(input_file, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    # Shuffle the lines randomly
    random.shuffle(lines)

    # Write the shuffled lines back to the output file
    with open(output_file, 'w', encoding='utf-8') as file:
        file.writelines(lines)

# Example usage

shuffle_lines('dataset-4.txt', 'dataset-5.txt')
