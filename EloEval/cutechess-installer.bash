#!/bin/bash

SCRIPT_DIR="$(dirname -f "${BASH_SOURCE[0]}")"
REPO_DIR="${SCRIPT_DIR}/cute-chess"
CORES=$(nproc --all)

clean_up(){
  echo "Failed..."
  cd "${START_DIR}" || exit 1
}

# clone the repo
git clone https://github.com/cutechess/cutechess "$REPO_DIR"
cd "$REPO_DIR" || clean_up

# build the exec in the special directory
mkdir build
cd build || clean_up
cmake ..
make -j "$CORES"

# move the exec to the eval directory
cp cutechess-cli ../..

rm -rf "${REPO_DIR}"