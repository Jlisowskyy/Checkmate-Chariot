#!/bin/bash

env CXXFLAGS='-fprofile-dir=pgo/pgoCHESS -fprofile-generate=pgo/pgoCHESS' cmake -DCMAKE_BUILD_TYPE=Release
make

./Checkmate-Chariot "go searchPerf Tests/ProfileSets/set1.csv" "exit"

rm -rf CMakeFiles/
rm cmake_install.cmake
rm Makefile
rm CMakeCache.txt
rm Checkmate-Chariot

env CXXFLAGS='-fprofile-dir=pgo/pgoCHESSo -fprofile-use=pgo/pgoCHESS -fprofile-correction -funroll-loops -fno-peel-loops -fno-tracer' cmake -DCMAKE_BUILD_TYPE=Release
make

rm -rf CMakeFiles/
rm cmake_install.cmake
rm Makefile
rm CMakeCache.txt