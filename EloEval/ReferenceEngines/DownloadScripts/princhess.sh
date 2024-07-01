#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
REPO_DIR="${SCRIPT_DIR}/princhess"

clean_up(){
  echo "Failed..."
  rm -rf "${REPO_DIR}"
}

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# Clone
git clone https://github.com/princesslana/princhess.git || clean_up
cd princhess || clean_up
git checkout 49397b8770d159be045feefb6191218d5f45ac1e || clean_up

# Build
cargo build --release || clean_up
cp target/release/princhess ../.. || clean_up # DownloadScripts/princhess

# clean up
rm -rf "${REPO_DIR}"
echo SUCCESS