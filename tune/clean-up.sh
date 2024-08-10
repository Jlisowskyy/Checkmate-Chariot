#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
clean_up(){
  echo "Failed..."
  exit 1
}

cd "$SCRIPT_DIR" || clean_up

rm Checkmate-Chariot
rm clop clop-gui clop-console cutechess-cli
rm Checkmate-Chariot-Set*