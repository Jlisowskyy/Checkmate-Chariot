#!/bin/bash

clean_up(){
  echo "Failed..."
  exit 1
}

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
cd "${SCRIPT_DIR}" || clean_up

# build CLOP
cd clop_src/programs/clop/compgcc || clean_up
make || clean_up
cp bin/clop "${SCRIPT_DIR}" || clean_up
make clean || clean_up
cd "${SCRIPT_DIR}" || clean_up

# build CLOP CLI
cd clop_src/programs/clop/compqt/clop-console || clean_up
qmake || clean_up
make || clean_up
cp clop-console "${SCRIPT_DIR}" || clean_up
make clean || clean_up
rm Makefile || clean_up

# build CLOP GUI
cd clop_src/programs/clop/compqt/clop-gui || clean_up
qmake || clean_up
make || clean_up
cp clop-console "${SCRIPT_DIR}" || clean_up
make clean || clean_up
rm Makefile || clean_up