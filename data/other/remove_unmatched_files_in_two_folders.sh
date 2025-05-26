#!/bin/bash

# Define source and destination folders
SOURCE_FOLDER="/home/oem/Basir/TTS/Datasets/Phone-Online/Female/wav-16/"
DESTINATION_FOLDER="/home/oem/Basir/TTS/Datasets/Phone-Online/Female/wav-22/"

# Loop through files in the destination folder
find "$DESTINATION_FOLDER" -type f | while read -r file; do
    # Get the filename (without the path)
    filename=$(basename "$file")

    # Check if the file does not exist in the source folder
    if [ ! -f "$SOURCE_FOLDER/$filename" ]; then
        # Remove the file from the destination folder
        echo "Removing file: $file"
        rm "$file"
    fi
done

echo "Done!"
