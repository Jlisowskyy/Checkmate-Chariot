#!/bin/python
import os
import re
import sys


def process_debug_file(debug_file: str, debug_out: str):
    with open(debug_file, 'r') as file:
        for line in file:
            # Extract the second word (identifier for the file name)
            identifier = line.split()[1]
            identifier = re.sub(r'[^a-zA-Z0-9-_]', '', identifier)

            # Create the output file path
            file_path = os.path.join(debug_out, f"{identifier}.out")

            # Append the line to the corresponding file
            with open(file_path, 'a') as output_file:
                output_file.write(line)


if __name__ == '__main__':
    debug = sys.argv[1]
    debug_dir = sys.argv[2]

    process_debug_file(debug, debug_dir)
