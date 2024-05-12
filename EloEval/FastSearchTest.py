#!/bin/python

import sys
from TestRunner import *

if __name__ == '__main__':

    # argument check
    if len(sys.argv) != 2:
        print('Usage: python FastSearchTest.py "Engine name"')
        sys.exit(1)

    engine_name = sys.argv[1]

    # check if engine_name is in present in the know engines dictionary
    if engine_name not in e.ext_engines:
        print(f'Engine {engine_name} not found in engines.py')
        sys.exit(1)

    real_cpu_count = get_real_cpu_count_unix()
    ram_per_bot = round(get_usable_ram_unix() / (real_cpu_count * 2 + 2))
    ram_per_bot = largest_power_of_2_less_than(ram_per_bot)
    print(f'ram_per_bot: {ram_per_bot}')

    each_options = f'-each tc=10+1 option.Threads=1 option.Hash={ram_per_bot}'
    game_options = f'-games 100 -repeat -gauntlet -concurrency {real_cpu_count}'
    output_options = f'-pgn {datetime.datetime.now().strftime("%Y-%m-%d-%H-%M")}-results-fast-run.pgn 0 -log'

    run_tests({engine_name: e.ext_engines[engine_name]}, each_options, game_options, "", output_options)
