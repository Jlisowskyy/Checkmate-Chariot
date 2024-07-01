#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CORES=$(nproc --all)
REPO_DIR="${SCRIPT_DIR}/Pingu"

clean_up(){
  echo "Failed..."
  rm -rf "${REPO_DIR}"
}

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# Clone
git clone https://github.com/WillChing01/Pingu.git || clean_up
cd Pingu || clean_up
git checkout 1a69999 || clean_up

# Build
make -j "${CORES}" || clean_up
cp Pingu ../.. # DownloadScripts/Pingu

# clean up
rm -rf "${REPO_DIR}"
