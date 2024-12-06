#!/bin/bash

# Possible arguments (only first positional argument):
# - "r" - rebuild the release version of Checkmate-Chariot and setup the sparing
# - "l" - compare latest commit from active branch with current code
# - "{Engine name contained in ReferenceEngines}" - simply tries to setup the sparing with given engine - by default
#                                                   Checkmate-Chariot


SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CORES=$(nproc --all)
OP="Checkmate-Chariot"

clean_up(){
  echo "Failed..."
  exit 1
}

# Prepare
cd "${SCRIPT_DIR}" || clean_up

# Reload flag to rebuild the main release exec
if [ "$1" == "r" ] ; then
  echo "Building the most recent release"

  ./ReferenceEngines/DownloadScripts/Checkmate-Chariot.sh
elif [ "$1" == "l" ]; then
  echo "Building active branch latest commit..."

  cd ..
  COMMIT_ID="$(git log --format="%H" -n 1)"
  cd "${SCRIPT_DIR}" || exit
  ./ReferenceEngines/DownloadScripts/Checkmate-Chariot.sh "${COMMIT_ID}"
elif [ -f "${SCRIPT_DIR}/ReferenceEngines/${1}" ] || [ "$1" == "params" ]; then
  OP=$1
else
  clean_up
fi

# Always rebuild dev version
cd ..
cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release || clean_up
make Checkmate-Chariot -j "${CORES}" || clean_up
cp Checkmate-Chariot "./EloEval/ReferenceEngines/Checkmate-Chariot-dev" || clean_up
./scripts/clean-cmake.sh

# run the tournament
./EloEval/Tournaments/short_test_tournament.bash Checkmate-Chariot-dev "${OP}"