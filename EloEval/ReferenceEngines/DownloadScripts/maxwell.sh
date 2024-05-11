#!/bin/bash

git clone https://github.com/eboatwright/Maxwell.git
cd Maxwell
git checkout 28cca0397b822237aaea42aceee990c118198b7d
cargo build --release
mv target/release/maxwell ../../Exes/
cd ../
rm -rf Maxwell
