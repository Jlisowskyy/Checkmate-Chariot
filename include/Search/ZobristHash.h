//
// Created by Jlisowskyy on 3/12/24.
//

#ifndef ZOBRISTHASH_H
#define ZOBRISTHASH_H

#include <cinttypes>

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"

/*  Class responsible for hashing the whole board into
 *  unique number, which special property that similar
 *  position receives completely distinct numbers
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

    [[nodiscard]] uint64_t GenerateHash(const Board &board) const;

    [[nodiscard]] INLINE uint64_t UpdateHash(uint64_t oldHash, const Move mv, const VolatileBoardData &data) const
    {
        oldHash ^= _colorHash;                                                 // swapping color
        oldHash ^= _mainHashes[mv.GetStartBoardIndex()][mv.GetStartField()];   // placing a figure on target square
        oldHash ^= _mainHashes[mv.GetTargetBoardIndex()][mv.GetTargetField()]; // removing a figure from start square
        oldHash ^=
            _mainHashes[mv.GetKilledBoardIndex()][mv.GetKilledFigureField()]; // removing a killed figure from the board

        oldHash ^= _elPassantHashes[ExtractMsbPos(data.OldElPassant)]; // removing old ElPassantField
        oldHash ^= _elPassantHashes[mv.GetElPassantField()];           // placing new elPassantFiled

        oldHash ^= _castlingHashes[data.Castlings.to_ullong()];         // removing old castlings
        oldHash ^= _castlingHashes[mv.GetCastlingRights().to_ullong()]; // placing new ones

        return oldHash;
    }

    [[nodiscard]] bool ValidateQuality(int diffBits, bool log = false) const;
    [[nodiscard]] static uint64_t SearchForSeed(uint64_t startSeed, int bitDiffs, bool log = false);

    void RollParameters(uint64_t seed);

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    [[nodiscard]] static bool _validateNum(uint64_t a, uint64_t b, int diffBits, int &sameCount, bool log = false);

    [[nodiscard]] bool _validateCase(uint64_t num, int diffBits, bool log = false) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    public:
    static constexpr uint64_t BaseSeed = 7084466601545828732LLU; // 17 unique bits guaranteed by this seed
    static constexpr int MaxRetries    = MinMsbPossible << 24;

    private:
    static constexpr size_t CastlingHashesCount = 32; // 2^(4 + 1) each castling property can be either 1 or 0
                                                      // and additional sentinel
    uint64_t _mainHashes[Board::BitBoardsCount + 1][Board::BitBoardFields]{};
    uint64_t _colorHash{};
    uint64_t _castlingHashes[CastlingHashesCount]{};
    uint64_t _elPassantHashes[Board::BitBoardFields]{};
};

extern ZobristHasher ZHasher;

#endif // ZOBRISTHASH_H
