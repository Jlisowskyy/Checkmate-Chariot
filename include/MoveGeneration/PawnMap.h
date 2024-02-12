//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef PAWNMAP_H
#define PAWNMAP_H

#include <cstdint>

#include "MoveGeneration.h"


class PawnMap {
    // ------------------------------
    // Class creation
    // ------------------------------
public:
    PawnMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr uint64_t GetBlackAttackFields(const uint64_t pawnBit) {
        const uint64_t leftAttack = (RightMask ^ pawnBit) >> 9;
        const uint64_t rightAttack = (LeftMask ^ pawnBit) >> 7;
        return leftAttack | rightAttack;
    }

    // Returns all moves excepts ElPassantOnes
    [[nodiscard]] static constexpr uint64_t GetBlackMoves(const uint64_t pawnBit, const uint64_t enemyMap, const uint64_t fullMap) {
        const uint64_t attackMoves = GetBlackAttackFields(pawnBit) & enemyMap;
        const uint64_t frontMove = (pawnBit >> 8) ^ fullMap;

        const uint64_t isOnStartField = pawnBit & StartBlackPawnMask;
        const uint64_t isNotBlockedOnFirstMove = fullMap << 8;
        const uint64_t frontDoubleMove = ((isOnStartField & isNotBlockedOnFirstMove) >> 16) ^ fullMap;

        return attackMoves | frontMove | frontDoubleMove;
    }

    [[nodiscard]] static constexpr uint64_t GetWhiteAttackFields(const uint64_t pawnBit) {
        const uint64_t leftAttack = (LeftMask ^ pawnBit) << 7;
        const uint64_t rightAttack = (RightMask ^ pawnBit) << 9;
        return leftAttack | rightAttack;
    }

    // Returns all moves excepts ElPassantOnes
    [[nodiscard]] static constexpr uint64_t GetWhiteMoves(const uint64_t pawnBit, const uint64_t enemyMap, const uint64_t fullMap) {
        const uint64_t attackMoves = GetWhiteAttackFields(pawnBit) & enemyMap;
        const uint64_t frontMove = (pawnBit << 8) ^ fullMap;

        const uint64_t isOnStartField = pawnBit & StartWhitePawnMask;
        const uint64_t isNotBlockedOnFirstMove = fullMap >> 8;
        const uint64_t frontDoubleMove = ((isOnStartField & isNotBlockedOnFirstMove) << 16) ^ fullMap;

        return attackMoves | frontMove | frontDoubleMove;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:

    // Mask with ones only on 'Ax" line
    static constexpr uint64_t LeftMask = GenMask(0,57, 8);

    // Mask with ones only on "Hx" line
    static constexpr uint64_t RightMask = GenMask(7, 64, 8);

    // Mask with ones only on "x2" line
    static constexpr uint64_t StartWhitePawnMask = GenMask(8, 16, 1);

    // Mask with ones only on "x7" line
    static constexpr uint64_t StartBlackPawnMask = GenMask(48, 56, 1);

};


#endif //PAWNMAP_H
