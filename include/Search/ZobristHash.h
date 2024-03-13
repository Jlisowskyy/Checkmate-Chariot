//
// Created by Jlisowskyy on 3/12/24.
//

#ifndef ZOBRISTHASH_H
#define ZOBRISTHASH_H

#include <cinttypes>

#include "../EngineTypeDefs.h"
#include "../MoveGeneration/Move.h"

/*  Class responsible for hashing whole board into
 *  unique number, which special property that similar
 *  position recevies completly distrinct numbers
 */

struct ZobristHasher
{
    // ------------------------------
    // Class creation
    // ------------------------------

    explicit ZobristHasher(uint64_t engineSeed = Seed);

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] uint64_t GenerateHash(const Board& board) const;

    [[nodiscard]] uint64_t UpdateHash(uint64_t oldHash, Move mv, uint64_t oldElPassant,
                                      std::bitset<Board::CastlingCount+1> oldCastlings) const;

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    static constexpr uint64_t Seed = 0x42731294812LLU; // Hard coded seed to enforce reproducability
    static constexpr size_t CastlingHashesCount = 32; // 2^(4 + 1) each castling property can be either 1 or 0
                                                        // and additional sentinel

    uint64_t _mainHashes[Board::BoardsCount+1][Board::BoardFields]{};
    uint64_t _colorHash{};
    uint64_t _castlingHashes[CastlingHashesCount]{};
    uint64_t _elPassantHashes[Board::BoardFields]{};
};

extern ZobristHasher ZHasher;

#endif //ZOBRISTHASH_H
