import sys


def check_argv_count(expected: int, msg: str = "") -> None:
    if len(sys.argv) != expected:
        raise Exception(
            f"Expected: {expected} command line arguments, received: {len(sys.argv)}!\n{msg}")
