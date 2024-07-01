#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CORES=$(nproc --all)
REPO_DIR="${SCRIPT_DIR}/Winter"

clean_up(){
  echo "Failed..."
  rm -rf "${REPO_DIR}"
}

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# Clone
git clone https://github.com/rosenthj/Winter.git || clean_up
cd Winter || clean_up
git checkout 3ac0a0ff0f6f11a456b2b9c987c3ca30fa98d029 || clean_up

# Build
make -j "${CORES}" || clean_up
cp Winter ../..|| clean_up # DownloadScripts/Winter

# clean up
rm -rf "${REPO_DIR}"
