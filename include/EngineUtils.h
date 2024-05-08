//
// Created by Jlisowskyy on 12/27/23.
//

#ifndef ENGINETYPEDEFS_H
#define ENGINETYPEDEFS_H

#include <cstdint>
#include <unordered_map>
#include <climits>

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

// Structure stores time information that were parsed and should be passed to 'go' search function
struct GoTimeInfo
{
    bool operator==(const GoTimeInfo &rhs) const = default;

    static constexpr lli Infinite = LONG_LONG_MAX;
    static constexpr lli NotSet = Infinite;

    lli wTime{NotSet};
    lli bTime{NotSet};
    lli wInc{NotSet};
    lli bInc{NotSet};
    lli moveTime{NotSet};

    static GoTimeInfo GetInfiniteTime();

    [[nodiscard]] bool IsColorTimeSet(int color) const;
};


// Structure stores information about search depth and time needed by 'go' function
struct GoInfo
{
    static constexpr int NotSet = INT_MAX;

    bool operator==(const GoInfo &rhs) const = default;

    GoTimeInfo timeInfo{};
    int depth {NotSet};
};

/*
 * Three types of nodes that we can find during the search.
 * To get more information about them, please visit:
 * https://www.chessprogramming.org/Node_Types
 * */

enum nodeType : uint8_t
{
    pvNode,
    lowerBound,
    upperBound
};

#endif // ENGINETYPEDEFS_H
