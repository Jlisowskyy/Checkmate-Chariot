#!/bin/bash
echo "Installing StockFish"
git clone https://github.com/official-stockfish/Stockfish.git

echo "Checking out to 2023-06-13"
cd Stockfish
git checkout 7922e07af83dd472da6e5b38fb84214cfe46a630

echo "Building StockFish"
cd src
make -j profile-build

echo "Copying StockFish"
mv stockfish ../../../Exes

echo "Cleaning up"
cd ../../
rm -rf Stockfish

