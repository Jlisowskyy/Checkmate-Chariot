#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CORES=$(nproc --all)
REPO_DIR="${SCRIPT_DIR}/Dumb"

clean_up(){
  echo "Failed..."
  rm -rf "${REPO_DIR}"
}

# Prepare

sudo pacman -S --noconfirm gdc
cd "${SCRIPT_DIR}" || clean_up

# Prepare repository
git clone https://github.com/abulmo/Dumb.git || clean_up
cd Dumb || clean_up
git checkout b67b883 || clean_up
cd src || clean_up

# build
make pgo DC=gdc -j "${CORES}" || clean_up
cp dumb ../../.. || clean_up # DownloadScripts/dumb/src

# clean up
rm -rf "${REPO_DIR}"