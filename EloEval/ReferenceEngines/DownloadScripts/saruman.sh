#!/bin/bash
git clone https://github.com/ConorGriffin37/chess.git saruman
cd saruman || exit
git checkout fb043b66cf6b8642cc1a32b8f2fbc908273f1733
cd engine || exit
make all
mv Saruman ../../../Exes/saruman
cd ../../
rm -rf saruman