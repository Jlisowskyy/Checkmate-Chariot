#!/bin/bash

#     IMPORTANT: Your pwd needs to be **./EloEval
#
#     Scripts automatically builds two version of Checkmate-Chariot
#     and sets up small tournament between the versions.
#     First build is specified by given hash commit as an command line argument,
#     the other one is simply built based on files that are currently present in the main repo
#     directory.
#
#     So if you need to test your changes on your branch, simply checkout to the desired branch
#     and run this script without any other constraints, when you want to check it against some desired version
#     simply pass and commit has as a command line argument e.g cd EloEval ; RunVersionTest.sh e368a960690d933e544c1d79fe0a6fb747c99390

start_path=`pwd`

clean_up(){
  echo "Failed..."
  cd "$start_path" || exit 1
  exit 1
}

if [ -z "$1" ]; then
  version="HEAD"
else
  version="$1"
fi

./ReferenceEngines/DownloadScripts/Checkmate-Chariot.sh "$version" || clean_up
cd .. || clean_up
cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release || clean_up
make -j `nproc`

cd "$start_path" || clean_up
./FastSearchTest.py Checkmate-Chariot || clean_up


echo "Success..."
