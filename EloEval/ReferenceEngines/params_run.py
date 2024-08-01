import os
import sys


def main(args: list[str]):
    if len(args) != 2:
        raise Exception(f"Wrong number of arguments. Expected 1, received: {len(args) - 1}")

    filename = sys.argv[1]
    args = []

    with open(filename, "r") as f:
        for line in f:
            split = line.split()

            if len(split) != 3 or split[1] != ':':
                raise Exception("Expected format: \"PARAM_NAME : PARAM_VALUE\"")
            args.append(f"tune {split[0]} {split[2]}")

    os.execv("Checkmate-Chariot", args)


if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.abspath(sys.argv[0]))
    os.chdir(script_dir)

    main(sys.argv)
