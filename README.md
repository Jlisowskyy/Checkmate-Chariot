
# Checkmate Chariot

## Table of Contents
1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Installation](#installation)
3. [Roadmap](#features)
4. [License](#license)
## Introduction

This project is a unique implementation of a chess engine, developed by Jakub Lisowski, Łukasz Kryczka, and Jakub Pietrzak from the Warsaw University of Technology. It serves as our primary focus and source of enjoyment in programming, providing a platform for exploring various intriguing programming schemes and algorithms. Our goal is to create the most powerful algorithm we can while ensuring well-tested software. While we strive to incorporate our own ideas and solutions, we occasionally draw inspiration from state-of-the-art concepts. This README will guide you through the key aspects of our engine, including its unique features and functionalities.

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
## Features

### Board Representation
- **Bitboards:** Utilizes bitboard structures for efficient board state representation.
- **Supports FEN Strings:** Loads board positions from FEN strings for flexible game setup.
- **Move Generation:** Implements comprehensive move generation including all standard chess moves, castling, en passant, and promotions.
- **Board State Management:** Maintains current board state with functionalities to make and unmake moves.

### Board Evaluation - a feature that is currently worked on the most
- **Material Count:** Considers the material balance of pieces.
- **Special Pawn Evaluation:** Includes specific evaluations for passed, isolated, and doubled pawns.
- **King Safety:** Evaluates attacked squares around kings.
- **Separate Middlegame and Endgame Tables:** Uses distinct evaluation metrics for different game phases.

### Opening Book
- **Predefined Openings:** Contains a rich database of opening moves from professional games.
- **Move Selection:** Selects optimal opening moves to transition into a favorable middle game.
- **Learning Capabilities:** Can update the opening book based on game outcomes to improve future performance.
- The engine starts with a random move from the opening book

### Best Move Search
- **Negamax Algorithm:** Implements the negamax variant of the minimax algorithm for simplicity.
- **Alpha-Beta Pruning:** Uses alpha-beta pruning to reduce the number of nodes evaluated in the search tree.
- **Iterative Deepening:** Implements iterative deepening to progressively deepen the search depth.
- **Transposition Tables:** Utilizes transposition tables to store and retrieve previously evaluated positions.
- **Quiescence Search:** Extends search in tactical positions to avoid horizon effect.
- **Advanced Pruning Techniques:** Includes null move pruning, reverse futility pruning, and delta pruning.
- **Late Move Reductions:** Reduces the search depth for late moves to improve efficiency.
- **Principal Variation Search:** Focuses on the most promising moves to find the best line of play.

### Move Sorting and Evaluation
- **Move Ordering:** Sorts moves to prioritize captures, checks, and promotions.
- **MVV-LVA:** Uses Most Valuable Victim - Least Valuable Attacker heuristic for move ordering.
- **Killer Moves:** Remembers two killer moves per depth to enhance search efficiency.
- **History Heuristic:** Sorts moves based on their historical effectiveness.

### Time Management
- **Dynamic Time Allocation:** Adjusts time usage based on the number of moves played and remaining time.
- This is currently work-in-progress

### UCI Interface
- **Supported Commands:** Full UCI implementation
- **Hash Size Configuration:** Allows setting the hash size via the "Hash" UCI option, with a default of 256 MB.

## License

Distributed under the MIT License. See `LICENSE.txt` for more information.
