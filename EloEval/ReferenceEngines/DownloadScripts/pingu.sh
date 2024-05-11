#!/bin/bash

git clone https://github.com/WillChing01/Pingu.git
cd Pingu
git checkout 1a69999
make
mv Pingu ../../Exes/pingu
cd ../
rm -rf Pingu



