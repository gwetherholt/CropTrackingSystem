#!/bin/bash

# Enable a script to display output of commands
set -e

# Loop through all JSON files in data/input
for FILE in data/input/*.json; do
    # Get the base name of the file without the extension
    BASE=$(basename "$FILE" .json)

    echo "Running $FILE ..."

    # Run the docker container for each JSON file
    docker run --rm -v "$PWD":/project tracking-solution \
      --input /project/"$FILE" \
      --output /project/data/output/"$BASE"/tracking_output.json \
      --vis-dir /project/data/output/"$BASE"/visualization
done

echo "All datasets processed."
