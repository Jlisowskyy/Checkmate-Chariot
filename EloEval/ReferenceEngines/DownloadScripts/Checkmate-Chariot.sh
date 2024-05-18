#!/bin/bash

start_path=`pwd`

clean_up(){
  echo "Failed..."
  cd "$start_path" || exit 1
  rm -rf Checkmate-Chariot
}

if [ -z "$1" ]; then
  version="HEAD"
else
  version="$1"
fi

git clone https://github.com/Jlisowskyy/Checkmate-Chariot || exit 1
cd Checkmate-Chariot || exit 1
git checkout "$version" || clean_up
cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release || clean_up
make || clean_up
mv Checkmate-Chariot "$start_path/Exes/" || clean_up
cd "$start_path" || clean_up
rm -rf Checkmate-Chariot

echo "Success..."
