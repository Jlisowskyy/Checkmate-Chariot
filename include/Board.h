//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <bitset>
#include <cinttypes>

#include "BitOperations.h"

enum Color : int
{
    WHITE,
    BLACK,
};

enum ColorlessDescriptors : size_t
{
    pawnsIndex,
    knightsIndex,
    bishopsIndex,
    rooksIndex,
    queensIndex,
    kingIndex,
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

struct Board
{
    // ------------------------------
    // class interaction
    // ------------------------------

    void ChangePlayingColor() { movColor ^= 1; }

    [[nodiscard]] constexpr int GetKingMsbPos(const int col) const
    {
        return ExtractMsbPos(boards[col * BoardsPerCol + kingIndex]);
    }

    static bool Comp(const Board &a, const Board &b);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr size_t BoardsCount             = 12;
    static constexpr size_t CastlingCount           = 4;
    static constexpr size_t BoardFields             = 64;
    static constexpr size_t BoardsPerCol            = 6;
    static constexpr size_t KingPosCount            = 2;
    static constexpr size_t CastlingsPerColor       = 2;
    static constexpr uint64_t InvalidElPassantField = 1;
    static constexpr uint64_t InvalidElPassantBoard = maxMsbPossible >> InvalidElPassantField;
    static constexpr size_t SentinelBoardIndex      = 12;
    static constexpr size_t SentinelCastlingIndex   = 4;

    static constexpr std::array<uint64_t, KingPosCount> DefaultKingBoards{
        maxMsbPossible >> ConvertToReversedPos(4), maxMsbPossible >> ConvertToReversedPos(60)
    };
    static constexpr std::array<int, CastlingCount> CastlingNewKingPos{
        ConvertToReversedPos(6), ConvertToReversedPos(2), ConvertToReversedPos(62), ConvertToReversedPos(58)
    };

    static constexpr std::array<uint64_t, CastlingCount> CastlingsRookMaps{1LLU << 7, 1LLU, 1LLU << 63, 1LLU << 56};

    static constexpr std::array<uint64_t, CastlingCount> CastlingNewRookMaps{
        1LLU << 5, 1LLU << 3, 1LLU << 61, 1LLU << 59
    };

    static constexpr std::array<uint64_t, CastlingCount> CastlingSensitiveFields{
        1LLU << 6 | 1LLU << 5, 1LLU << 2 | 1LLU << 3, 1LLU << 61 | 1LLU << 62, 1LLU << 58 | 1LLU << 59
    };

    static constexpr std::array<uint64_t, CastlingCount> CastlingTouchedFields{
        1LLU << 6 | 1LLU << 5, 1LLU << 2 | 1LLU << 3 | 1LLU << 1, 1LLU << 61 | 1LLU << 62,
        1LLU << 58 | 1LLU << 59 | 1LLU << 57
    };

    std::bitset<CastlingCount + 1> Castlings{0}; // additional sentinel field
    uint64_t elPassantField          = maxMsbPossible >> InvalidElPassantField;
    int movColor                     = WHITE;
    uint64_t boards[BoardsCount + 1] = {}; // additional sentinel board
};

#endif // BOARD_H
