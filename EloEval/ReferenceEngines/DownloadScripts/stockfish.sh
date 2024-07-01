#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
REPO_DIR="${SCRIPT_DIR}/Stockfish"

clean_up(){
  echo "Failed..."
  rm -rf "${REPO_DIR}"
}

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# Clone
git clone https://github.com/official-stockfish/Stockfish.git || clean_up
cd Stockfish || clean_up
git checkout 7922e07af83dd472da6e5b38fb84214cfe46a630 || clean_up
cd src || clean_up

# Build
make -j profile-build ARCH=x86-64-avx2
cp stockfish ../../.. || clean_up # DownloadScripts/Stockfish/src

# clean up
rm -rf "${REPO_DIR}"
