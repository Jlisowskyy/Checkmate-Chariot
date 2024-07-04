//
// Created by Jlisowskyy on 2/14/24.
//

#ifndef BLACKPAWNMAP_H
#define BLACKPAWNMAP_H

#include <cstdint>

#include "MoveGenerationUtils.h"

class BlackPawnMap
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    BlackPawnMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr size_t GetBoardIndex([[maybe_unused]] const int color) { return bPawnsIndex; }

    [[nodiscard]] static constexpr int GetColor() { return BLACK; }

    [[nodiscard]] static constexpr size_t GetEnemyPawnBoardIndex() { return wPawnsIndex; }

    [[nodiscard]] static constexpr uint64_t GetAttackFields(uint64_t pawnBits);

    [[nodiscard]] static constexpr uint64_t GetPlainMoves(uint64_t pawnBit, uint64_t fullMap);

    [[nodiscard]] static constexpr uint64_t GetSinglePlainMoves(uint64_t pawnBit, uint64_t fullMap);

    [[nodiscard]] static constexpr uint64_t RevertSinglePlainMoves(uint64_t pawnBit);

    // Returns all moves excepts ElPassantOnes
    [[nodiscard]] static constexpr uint64_t GetMoves(int msbPos, uint64_t fullMap, uint64_t enemyMap);

    [[nodiscard]] static constexpr uint64_t GetElPassantSuspectedFields(uint64_t elPassantField);

    [[nodiscard]] static constexpr uint64_t GetElPassantMoveField(uint64_t elPassantField);

    [[nodiscard]] static constexpr uint64_t GetElPassantField(uint64_t moveField, uint64_t startField);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint64_t PromotingMask = GenMask(8, 16, 1);
    // Mask indicating whether black pawn can be killed on that field by enemy pawn
    static constexpr uint64_t ElPassantMask = GenMask(32, 40, 1);
    // Mask indicating whether white pawn can be killed on that field by enemy pawn
    static constexpr uint64_t EnemyElPassantMask = GenMask(24, 32, 1);
    // Mask with ones only on "x7" line
    static constexpr uint64_t StartMask = GenMask(48, 56, 1);

    private:
    // Mask with ones only on 'Ax" line
    static constexpr uint64_t LeftMask = ~GenMask(0, 57, 8);

    // Mask with ones only on "Hx" line
    static constexpr uint64_t RightMask = ~GenMask(7, 64, 8);
};

constexpr uint64_t BlackPawnMap::GetAttackFields(const uint64_t pawnBits)
{
    const uint64_t leftAttack  = (LeftMask & pawnBits) >> 9;
    const uint64_t rightAttack = (RightMask & pawnBits) >> 7;
    return leftAttack | rightAttack;
}

constexpr uint64_t BlackPawnMap::GetPlainMoves(const uint64_t pawnBit, const uint64_t fullMap)
{
    const uint64_t frontMove = (pawnBit >> 8) & ~fullMap;

    const uint64_t isOnStartField  = ((frontMove << 8) & pawnBit & StartMask) >> 16;
    const uint64_t frontDoubleMove = isOnStartField & ~fullMap;

    return frontMove | frontDoubleMove;
}

constexpr uint64_t BlackPawnMap::GetMoves(const int msbPos, const uint64_t fullMap, const uint64_t enemyMap)
{
    const uint64_t pawnBit     = MaxMsbPossible >> msbPos;
    const uint64_t attackMoves = GetAttackFields(pawnBit) & enemyMap;
    const uint64_t plainMoves  = GetPlainMoves(pawnBit, fullMap);

    return attackMoves | plainMoves;
}

constexpr uint64_t BlackPawnMap::GetElPassantSuspectedFields(const uint64_t elPassantField)
{
    const uint64_t leftField = (LeftMask & elPassantField) >> 1;
    const uint64_t righField = (RightMask & elPassantField) << 1;
    return leftField | righField;
}

constexpr uint64_t BlackPawnMap::GetElPassantMoveField(const uint64_t elPassantField) { return elPassantField >> 8; }

constexpr uint64_t BlackPawnMap::GetElPassantField(const uint64_t moveField, const uint64_t startField)
{
    return moveField & ElPassantMask & (StartMask & startField) >> 16;
}

constexpr uint64_t BlackPawnMap::GetSinglePlainMoves(const uint64_t pawnBit, const uint64_t fullMap) {
    return (pawnBit >> 8) & ~fullMap;
}

constexpr uint64_t BlackPawnMap::RevertSinglePlainMoves(uint64_t pawnBit) {
    return pawnBit << 8;
}

#endif // BLACKPAWNMAP_H
