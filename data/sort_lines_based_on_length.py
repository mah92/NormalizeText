def sort_lines_by_length(input_file, output_file=None):
    # If no output file is provided, overwrite the input file
    if output_file is None:
        output_file = input_file

    # Read all lines from the input file
    with open(input_file, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    # Sort the lines by their length (shortest to longest)
    lines.sort(key=len)

    # Write the sorted lines back to the output file
    with open(output_file, 'w', encoding='utf-8') as file:
        file.writelines(lines)

# Example usage
input_file = 'dataset_AR-EN.txt'  # Replace with your file path
sort_lines_by_length(input_file)
