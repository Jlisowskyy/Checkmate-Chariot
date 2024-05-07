#!/bin/bash
echo "Installing Snowy"
git clone --recursive https://github.com/JasonCreighton/snowy.git

echo "Building Snowy"
cd snowy
git checkout b0aa777ca69ef3411d9fb531e591306dbfaf6a48
make

echo "Moving Snowy to Exes"
mv build/release ../../Exes/snowy

echo "Cleaning up"
cd ../
rm -rf snowy
