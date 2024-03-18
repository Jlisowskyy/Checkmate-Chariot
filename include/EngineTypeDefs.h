//
// Created by Jlisowskyy on 12/27/23.
//

#ifndef ENGINETYPEDEFS_H
#define ENGINETYPEDEFS_H

#include <cstdint>
#include <unordered_map>

#include "Board.h"

enum MoveTypes
{
    NormalMove,
    PromotingMove
};

enum Descriptors : size_t
{
    wPawnsIndex,
    wKnightsIndex,
    wBishopsIndex,
    wRooksIndex,
    wQueensIndex,
    wKingIndex,
    bPawnsIndex,
    bKnightsIndex,
    bBishopsIndex,
    bRooksIndex,
    bQueensIndex,
    bKingIndex,
};

enum Field : uint64_t
{
    INVALID = 0,
    A1 = 1LLU,
    B1 = 1LLU << 1,
    C1 = 1LLU << 2,
    D1 = 1LLU << 3,
    E1 = 1LLU << 4,
    F1 = 1LLU << 5,
    G1 = 1LLU << 6,
    H1 = 1LLU << 7,
    A2 = 1LLU << 8,
    B2 = 1LLU << 9,
    C2 = 1LLU << 10,
    D2 = 1LLU << 11,
    E2 = 1LLU << 12,
    F2 = 1LLU << 13,
    G2 = 1LLU << 14,
    H2 = 1LLU << 15,
    A3 = 1LLU << 16,
    B3 = 1LLU << 17,
    C3 = 1LLU << 18,
    D3 = 1LLU << 19,
    E3 = 1LLU << 20,
    F3 = 1LLU << 21,
    G3 = 1LLU << 22,
    H3 = 1LLU << 23,
    A4 = 1LLU << 24,
    B4 = 1LLU << 25,
    C4 = 1LLU << 26,
    D4 = 1LLU << 27,
    E4 = 1LLU << 28,
    F4 = 1LLU << 29,
    G4 = 1LLU << 30,
    H4 = 1LLU << 31,
    A5 = 1LLU << 32,
    B5 = 1LLU << 33,
    C5 = 1LLU << 34,
    D5 = 1LLU << 35,
    E5 = 1LLU << 36,
    F5 = 1LLU << 37,
    G5 = 1LLU << 38,
    H5 = 1LLU << 39,
    A6 = 1LLU << 40,
    B6 = 1LLU << 41,
    C6 = 1LLU << 42,
    D6 = 1LLU << 43,
    E6 = 1LLU << 44,
    F6 = 1LLU << 45,
    G6 = 1LLU << 46,
    H6 = 1LLU << 47,
    A7 = 1LLU << 48,
    B7 = 1LLU << 49,
    C7 = 1LLU << 50,
    D7 = 1LLU << 51,
    E7 = 1LLU << 52,
    F7 = 1LLU << 53,
    G7 = 1LLU << 54,
    H7 = 1LLU << 55,
    A8 = 1LLU << 56,
    B8 = 1LLU << 57,
    C8 = 1LLU << 58,
    D8 = 1LLU << 59,
    E8 = 1LLU << 60,
    F8 = 1LLU << 61,
    G8 = 1LLU << 62,
    H8 = 1LLU << 63
};

enum CastlingIndexes : size_t
{
    KingCastlingIndex,
    QueenCastlingIndex,
};

enum CastlingPossibilities : size_t
{
    WhiteKingSide,
    WhiteQueenSide,
    BlackKingSide,
    BlackQueenSide,
};

constexpr int SwapColor(const int col) { return col ^ 1; }

void DisplayMask(uint64_t mask);
void DisplayBoard(const Board& bd);

std::tuple<uint64_t, uint64_t, MoveTypes> FindMove(const Board& oldBoard, const Board& newBoard);

std::string GetEncodingFromBoards(const Board& bd, uint64_t oldMap, uint64_t newMap, MoveTypes mType);

std::string GetLongAlgebraicMoveEncoding(const Board& oBoard, const Board& nBoard);

// returns 0 if invalid characters were passed
uint64_t ExtractPosFromStr(int x, int y);

// in case of invalid encoding returns 0 in corresponding value, if string is to short returns (0, 0).
// return scheme: [ oldMap, newMap ]
std::pair<uint64_t, uint64_t> ExtractPositionsFromEncoding(const std::string& encoding);

// global defined stack capacity used to store generated moves per thread
static constexpr size_t MB = 1024 * 1024;
static constexpr size_t DefaultStackSize = 32 * MB / sizeof(uint64_t);
static constexpr int MaxSearchDepth = 256;

extern const std::unordered_map<size_t, Descriptors> figToDescMap;
extern const std::unordered_map<size_t, char> descToFigMap;
extern const std::unordered_map<Field, std::string> fieldStrMap;
extern const std::unordered_map<std::string, Field> strFieldMap;

#endif  // ENGINETYPEDEFS_H
