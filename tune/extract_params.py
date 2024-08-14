#!/bin/python

import os
import subprocess
import sys


def usage():
    print(f"Expected Usage:\n\t{__file__} [filepath]\n\tWhere filepath is path to .clop file")
    exit(1)


# Strip clop file from the IntegerParameters specified
def extract_params(filepath: str) -> list[str]:
    rv = []

    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            split = line.split()

            if len(split) == 4 and split[0] == "IntegerParameter":
                rv.append(split[1])

    return rv


# Run clop-console to return best value once
def extract_values(filepath: str, param_count: int) -> list[int]:
    rv = []
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # redirect the settings to the newly spawned process
    with open(filepath, "r") as f:
        process = subprocess.Popen(
            [f"{script_dir}/clop-console", "c"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdin=f,
            text=True
        )

        first_line = process.stdout.readline().strip()
        process.terminate()
        process.wait()

    print(f"Got line from the CLOP console: {first_line}")
    first_line = first_line.strip()
    split = first_line.split()

    if len(split) != param_count + 4:
        print(f"Expected {param_count} parameters, got {len(split)} from the CLOP")
        exit(1)

    for val in split[4:]:
        rv.append(int(val))

    return rv


def prepare_exec_command(params: list[str], values: list[int]) -> str:
    rv = ""

    if len(params) != len(values):
        print(f"Params length ({len(params)}) does not match values length ({len(values)})")
        exit(1)

    for i in range(len(params)):
        rv += f" \"tune {params[i]} {values[i]} \""

    return rv


def prepare_params_file(params: list[str], values: list[int]) -> str:
    rv = ""

    if len(params) != len(values):
        print(f"Params length ({len(params)}) does not match values length ({len(values)})")
        exit(1)

    for i in range(len(params)):
        rv += f"{params[i]} : {values[i]}\n"

    return rv


def main(args: list[str]):
    if len(args) < 1:
        usage()

    path = args[0]
    if not os.path.exists(path):
        usage()

    params = extract_params(path)
    values = extract_values(path, len(params))
    command = prepare_exec_command(params, values)
    file = prepare_params_file(params, values)

    print(f"Resulting command:\n\t{command}")
    print(f"Resulting file:\n{file}")


if __name__ == '__main__':
    main(sys.argv[1:])
