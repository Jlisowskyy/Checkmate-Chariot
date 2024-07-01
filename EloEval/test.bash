#!/bin/bash

# Prepare
SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
cd "${SCRIPT_DIR}" || exit

# run the tournament
./cutechess-cli -concurrency 7 -resign score=600 movecount=5 \
          -pgnout ./out.pgn -games 10 \
          -engine conf=pingu \
          -engine conf=dumb  \
          -each tc=10+1 timemargin=50 option.Hash=1024