#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CORES=$(nproc --all)
REPO_DIR="${SCRIPT_DIR}/saruman"

clean_up(){
  echo "Failed..."
  cd "${START_DIR}" || exit 1
}

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# Clone
git clone https://github.com/ConorGriffin37/chess.git saruman || clean_up
cd saruman || clean_up
git checkout fb043b66cf6b8642cc1a32b8f2fbc908273f1733 || clean_up
cd engine || clean_up

# Build
make all -j "${CORES}" || clean_up
cp Saruman ../../.. || clean_up # DownloadScripts/saruman/engine

# clean up
rm -rf "${REPO_DIR}"