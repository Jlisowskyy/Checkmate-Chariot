#!/bin/python

from TestRunner import *

if __name__ == '__main__':
    real_cpu_count = get_real_cpu_count_unix()
    ram_per_bot = round ( get_usable_ram_unix() / ( real_cpu_count * 2 + 2 ) )
    ram_per_bot = largest_power_of_2_less_than(ram_per_bot)
    print(f'ram_per_bot: {ram_per_bot}')

    each_options = f'-each tc=10+1 option.Threads=1 option.Hash={ram_per_bot}'
    game_options = f'-games 1000 -repeat -gauntlet -concurrency {real_cpu_count}'
    resign_options = f'-resign count=3 score=700 -draw number=40 count=12 score=10'
    output_options = f'-pgn {datetime.datetime.now().strftime("%Y-%m-%d-%H-%M")}-results.pgn 0 -log'

    run_tests(e.ext_engines, each_options, game_options, resign_options, output_options)
