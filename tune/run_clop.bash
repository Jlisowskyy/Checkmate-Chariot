#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

clean_up(){
  echo "Failed..."
  exit 1
}

echo TODO & exit 0

SET_NAME=$1

if [ "${SET_NAME}" == "" ]; then
  echo Expected usage: "$SCRIPT_DIR"/run_clop.bash {SET NAME}
fi

cd "${SCRIPT_DIR}" || clean_up
cp ./Checkmate-Chariot-Adapter.py ./test_sets/"$SET_NAME"

