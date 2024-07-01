#!/bin/bash

SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
REPO_DIR="${SCRIPT_DIR}/cute-chess"
START_DIR=$(pwd)
CORES=$(nproc --all)

# clone the repo
git clone https://github.com/cutechess/cutechess "$REPO_DIR" || exit
cd "$REPO_DIR" || exit

# build the exec in the special directory
mkdir build || exit
cd build || exit
cmake ..
make -j "$CORES"

# move the exec to the eval directory
mv cutechess-cli "${SCRIPT_DIR}"

cd "$START_DIR" || exit
#rm -rf "${REPO_DIR}"