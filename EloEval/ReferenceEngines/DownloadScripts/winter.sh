#!/bin/bash

git clone https://github.com/rosenthj/Winter.git
cd Winter
git checkout 3ac0a0ff0f6f11a456b2b9c987c3ca30fa98d029
make
mv Winter ../../Exes/winter
cd ..
rm -rf Winter
