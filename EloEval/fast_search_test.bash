#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CORES=$(nproc --all)

clean_up(){
  echo "Failed..."
  exit 1
}

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# Reload flag to rebuild the main release exec
if [ "$1" == "r" ] ; then
  ./ReferenceEngines/DownloadScripts/Checkmate-Chariot.sh
fi

# Always rebuild dev version
cd ..
cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release || clean_up
make -j "${CORES}" || clean_up
cp Checkmate-Chariot "./EloEval/ReferenceEngines/Checkmate-Chariot-dev" || clean_up

# Prepare logs directory
cd "${SCRIPT_DIR}" || clean_up
mkdir Logs || clean_up

# run the tournament