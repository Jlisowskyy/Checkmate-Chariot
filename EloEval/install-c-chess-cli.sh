#!/bin/bash

git clone https://github.com/lucasart/c-chess-cli.git c-chess-cli-src
cd c-chess-cli-src
python make.py
mv c-chess-cli ../c-chess-cli
cd ../
rm -rf c-chess-cli-src
