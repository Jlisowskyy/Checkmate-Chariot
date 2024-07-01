#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
REPO_DIR="${SCRIPT_DIR}/Maxwell"

clean_up(){
  echo "Failed..."
  rm -rf "${REPO_DIR}"
}

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# clone
git clone https://github.com/eboatwright/Maxwell.git || clean_up
cd Maxwell || clean_up
git checkout 28cca0397b822237aaea42aceee990c118198b7d || clean_up

# build
cargo build --release || clean_up
cp target/release/maxwell ../.. || clean_up # DownloadScripts/Maxwell

# clean up
rm -rf "${REPO_DIR}"
