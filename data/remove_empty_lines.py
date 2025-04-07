def remove_empty_lines(input_file, output_file=None):
    # If no output file is provided, overwrite the input file
    if output_file is None:
        output_file = input_file

    # Read all lines from the input file
    with open(input_file, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    # Remove empty lines (lines with only whitespace or newline characters)
    non_empty_lines = [line for line in lines if line.strip()]

    # Write the non-empty lines back to the output file
    with open(output_file, 'w', encoding='utf-8') as file:
        file.writelines(non_empty_lines)

# Example usage

remove_empty_lines('dataset_AR-EN.csv', 'dataset_AR-EN-2.csv')
