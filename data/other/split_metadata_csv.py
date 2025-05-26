import random

# File paths
input_file      = "/home/oem/Basir/TTS/Datasets/Quran/english-persian-22/metadata-persian.csv"
wav_folder      = "/home/oem/Basir/TTS/Datasets/Quran/english-persian-22/wav-persian-22/"
train_file      = "/home/oem/Basir/TTS/Datasets/Quran/english-persian-22/train-p.txt"
validation_file = "/home/oem/Basir/TTS/Datasets/Quran/english-persian-22/val-p.txt"
test_file       = "/home/oem/Basir/TTS/Datasets/Quran/english-persian-22/test-p.txt"

# Read the file as raw text
with open(input_file, "r", encoding="utf-8") as f:
    lines = f.readlines()

# Transform the format
transformed_lines = []
for line in lines:
    file_id, text = line.strip().split("|", 1)  # Split on the first "|"
    transformed_line = f"{wav_folder}/{file_id}.wav|{text}"
    transformed_lines.append(transformed_line)

# Shuffle the data
random.shuffle(transformed_lines)

# Calculate split sizes
total_lines = len(transformed_lines)
train_size = int(0.95 * total_lines)
validation_size = int(0.045 * total_lines)
test_size = total_lines - train_size - validation_size

# Split the data
train_data = transformed_lines[:train_size]
validation_data = transformed_lines[train_size:train_size + validation_size]
test_data = transformed_lines[train_size + validation_size:]

# Save to files
with open(train_file, "w", encoding="utf-8") as f:
    f.write("\n".join(train_data))

with open(validation_file, "w", encoding="utf-8") as f:
    f.write("\n".join(validation_data))

with open(test_file, "w", encoding="utf-8") as f:
    f.write("\n".join(test_data))

print(f"Data split and saved successfully!")
print(f"Train: {len(train_data)} lines")
print(f"Validation: {len(validation_data)} lines")
print(f"Test: {len(test_data)} lines")
