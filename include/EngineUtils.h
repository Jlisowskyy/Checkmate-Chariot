//
// Created by Jlisowskyy on 12/27/23.
//

#ifndef ENGINETYPEDEFS_H
#define ENGINETYPEDEFS_H

#include <chrono>
#include <climits>
#include <cstdint>
#include <numeric>
#include <unordered_map>

#include "Board.h"
#include "CompilationConstants.h"

/*
 * Header gather various simple functions with different purposes,
 * that are not directly connected with any class inside the project.
 *
 * */

constexpr int SwapColor(const int col) { return col ^ 1; }

/* Should not be used when "singleBitBitMap" has more than one bit positive */
constexpr uint64_t RemovePiece(uint64_t &bitMap, uint64_t singleBitBitMap) { return bitMap ^= singleBitBitMap; }

constexpr uint64_t RemovePiece(uint64_t &bitMap, int msbPos) { return bitMap ^= (MaxMsbPossible >> msbPos); }

constexpr uint64_t ConvertMsbPosToBitMap(int msbPos) { return (MaxMsbPossible >> msbPos); }

/* functions returns 'moves' that are present on 'allowedMoves' BitMap */
constexpr uint64_t FilterMoves(uint64_t moves, uint64_t allowedMoves) { return moves & allowedMoves; }

// ------------------------------
// Those functions below allows to perform aligned alloc operations without any problems on windows+linux
// Because default windows implementation of C++ library does not support std::aligned_alloc function

void *AlignedAlloc(size_t alignment, size_t size);
void AlignedFree(void *ptr);

// ------------------------------

/* Function simply prints given uint64_t as 8x8 block of 'x' chars when there is positive bit */
/* Prints it according to internal BitBoard indexing order */
void DisplayMask(uint64_t mask);

/* Function simply pretty prints the given board */
void DisplayBoard(const Board &bd);

/* Function simply returns sting containing information about the current datetime, mostly used in logging*/
std::string GetCurrentTimeStr();

/* Functions extract the name of file given inside the 'path' variable */
const char *GetFileName(const char *path);

// returns 0 if invalid characters were passed
uint64_t ExtractPosFromStr(int x, int y);

// in case of invalid encoding returns 0 in the corresponding value, if string is to short returns (0, 0).
// return scheme: [ oldMap, newMap ]
std::pair<uint64_t, uint64_t> ExtractPositionsFromEncoding(const std::string &encoding);

// Move to strings converting functions
std::pair<char, char> ConvertToCharPos(int boardPosMsb);

/* Function converts the given integer to string encoding the field accordingly to typical chess notation [A-H][1-8]*/
std::string ConvertToStrPos(int boardPosMsb);

/* Does same thing as function above, but additionally extracts the MsbPos from the BitBoard */
std::string ConvertToStrPos(uint64_t boardMap);

/* Table maps BitBoard index to character representing corresponding chess figure */
extern const char IndexToFigCharMap[Board::BitBoardsCount];

/* Does the same thing as above but in reverse */
extern const std::unordered_map<char, size_t> FigCharToIndexMap;

// Structure stores time information that were parsed and should be passed to 'go' search function
struct GoTimeInfo
{
    bool operator==(const GoTimeInfo &rhs) const = default;

    static constexpr lli Infinite = std::numeric_limits<lli>::max();
    static constexpr lli NotSet   = Infinite;

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
    static constexpr int NotSet = std::numeric_limits<int>::max();

    bool operator==(const GoInfo &rhs) const = default;

    GoTimeInfo timeInfo{};
    int depth{NotSet};
    bool isPonderSearch {false};
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
