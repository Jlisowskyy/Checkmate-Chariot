//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef PAWNMAP_H
#define PAWNMAP_H

#include <cstdint>

#include "MoveGeneration.h"

class WhitePawnMap {
    // ------------------------------
    // Class creation
    // ------------------------------
public:
    WhitePawnMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetAttackFields(const uint64_t pawnBits) {
        const uint64_t leftAttack = (LeftMask & pawnBits) << 7;
        const uint64_t rightAttack = (RightMask & pawnBits) << 9;
        return leftAttack | rightAttack;
    }

    // Returns all moves excepts ElPassantOnes
    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbPos, const uint64_t fullMap, const uint64_t enemyMap) {
        const uint64_t pawnBit = maxMsbPossible >> msbPos;
        const uint64_t attackMoves = GetAttackFields(pawnBit) & enemyMap;
        const uint64_t frontMove = (pawnBit << 8) & ~fullMap;

        const uint64_t isOnStartField = ((frontMove >> 8) & pawnBit & StartMask) << 16;
        const uint64_t frontDoubleMove = isOnStartField & ~fullMap;

        return attackMoves | frontMove | frontDoubleMove;
    }

    [[nodiscard]] static constexpr uint64_t GetElPassantSuspectedFields(const uint64_t elPassantField) {
        const uint64_t leftField = (LeftMask & elPassantField) >> 1;
        const uint64_t righField = (RightMask & elPassantField) << 1;
        return leftField | righField;
    }

    [[nodiscard]] static constexpr uint64_t GetElPassantMoveField(const uint64_t elPassantField) {
        return elPassantField >> 8;
    }

    [[nodiscard]] static Field GetElPassantField(const uint64_t moveField, const uint64_t allMovesMap) {
        return static_cast<Field>((moveField & ElPassantMask & (allMovesMap << 8)));
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint64_t PromotingMask = GenMask(48, 56, 1);
    static constexpr uint64_t ElPassantMask = GenMask(24, 32, 1);
    // Mask with ones only on "x2" line
    static constexpr uint64_t StartMask = GenMask(8, 16, 1);

private:

    // Mask with ones only on 'Ax" line
    static constexpr uint64_t LeftMask = ~GenMask(0,57, 8);

    // Mask with ones only on "Hx" line
    static constexpr uint64_t RightMask = ~GenMask(7, 64, 8);

};


#endif //PAWNMAP_H
