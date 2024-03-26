//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef PAWNMAP_H
#define PAWNMAP_H

#include <cstdint>

#include "MoveGeneration.h"

class WhitePawnMap
{
    // ------------------------------
    // Class creation
    // ------------------------------
   public:
    WhitePawnMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr size_t GetBoardIndex([[maybe_unused]] int color);

    [[nodiscard]] static constexpr int GetColor() { return WHITE; }

    [[nodiscard]] static constexpr size_t GetEnemyPawnBoardIndex();

    [[nodiscard]] static constexpr uint64_t GetAttackFields(uint64_t pawnBits);

    [[nodiscard]] static constexpr uint64_t GetPlainMoves(uint64_t pawnBit, uint64_t fullMap);


    // Returns all moves excepts ElPassantOnes
    [[nodiscard]] static constexpr uint64_t GetMoves(int msbPos, uint64_t fullMap, uint64_t enemyMap);

    [[nodiscard]] static constexpr uint64_t GetElPassantSuspectedFields(uint64_t elPassantField);

    [[nodiscard]] static constexpr uint64_t GetElPassantMoveField(uint64_t elPassantField);

    [[nodiscard]] static uint64_t GetElPassantField(uint64_t moveField, uint64_t startField);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint64_t PromotingMask = GenMask(48, 56, 1);
    // Mask indicating whether enemy pawn can kill white pawn on that field
    static constexpr uint64_t ElPassantMask = GenMask(24, 32, 1);
    // Mask indicating whether enemy pawn can kill black pawn on that field
    static constexpr uint64_t EnemyElPassantMask = GenMask(32, 40, 1);
    // Mask with ones only on "x2" line
    static constexpr uint64_t StartMask = GenMask(8, 16, 1);

   private:
    // Mask with ones only on "Ax" line
    static constexpr uint64_t LeftMask = ~GenMask(0, 57, 8);

    // Mask with ones only on "Hx" line
    static constexpr uint64_t RightMask = ~GenMask(7, 64, 8);
};

inline uint64_t WhitePawnMap::GetElPassantField(const uint64_t moveField, const uint64_t startField)
{
    return moveField & ElPassantMask & (StartMask & startField) << 16;
}

constexpr size_t WhitePawnMap::GetBoardIndex([[maybe_unused]]const int color)
{ return wPawnsIndex; }

constexpr size_t WhitePawnMap::GetEnemyPawnBoardIndex()
{ return bPawnsIndex; }

constexpr uint64_t WhitePawnMap::GetAttackFields(const uint64_t pawnBits)
{
    const uint64_t leftAttack = (LeftMask & pawnBits) << 7;
    const uint64_t rightAttack = (RightMask & pawnBits) << 9;
    return leftAttack | rightAttack;
}

constexpr uint64_t WhitePawnMap::GetPlainMoves(const uint64_t pawnBit, const uint64_t fullMap)
{
    const uint64_t frontMove = (pawnBit << 8) & ~fullMap;

    const uint64_t isOnStartField = ((frontMove >> 8) & pawnBit & StartMask) << 16;
    const uint64_t frontDoubleMove = isOnStartField & ~fullMap;

    return frontMove | frontDoubleMove;
}

constexpr uint64_t WhitePawnMap::GetMoves(const int msbPos, const uint64_t fullMap, const uint64_t enemyMap)
{
    const uint64_t pawnBit = maxMsbPossible >> msbPos;
    const uint64_t attackMoves = GetAttackFields(pawnBit) & enemyMap;
    const uint64_t plainMoves = GetPlainMoves(pawnBit, fullMap);

    return attackMoves | plainMoves;
}

constexpr uint64_t WhitePawnMap::GetElPassantSuspectedFields(const uint64_t elPassantField)
{
    const uint64_t leftField = (LeftMask & elPassantField) >> 1;
    const uint64_t rightField = (RightMask & elPassantField) << 1;
    return leftField | rightField;
}

constexpr uint64_t WhitePawnMap::GetElPassantMoveField(const uint64_t elPassantField)
{
    return elPassantField << 8;
}

#endif  // PAWNMAP_H
