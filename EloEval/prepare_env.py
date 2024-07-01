#!/bin/python

import os
import subprocess

from engines import *

WORKING_DIR = os.path.dirname(__file__)
TOURNAMENT_INSTALLER = "cutechess-installer.bash"


def run_script(script: str):
    print(f"Installing with: {script}")
    subprocess.call([script], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)


def full_env_prepare(scripts_dir: str = WORKING_DIR):
    manager_path = os.path.join(scripts_dir, "cutechess-cli")

    # install tournament manager if lacking
    if not os.path.isfile(manager_path):
        manager_installer = os.path.join(scripts_dir, TOURNAMENT_INSTALLER)

        run_script(manager_installer)
    else:
        print(f"Already installed: {manager_path}")

    # install every engine if lacking
    for engine in engines:
        engine_path = os.path.join(scripts_dir, "ReferenceEngines", engine)

        if not os.path.isdir(engine_path):
            script_dir = os.path.join(scripts_dir, "ReferenceEngines", "DownloadScripts", f"{engine}.sh")

            run_script(script_dir)
        else:
            print(f"Already installed: {engine_path}")


if __name__ == '__main__':
    full_env_prepare()
