//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <bitset>
#include <cinttypes>
#include <unordered_map>

#include "BitOperations.h"

/*
 * Given enum defines values and order of both colors. All indexing schemes used across the projects follows given
 * order. It is important to keep it consistent across the project. It is also very useful when performing some indexing
 * and color switching operations.
 * */

enum Color : int
{
    WHITE,
    BLACK,
};

/*
 * Given enum defines indexing order of all BitBoards inside the board.
 * It is important to keep this order consistent across the project.
 * Again very useful when performing some indexing and color switching operations.
 * */

enum ColorlessDescriptors : size_t
{
    pawnsIndex,
    knightsIndex,
    bishopsIndex,
    rooksIndex,
    queensIndex,
    kingIndex,
};

/*
 * Given enum defines indexing order of all (color, piece) BitBoards inside the board.
 * It is important to keep it consistent across the project.
 * Used rather less frequently than previous ones but still defines order of all bitboards.
 * */

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

/*
 * Defines the order of castling indexes for given color.
 * */

enum CastlingIndexes : size_t
{
    KingCastlingIndex,
    QueenCastlingIndex,
};

/*
 * Defines indexes of all castling possibilities.
 * */

enum CastlingPossibilities : size_t
{
    WhiteKingSide,
    WhiteQueenSide,
    BlackKingSide,
    BlackQueenSide,
};

/*
 *  The most important class used around the project.
 *  It defines representation of the board state.
 *  Currently, it consists of:
 *      - BitBoards: 12 bitboards representing all pieces of both colors with one additional sentinel board at the end.
 *        Such representation allows to easily iterate over all pieces of given color and perform operation with very
 * fast bit operations. Additionally, sentinel allows to unconditionally treats all move types without any additional
 * checks.
 *      - Single ElPassantField: 64-bit integer representing field where en passant is possible.
 *      - Single MovingColor: integer representing color of the player who is currently moving.
 *      - Castlings: bitset representing all castling possibilities for both colors with one additional sentinel field
 * at the end.
 *
 * */

struct Board
{
    // ------------------------------
    // Class creation
    // ------------------------------

    Board()  = default;
    ~Board() = default;

    Board(const Board &)            = default;
    Board &operator=(const Board &) = default;

    // ------------------------------
    // class interaction
    // ------------------------------

    void ChangePlayingColor() { MovingColor ^= 1; }

    [[nodiscard]] constexpr int GetKingMsbPos(const int col) const
    {
        return ExtractMsbPos(BitBoards[col * BitBoardsPerCol + kingIndex]);
    }

    static bool Comp(const Board &a, const Board &b);

    constexpr uint64_t GetFigBoard(int col, size_t figDesc) const { return BitBoards[col * BitBoardsPerCol + figDesc]; }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr size_t BitBoardsCount             = 12;
    static constexpr size_t CastlingCount              = 4;
    static constexpr size_t BitBoardFields             = 64;
    static constexpr size_t BitBoardsPerCol            = 6;
    static constexpr size_t KingPosCount               = 2;
    static constexpr size_t CastlingsPerColor          = 2;
    static constexpr uint64_t InvalidElPassantField    = 1;
    static constexpr uint64_t InvalidElPassantBitBoard = MaxMsbPossible >> InvalidElPassantField;
    static constexpr size_t SentinelBoardIndex         = 12;
    static constexpr size_t SentinelCastlingIndex      = 4;

    static constexpr std::array<uint64_t, KingPosCount> DefaultKingBoards{
        MaxMsbPossible >> ConvertToReversedPos(4), MaxMsbPossible >> ConvertToReversedPos(60)
    };
    static constexpr std::array<int, CastlingCount> CastlingNewKingPos{
        ConvertToReversedPos(6), ConvertToReversedPos(2), ConvertToReversedPos(62), ConvertToReversedPos(58)
    };

    static constexpr std::array<uint64_t, CastlingCount> CastlingsRookMaps{
        MinMsbPossible << 7, MinMsbPossible, MinMsbPossible << 63, MinMsbPossible << 56
    };

    static constexpr std::array<uint64_t, CastlingCount> CastlingNewRookMaps{
        MinMsbPossible << 5, MinMsbPossible << 3, MinMsbPossible << 61, MinMsbPossible << 59
    };

    static constexpr std::array<uint64_t, CastlingCount> CastlingSensitiveFields{
        MinMsbPossible << 6 | MinMsbPossible << 5, MinMsbPossible << 2 | MinMsbPossible << 3,
        MinMsbPossible << 61 | MinMsbPossible << 62, MinMsbPossible << 58 | MinMsbPossible << 59
    };

    static constexpr std::array<uint64_t, CastlingCount> CastlingTouchedFields{
        MinMsbPossible << 6 | MinMsbPossible << 5, MinMsbPossible << 2 | MinMsbPossible << 3 | MinMsbPossible << 1,
        MinMsbPossible << 61 | MinMsbPossible << 62, MinMsbPossible << 58 | MinMsbPossible << 59 | MinMsbPossible << 57
    };

    // --------------------------------
    // Main processing components
    // --------------------------------

    std::bitset<CastlingCount + 1> Castlings           = {0}; // additional sentinel field
    uint64_t ElPassantField                            = MaxMsbPossible >> InvalidElPassantField;
    int MovingColor                                    = WHITE;
    std::array<uint64_t, BitBoardsCount + 1> BitBoards = {}; // additional sentinel board

    // --------------------------------------
    // Draw and state monitoring fields
    // --------------------------------------

    int HalfMoves                                 = {};
    uint16_t Age                                  = {}; // stores total half moves since the beginning of the game
    std::unordered_map<uint64_t, int> Repetitions = {}; // Stores hashes of previous encountered positions
};

#endif // BOARD_H
