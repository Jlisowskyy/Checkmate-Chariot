#!/bin/bash

CORES=$(nproc --all)
export FAIR_CORES=$((CORES / 2))

# Prepare logs directory
mkdir ../../Logs 2> /dev/null

DATE=$(date '+%Y-%m-%d_%H:%M:%S')

export PGN_OUT="Logs/${DATE}_out.pgn"
DEBUG_OUT="Logs/${DATE}_debug"

OUT_FILE="output.log"
DEBUG_FILE="debug.out"

# Function to stop the tail -f process
stop_tail() {
  echo "Stopping tail -f..."
  kill "$TAIL_PID" || exit
}

post_tournamanet(){
  # Create live preview of tournament
  tail -f "${OUT_FILE}" | grep -v '^[0-9]' &
  TAIL_PID=$!

  # Trap the script termination signals and stop tail -f
  trap stop_tail SIGINT SIGTERM

  # Wait for the tail -f process to finish
  wait $TAIL_PID

  # Separate debug info to the debug file
  grep '^[0-9]' "${OUT_FILE}" >> "${DEBUG_FILE}"

  mkdir "${DEBUG_OUT}"

  # Process the file
  ./parse_debug.py "${DEBUG_FILE}" "${DEBUG_OUT}"

  rm "${DEBUG_FILE}"
#  rm "${OUT_FILE}"

  echo SUCCESS
}