//
// Created by Jlisowskyy on 3/26/24.
//

#ifndef KINGSAFETYFIELDS_H
#define KINGSAFETYFIELDS_H

#include "../MoveGeneration/MoveGeneration.h"



struct KingSafetyFields
{
    struct _kingSafetyInfo_t
    {
        int32_t attackCounts;
        int32_t attackPoints;
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    KingSafetyFields() = delete;
    ~KingSafetyFields() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    static uint64_t GetSafetyFields(const Board& bd, int col);

    static void  __attribute__((always_inline)) UpdateKingAttacks(_kingSafetyInfo_t& info, const uint64_t attacks, const uint64_t ring, const int32_t points)
    {
        const int kintAttackingCount = CountOnesInBoard(attacks & ring);

        info.attackCounts += kintAttackingCount > 0;
        info.attackPoints += kintAttackingCount * points;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:

    // static constexpr std::array<uint64_t, Board::BoardFields> _safetyFieldsMap = []() constexpr
    // {
    //     std::array<uint64_t, Board::BoardFields> rv{};
    //
    //
    //
    //
    //     return rv;
    // }();
};

#endif //KINGSAFETYFIELDS_H
