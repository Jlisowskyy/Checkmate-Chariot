#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

clean_up(){
  echo "Failed..."
  exit 1
}

# Prepare

cd "${SCRIPT_DIR}" || clean_up
source tournament_func.bash

# Ensure good working directory
cd ..

./cutechess-cli -concurrency "${FAIR_CORES}" -resign score=600 movecount=5 \
          -pgnout "${PGN_OUT}" -games 100 \
          -engine conf="$1" \
          -engine conf="$2"  \
          -each tc=10+1 timemargin=50 option.Hash=1024 dir="ReferenceEngines"