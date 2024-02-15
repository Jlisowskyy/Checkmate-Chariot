//
// Created by Jlisowskyy on 12/27/23.
//

#ifndef ENGINETYPEDEFS_H
#define ENGINETYPEDEFS_H

#include <cstdint>
#include <unordered_map>
#include <iostream>
#include <array>

#include "BitOperations.h"

enum MoveTypes {
    NormalMove,
    PromotingMove
};

enum Color: int {
    WHITE,
    BLACK,
};

int SwapColor(int col);

enum ColorlessDescriptors: size_t {
    pawnsIndex,
    knightsIndex,
    bishopsIndex,
    rooksIndex,
    queensIndex,
    kingIndex,
};

enum Descriptors: size_t {
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

enum Field : uint64_t {
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

enum CastlingIndexes: size_t {
    KingCastlingIndex,
    QueenCastlingIndex,
};

enum CastlingPossibilities: size_t {
    WhiteKingSide,
    WhiteQueenSide,
    BlackKingSide,
    BlackQueenSide,
};

struct Board {
    // ------------------------------
    // class interaction
    // ------------------------------

    void ChangePlayingColor() {
        movColor = SwapColor(movColor);
    }

    friend std::ostream& operator<<(std::ostream& out, const Board& bd);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr size_t BoardsCount = 12;
    static constexpr size_t CastlingCount = 4;
    static constexpr size_t BoardFields = 64;
    static constexpr size_t BoardsPerCol = 6;
    static constexpr size_t KingPosCount = 2;
    static constexpr size_t CastlingsPerColor = 2;

    static constexpr std::array<uint8_t, KingPosCount> DefaultKingPos { 4, 60 };
    static constexpr std::array<uint64_t, CastlingCount> CastlingNewKingPos { 6, 2, 62, 58 };

    static constexpr std::array<uint64_t, CastlingCount> CastlingsRookMaps {
        1LLU << 7,
        1LLU,
        1LLU << 63,
        1LLU << 56
    };

    static constexpr std::array<uint64_t, CastlingCount> CastlingNewRookMaps {
        1LLU << 5,
        1LLU << 3,
        1LLU << 61,
        1LLU << 59
    };

    static constexpr std::array<uint64_t, CastlingCount> CastlingSensitiveFields {
        1LLU << 6 | 1LLU << 5,
        1LLU << 2 | 1LLU << 3,
        1LLU << 61 | 1LLU << 62,
        1LLU << 58 | 1LLU << 59
    };

    std::array<bool, CastlingCount> Castlings{ false, false, false, false };
    Field elPassantField = INVALID;
    int movColor = WHITE;
    uint8_t kingMSBPositions[KingPosCount] = { 0 };
    uint64_t boards[BoardsCount] = { 0 };
};

void DisplayMask(uint64_t mask);
std::tuple<uint64_t, uint64_t, MoveTypes> FindMove(const Board& oldBoard, const Board& newBoard);
std::string GetShortAlgebraicMoveEncoding(const Board& bd, uint64_t oldMap, uint64_t newMap, MoveTypes mType);

inline const std::unordered_map<size_t, Descriptors> figToDescMap{
        {'P', wPawnsIndex},
        {'N', wKnightsIndex},
        {'B', wBishopsIndex},
        {'R', wRooksIndex},
        {'Q', wQueensIndex},
        {'K', wKingIndex},
        {'p', bPawnsIndex},
        {'n', bKnightsIndex},
        {'b', bBishopsIndex},
        {'r', bRooksIndex},
        {'q', bQueensIndex},
        {'k', bKingIndex}
};

inline const std::unordered_map<size_t, char> descToFigMap{
        {wPawnsIndex, 'P'},
        {wKnightsIndex, 'N'},
        {wBishopsIndex, 'B'},
        {wRooksIndex, 'R'},
        {wQueensIndex, 'Q'},
        {wKingIndex, 'K'},
        {bPawnsIndex, 'p'},
        {bKnightsIndex, 'n'},
        {bBishopsIndex, 'b'},
        {bRooksIndex, 'r'},
        {bQueensIndex, 'q'},
        {bKingIndex, 'k'}
};

inline const std::unordered_map<Field, std::string> fieldStrMap{
        {A1, "a1"},
        {B1, "b1"},
        {C1, "c1"},
        {D1, "d1"},
        {E1, "e1"},
        {F1, "f1"},
        {G1, "g1"},
        {H1, "h1"},
        {A2, "a2"},
        {B2, "b2"},
        {C2, "c2"},
        {D2, "d2"},
        {E2, "e2"},
        {F2, "f2"},
        {G2, "g2"},
        {H2, "h2"},
        {A3, "a3"},
        {B3, "b3"},
        {C3, "c3"},
        {D3, "d3"},
        {E3, "e3"},
        {F3, "f3"},
        {G3, "g3"},
        {H3, "h3"},
        {A4, "a4"},
        {B4, "b4"},
        {C4, "c4"},
        {D4, "d4"},
        {E4, "e4"},
        {F4, "f4"},
        {G4, "g4"},
        {H4, "h4"},
        {A5, "a5"},
        {B5, "b5"},
        {C5, "c5"},
        {D5, "d5"},
        {E5, "e5"},
        {F5, "f5"},
        {G5, "g5"},
        {H5, "h5"},
        {A6, "a6"},
        {B6, "b6"},
        {C6, "c6"},
        {D6, "d6"},
        {E6, "e6"},
        {F6, "f6"},
        {G6, "g6"},
        {H6, "h6"},
        {A7, "a7"},
        {B7, "b7"},
        {C7, "c7"},
        {D7, "d7"},
        {E7, "e7"},
        {F7, "f7"},
        {G7, "g7"},
        {H7, "h7"},
        {A8, "a8"},
        {B8, "b8"},
        {C8, "c8"},
        {D8, "d8"},
        {E8, "e8"},
        {F8, "f8"},
        {G8, "g8"},
        {H8, "h8"}
};

inline const std::unordered_map<std::string, Field> strFieldMap{
        {"a1", A1},
        {"b1", B1},
        {"c1", C1},
        {"d1", D1},
        {"e1", E1},
        {"f1", F1},
        {"g1", G1},
        {"h1", H1},
        {"a2", A2},
        {"b2", B2},
        {"c2", C2},
        {"d2", D2},
        {"e2", E2},
        {"f2", F2},
        {"g2", G2},
        {"h2", H2},
        {"a3", A3},
        {"b3", B3},
        {"c3", C3},
        {"d3", D3},
        {"e3", E3},
        {"f3", F3},
        {"g3", G3},
        {"h3", H3},
        {"a4", A4},
        {"b4", B4},
        {"c4", C4},
        {"d4", D4},
        {"e4", E4},
        {"f4", F4},
        {"g4", G4},
        {"h4", H4},
        {"a5", A5},
        {"b5", B5},
        {"c5", C5},
        {"d5", D5},
        {"e5", E5},
        {"f5", F5},
        {"g5", G5},
        {"h5", H5},
        {"a6", A6},
        {"b6", B6},
        {"c6", C6},
        {"d6", D6},
        {"e6", E6},
        {"f6", F6},
        {"g6", G6},
        {"h6", H6},
        {"a7", A7},
        {"b7", B7},
        {"c7", C7},
        {"d7", D7},
        {"e7", E7},
        {"f7", F7},
        {"g7", G7},
        {"h7", H7},
        {"a8", A8},
        {"b8", B8},
        {"c8", C8},
        {"d8", D8},
        {"e8", E8},
        {"f8", F8},
        {"g8", G8},
        {"h8", H8}
};

#endif //ENGINETYPEDEFS_H
