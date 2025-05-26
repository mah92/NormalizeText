import os
import csv

# Path to the metadata file and the wav folder
metadata_file = 'metadata.csv'
wav_folder = 'wav-12'

# Read the IDs from the metadata file
ids_in_metadata = set()
with open(metadata_file, 'r', encoding='utf-8') as file:
    reader = csv.reader(file, delimiter='|')
    for row in reader:
        ids_in_metadata.add(row[0].strip())  # Add the ID to the set

# Iterate over the wav files and delete those not in the metadata
for filename in os.listdir(wav_folder):
    if filename.endswith('.wav'):
        file_id = filename.split('.')[0]  # Extract the ID from the filename
        if file_id not in ids_in_metadata:
            file_path = os.path.join(wav_folder, filename)
            os.remove(file_path)
            print(f"Deleted: {file_path}")
