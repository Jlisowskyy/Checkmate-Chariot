#!/bin/bash

clean_up(){
  echo "Failed..."
  exit 1
}

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CORES=$(nproc --all)
cd "${SCRIPT_DIR}" || clean_up

# build CLOP
cd clop_src/programs/clop/compgcc || clean_up
make -j "${CORES}" || clean_up
cp bin/clop "${SCRIPT_DIR}" || clean_up
make clean || clean_up
cd "${SCRIPT_DIR}" || clean_up

# build CLOP CLI
cd clop_src/programs/clop/compqt/clop-console || clean_up
qmake || clean_up
make -j "${CORES}"|| clean_up
cp clop-console "${SCRIPT_DIR}" || clean_up
make clean || clean_up
rm Makefile || clean_up

# build CLOP GUI
#cd clop_src/programs/clop/compqt/clop-gui || clean_up
#qmake || clean_up
#make || clean_up
#cp clop-console "${SCRIPT_DIR}" || clean_up
#make clean || clean_up
#rm Makefile || clean_up

# build cute-chess-cli
cd "${SCRIPT_DIR}" || clean_up
cd ..
cd EloEval || clean_up
./cutechess-installer.bash
cp cutechess-cli "${SCRIPT_DIR}" || clean_up

# build the engine
cd "${SCRIPT_DIR}" || clean_up

cd ..
COMMIT_ID="$(git log --format="%H" -n 1)"

cd ./EloEval/ReferenceEngines/DownloadScripts || clean_up
./Checkmate-Chariot.sh "${COMMIT_ID}"
cd ..
cp Checkmate-Chariot "${SCRIPT_DIR}" || clean_up