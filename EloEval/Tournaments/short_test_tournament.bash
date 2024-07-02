#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

# Prepare
cd "${SCRIPT_DIR}" || exit 1
source tournament_func.bash

# Ensure good working directory
cd ..

./cutechess-cli -debug -concurrency "${FAIR_CORES}" -resign score=600 movecount=5 \
          -pgnout "${PGN_OUT}" -games 10 \
          -engine conf="$1" \
          -engine conf="$2"  \
          -each tc=10 timemargin=500 option.Hash=1024 > "${OUT_FILE}" 2>&1 &

post_tournamanet