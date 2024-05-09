#!/bin/bash
git clone https://github.com/princesslana/princhess.git
cd princhess
git checkout 49397b8770d159be045feefb6191218d5f45ac1e
cargo build --release
mv target/release/princhess ../../Exes/princhess
cd ..
rm -rf princhess
