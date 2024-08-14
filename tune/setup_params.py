import sys, os


def parse_to_clop(file: str):
    with open(file, 'r') as file:

        for line in file:
            stripped = line.strip()
            if len(stripped) < 2 or ((stripped[0] == stripped[1]) and stripped[0] == '/'):
                continue

            splitted = line.split()

            if len(splitted) < 4:
                continue

            print(f'IntegerParameter {splitted[0]} {splitted[2]} {splitted[3]}')


def parse_args(args: list[str]):
    if len(args) != 3:
        exit(1)

    opt = args[1]
    file = args[2]

    opts = {
        '--parse_to_clop': parse_to_clop,
    }

    if opt in opts.keys():
        opts[opt](file)


if __name__ == "__main__":
    parse_args(sys.argv)
