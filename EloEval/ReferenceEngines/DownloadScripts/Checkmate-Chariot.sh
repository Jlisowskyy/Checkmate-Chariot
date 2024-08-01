#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CORES=$(nproc --all)
REPO_DIR="${SCRIPT_DIR}/Checkmate-Chariot"

clean_up(){
  echo "Failed..."
  cd "${START_DIR}" || exit 1
}

if [ -z "$1" ]; then
  VERSION="HEAD"
else
  VERSION="$1"
fi

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# Clone
git clone https://github.com/Jlisowskyy/Checkmate-Chariot || clean_up
cd Checkmate-Chariot || clean_up
git checkout "${VERSION}" || clean_up

# Build
cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release || clean_up
make Checkmate-Chariot -j "${CORES}" || clean_up
cp Checkmate-Chariot "../../Checkmate-Chariot" ||
cp uci_ready_long "../../uci_ready_long" || clean_up


# clean up
rm -rf "${REPO_DIR}"
echo SUCCESS