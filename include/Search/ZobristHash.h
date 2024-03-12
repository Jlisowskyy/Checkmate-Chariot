//
// Created by Jlisowskyy on 3/12/24.
//

#ifndef ZOBRISTHASH_H
#define ZOBRISTHASH_H

#include <cinttypes>

#include "../EngineTypeDefs.h"

struct ZobristHasher
{
    // ------------------------------
    // Class creation
    // ------------------------------

    ZobristHasher()
    {

    }

    // ------------------------------
    // Class interaction
    // ------------------------------

    uint64_t GenerateHash(const Board& bd) const
    {

    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    static constexpr uint64_t Seed = 0x42731294812LLU; // Hard coded seed to enforce reproducability
    static constexpr size_t CastlingHashesCount = 16; // 2^4 each castling property can be either 1 or 0

    uint64_t _mainHashes[Board::BoardsCount+1][Board::BoardFields];
    uint64_t _colorHash;
    uint64_t _castlingHashes[CastlingHashesCount];
    uint64_t _elPassantHashes[Board::BoardFields];
};

#endif //ZOBRISTHASH_H
