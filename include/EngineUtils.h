//
// Created by Jlisowskyy on 12/27/23.
//

#ifndef ENGINETYPEDEFS_H
#define ENGINETYPEDEFS_H

#include <cstdint>
#include <unordered_map>

#include "Board.h"
#include "CompilationConstants.h"

constexpr int SwapColor(const int col) { return col ^ 1; }

constexpr uint64_t RemovePiece(uint64_t &bitMap, uint64_t singleBitBitMap) { return bitMap ^= singleBitBitMap; }

constexpr uint64_t RemovePiece(uint64_t &bitMap, int msbPos) { return bitMap ^= (maxMsbPossible >> msbPos); }

constexpr uint64_t ConvertMsbPosToBitMap(int msbPos) { return (maxMsbPossible >> msbPos); }

constexpr uint64_t FilterMoves(uint64_t moves, uint64_t allowedMoves) { return moves & allowedMoves; }

void *AlignedAlloc(size_t alignment, size_t size);
void AlignedFree(void *ptr);

void DisplayMask(uint64_t mask);
void DisplayBoard(const Board &bd);

// returns 0 if invalid characters were passed
uint64_t ExtractPosFromStr(int x, int y);

// in case of invalid encoding returns 0 in the corresponding value, if string is to short returns (0, 0).
// return scheme: [ oldMap, newMap ]
std::pair<uint64_t, uint64_t> ExtractPositionsFromEncoding(const std::string &encoding);

// Move to strings converting functions
std::pair<char, char> ConvertToCharPos(int boardPosMsb);
std::string ConvertToStrPos(int boardPosMsb);
std::pair<char, char> ConvertToCharPos(uint64_t boardMap);
std::string ConvertToStrPos(uint64_t boardMap);

extern const char IndexToFigCharMap[Board::BitBoardsCount];
extern const std::unordered_map<char, size_t> FigCharToIndexMap;

struct GoTimeInfo
{
    lli wTime{-1};
    lli bTime{-1};
    lli wInc{-1};
    lli bInc{-1};
    lli moveTime{-1};
};

#endif // ENGINETYPEDEFS_H
