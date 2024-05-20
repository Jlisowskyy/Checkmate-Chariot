#!/usr/bin/env python


import multiprocessing
import os
import subprocess


# Define the format_file function at the top level
def format_file(file):
    subprocess.run(['clang-format', '-style=file', '-i', file])


def clang_format_files():
    # Find all files matching the regex: .*\.\(cpp\|cc\|h\|c\)
    files = []
    for root, _, filenames in os.walk('.'):
        for filename in filenames:
            if filename.endswith(('.cpp', '.cc', '.h', '.c')):
                files.append(os.path.join(root, filename))

    # Format files in parallel
    with multiprocessing.Pool() as pool:
        pool.map(format_file, files)


if __name__ == '__main__':
    clang_format_files()
