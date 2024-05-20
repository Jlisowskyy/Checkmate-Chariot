import datetime
import os
import subprocess

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


def run_tests(engines: dict[str, int], each_options, game_options, resign_options, output_options):
    # check if we are in the correct directory (EloEval or ../EloEval)
    # if in EloEval, change directory to root
    if os.path.basename(os.getcwd()) == 'EloEval':
        os.chdir('../')

    # check if the EloEval folder exists
    if not os.path.exists(f'{e.elo_eval_path}'):
        print('EloEval folder not found, please run this script from the root or EloEval folder of the repository.')
        return

    # install things from EloEval folder
    os.chdir(f'{e.elo_eval_path}')

    # check if c-chess-cli exists
    if not os.path.exists('c-chess-cli'):
        # run c-chess-cli installer
        print('c-chess-cli not found, running installer...')
        os.system('chmod +x install-c-chess-cli.sh')
        os.system('./install-c-chess-cli.sh')

    # if the exes_path folders do not exist, create them
    if not os.path.exists(e.exes_path):
        os.makedirs(e.exes_path)

    # install missing engines
    for engine in engines:
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
    for engine in engines:
        print(f'\t{engine} ({engines[engine]})')

    # go back to root
    os.chdir('../')

    # run tests

    engine_options = ''
    # add main engine
    engine_options += f' -engine cmd=./Checkmate-Chariot name=Checkmate-Chariot_development'

    # add external engines
    for engine in engines:
        engine_options += f' -engine cmd={e.elo_eval_path}/{e.exes_path}/{engine} name={engine}'

    print(f'./{e.elo_eval_path}/c-chess-cli {each_options} {game_options} {resign_options} {output_options} {engine_options}')
    os.system(f'./{e.elo_eval_path}/c-chess-cli {each_options} {game_options} {resign_options} {output_options} {engine_options}')
