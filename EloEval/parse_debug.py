#!/bin/python
import os
import re
import sys


def process_debug_file(debug_file: str, debug_out: str):
    with open(debug_file, 'r') as file:
        for line in file:
            # Extract the second word (identifier for the file name)
            split = line.split()
            identifier = split[1]
            dir_sign = "out" if identifier[0] == '<' else "in"
            identifier = re.sub(r'[^a-zA-Z0-9-_]', '', identifier)

            # Create the output file path
            file_path = os.path.join(debug_out, f"{identifier}.{dir_sign}")

            # Create file concatenating both files
            full_file_path = os.path.join(debug_out, f"{identifier}.full")

            with open(full_file_path, 'a') as output_file:
                output_file.write(line)

            # Append the line to the corresponding file
            with open(file_path, 'a') as output_file:
                if dir_sign == "in":
                    line = ' '.join(split[0:1] + split[2:]) + '\n'

                output_file.write(line)


if __name__ == '__main__':
    debug = sys.argv[1]
    debug_dir = sys.argv[2]

    process_debug_file(debug, debug_dir)
