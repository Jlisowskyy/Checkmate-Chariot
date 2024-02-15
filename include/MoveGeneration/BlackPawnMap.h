//
// Created by Jlisowskyy on 2/14/24.
//

#ifndef BLACKPAWNMAP_H
#define BLACKPAWNMAP_H

#include <cstdint>

#include "MoveGeneration.h"

class BlackPawnMap {
    // ------------------------------
    // Class creation
    // ------------------------------
public:
    BlackPawnMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetAttackFields(const uint64_t pawnBits) {
        const uint64_t leftAttack = (LeftMask & pawnBits) >> 9;
        const uint64_t rightAttack = (RightMask & pawnBits) >> 7;
        return leftAttack | rightAttack;
    }

    // Returns all moves excepts ElPassantOnes
    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbPos, const uint64_t fullMap, const uint64_t enemyMap) {
        const uint64_t pawnBit = maxMsbPossible >> msbPos;
        const uint64_t attackMoves = GetAttackFields(pawnBit) & enemyMap;
        const uint64_t frontMove = (pawnBit >> 8) & ~fullMap;

        const uint64_t isOnStartField = pawnBit & StartMask;
        const uint64_t isNotBlockedOnFirstMove = (~fullMap) << 8;
        const uint64_t frontDoubleMove = (isOnStartField & isNotBlockedOnFirstMove) >> 16;

        return attackMoves | frontMove | frontDoubleMove;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint64_t PromotingMask = GenMask(8, 16, 1);

private:

    // Mask with ones only on 'Ax" line
    static constexpr uint64_t LeftMask = ~GenMask(0,57, 8);

    // Mask with ones only on "Hx" line
    static constexpr uint64_t RightMask = ~GenMask(7, 64, 8);

    // Mask with ones only on "x7" line
    static constexpr uint64_t StartMask = GenMask(48, 56, 1);

};

#endif //BLACKPAWNMAP_H
