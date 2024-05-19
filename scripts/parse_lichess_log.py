#!/bin/python

import io

from parserTools import *


def parse_lichess_line(line: str, out_file: io.TextIOWrapper, in_file: io.TextIOWrapper):
    if line.find("<<") != -1:
        input_command = line.split("<<")[1]
        in_file.write(input_command)
    elif line.find(">>") != -1:
        output_command = line.split(">>")[1]
        out_file.write(output_command)


def parse_lichess(path: str):
    with open(path, "r") as f:
        out_file = open(f"{path}_out", "w")
        in_file = open(f"{path}_in", "w")

        for line in f:
            parse_lichess_line(line, out_file, in_file)

        out_file.close()
        in_file.close()


if __name__ == "__main__":
    usage = '''
        Script expects to receive as first argument path to the file, that needs to be parsed.
    
        Script outputs both input and output to corresponding files: 'path'_out and 'path'_in, 
        where 'path' is given as first argument
    '''

    check_argv_count(2, usage)
    parse_lichess(sys.argv[1])
