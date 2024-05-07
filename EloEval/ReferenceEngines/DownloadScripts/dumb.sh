#!/bin/bash

sudo pacman -S --noconfirm gdc
git clone https://github.com/abulmo/Dumb.git
cd Dumb
git checkout b67b883
cd src
make pgo DC=gdc
mv dumb ../../../Exes/
cd ../../
rm -rf Dumb
