#!/bin/bash

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
make

cd "$start_path" || clean_up
./FastSearchTest.py Checkmate-Chariot || clean_up


echo "Success..."
