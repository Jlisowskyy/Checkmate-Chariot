import os
import multiprocessing
import datetime
import subprocess
import platform

import engines as e


def get_real_cpu_count_unix():
    return int(subprocess.check_output("cat /proc/cpuinfo | grep 'cpu cores' | uniq | awk '{print $4}'"
, shell=True).strip())

def get_usable_ram_unix():
    with open('/proc/meminfo', 'r') as file:
        for line in file:
            if 'MemAvailable' in line:
                _, available_memory_kb = line.split(':')
                available_memory_kb = available_memory_kb.strip().split()[0]
                return int(available_memory_kb) // 1024  # Convert KB to MB

def largest_power_of_2_less_than(x):
    # Step 1: Check if the input is less than or equal to 1, as there are no powers of 2 less than 1
    if x <= 1:
        return 0

    # Step 2: Find the largest power of 2 that is less than x
    # Shift the bits of x-1 right until only the highest bit is left
    power = 1
    while power <= x // 2:
        power <<= 1  # This doubles the number, effectively moving to the next power of 2

    return power

real_cpu_count = get_real_cpu_count_unix()
ram_per_bot = round ( get_usable_ram_unix() / ( real_cpu_count * 2 + 2 ) )
ram_per_bot = largest_power_of_2_less_than(ram_per_bot)
print(f'ram_per_bot: {ram_per_bot}')

each_options = f'-each tc=10+1 option.Threads=1 option.Hash={ram_per_bot}'
game_options = f'-games 1000 -repeat -gauntlet -concurrency {real_cpu_count}'
game_options = f'-games 1000 -repeat -gauntlet -concurrency 8'
resign_options = f'-resign count=3 score=700 -draw number=40 count=12 score=10'
output_options = f'-pgn {datetime.datetime.now().strftime("%Y-%m-%d-%H-%M")}-results.pgn 0 -log'

# check if c-chess-cli exists
if not os.path.exists('c-chess-cli'):
    # run c-chess-cli installer
    print('c-chess-cli not found, running installer...')
    os.system('chmod +x c-chess-cli-installer.sh')
    os.system('./c-chess-cli-installer.sh')

# if the exes_path folders do not exist, create them
if not os.path.exists(e.exes_path):
    os.makedirs(e.exes_path)

# install missing engines
for engine in e.ext_engines:
    if not os.path.exists(f'{e.exes_path}/{engine}'):
        # change folder to installers
        print(f'{engine} not found, running installer...')
        os.system(f'chmod +x {e.installers_path}/{engine}.sh')
        os.chdir(e.installers_path)
        os.system(f'./{engine}.sh')
        # change folder back to root
        os.chdir('../../')

# print elo of each engine
print('running tests with the following engines:')
for engine in e.ext_engines:
    print(f'\t{engine} ({e.ext_engines[engine]})')

# run tests

engine_options = ''
# add main engine
engine_options += f' -engine cmd=../Checkmate-Chariot name=Checkmate-Chariot'

# add external engines
for engine in e.ext_engines:
    engine_options += f' -engine cmd={e.exes_path}/{engine} name={engine}'

print(f'./c-chess-cli {each_options} {game_options} {resign_options} {output_options} {engine_options}')
os.system(f'./c-chess-cli {each_options} {game_options} {resign_options} {output_options} {engine_options}')




