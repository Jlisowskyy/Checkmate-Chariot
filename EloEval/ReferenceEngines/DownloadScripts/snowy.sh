#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CORES=$(nproc --all)
REPO_DIR="${SCRIPT_DIR}/snowy"

clean_up(){
  echo "Failed..."
  rm -rf "${REPO_DIR}"
}

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# Clone
git clone --recursive https://github.com/JasonCreighton/snowy.git || clean_up
cd snowy || clean_up
git checkout b0aa777ca69ef3411d9fb531e591306dbfaf6a48 || clean_up

# Build
make -j "${CORES}"
mv build/release ../../snowy # DownloadScripts/snowy

# clean up
rm -rf "${REPO_DIR}"
echo SUCCESS