#!/bin/bash

CORES=$(nproc --all)
FAIR_CORES=$((CORES / 2))

# Prepare logs directory
mkdir Logs

DATE=$(date '+%Y-%m-%d_%H:%M:%S')

PGN_OUT="Logs/${DATE}_out.pgn"
