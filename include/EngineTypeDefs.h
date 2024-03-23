//
// Created by Jlisowskyy on 12/27/23.
//

#ifndef ENGINETYPEDEFS_H
#define ENGINETYPEDEFS_H

#include <cstdint>
#include <unordered_map>

#include "Board.h"

constexpr int SwapColor(const int col) { return col ^ 1; }

void DisplayMask(uint64_t mask);
void DisplayBoard(const Board& bd);

// returns 0 if invalid characters were passed
uint64_t ExtractPosFromStr(int x, int y);

// in case of invalid encoding returns 0 in the corresponding value, if string is to short returns (0, 0).
// return scheme: [ oldMap, newMap ]
std::pair<uint64_t, uint64_t> ExtractPositionsFromEncoding(const std::string& encoding);

// Move to strings converting functions
std::pair<char, char> ConvertToCharPos(int boardPosMsb);
std::string ConvertToStrPos(int boardPosMsb);
std::pair<char, char> ConvertToCharPos(uint64_t boardMap);
std::string ConvertToStrPos(uint64_t boardMap);

// global defined stack capacity used to store generated moves per thread
static constexpr size_t MB = 1024 * 1024;
static constexpr size_t DefaultStackSize = 32 * MB / sizeof(uint64_t);
static constexpr int MaxSearchDepth = 256;

extern const char IndexToFigCharMap[Board::BoardsCount];
extern const std::unordered_map<char, size_t> FigCharToIndexMap;

#endif  // ENGINETYPEDEFS_H
