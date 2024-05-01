
# Checkmate Chariot

## Table of Contents
1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Installation](#installation)
3. [Roadmap](#roadmap)
4. [License](#license)
## Introduction

This project is a unique implementation of a chess engine, serving as my primary focus and source of enjoyment in programming.
It's a platform for exploring various intriguing programming schemes and algorithms.
The goal is to create the most powerful algorithm I can, while ensuring well-tested software.
While I strive to incorporate my own ideas and solutions, I occasionally draw inspiration from state-of-the-art ideas.

## Getting Started

### Prerequisites

- OpenMp - is used to parallelize map parameter search
- GCC compiler
- x86_64 platform 
- linux environment - to correctly run tests

### Installation

Clone the repository to the desired directory:
```shell
   git clone https://github.com/Jlisowskyy/Checkmate-Chariot ; cd Checkmate-Chariot
```

Use a command below to build the project:
```shell
   cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=RELEASE 
```

## Roadmap

Progress in this repository may be slower due to demanding university duties.
However, plans include the incorporation of the following features:

- [x] Working UCI interface
- [x] Rook and Bishop moves generation implemented and tested
- [ ] Move generation implemented and well tested
- [ ] Opening book implemented and collected
- [ ] Search algorithm implemented and well tested
- [ ] Parallel implementation of search algorithm
- [ ] Evaluation stage of the board implemented

## License

Distributed under the MIT License. See `LICENSE.txt` for more information.
