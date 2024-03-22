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

    explicit ZobristHasher(uint64_t engineSeed);

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] uint64_t GenerateHash(const Board& board) const;

    [[nodiscard]] uint64_t __attribute__((always_inline)) UpdateHash(uint64_t oldHash, const Move mv, const uint64_t oldElPassant,
                                      const std::bitset<Board::CastlingCount+1> oldCastlings) const
    {
        oldHash ^= _colorHash; //swapping collor
        oldHash ^= _mainHashes[mv.GetStartBoardIndex()][mv.GetStartField()]; // placing figure on target square
        oldHash ^= _mainHashes[mv.GetTargetBoardIndex()][mv.GetTargetField()]; // removing figure from start square
        oldHash ^= _mainHashes[mv.GetKilledBoardIndex()][mv.GetKilledFigureField()]; // removing killed figure from board

        oldHash ^= _elPassantHashes[ExtractMsbPos(oldElPassant)]; // removing old elPassantField
        oldHash ^= _elPassantHashes[mv.GetElPassantField()]; // placing new elPassantFiled

        oldHash ^= _castlingHashes[oldCastlings.to_ullong()]; // removing old castlings
        oldHash ^= _castlingHashes[mv.GetCastlingRights().to_ullong()]; // placing new ones

        return oldHash;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    // static constexpr uint64_t BaseSeed = 0x42731294812LLU;
    static constexpr uint64_t BaseSeed = 0x194814141LLU;
private:
    static constexpr size_t CastlingHashesCount = 32; // 2^(4 + 1) each castling property can be either 1 or 0
                                                        // and additional sentinel
    uint64_t _mainHashes[Board::BoardsCount+1][Board::BoardFields]{};
    uint64_t _colorHash{};
    uint64_t _castlingHashes[CastlingHashesCount]{};
    uint64_t _elPassantHashes[Board::BoardFields]{};
};

extern ZobristHasher ZHasher;

#endif //ZOBRISTHASH_H
