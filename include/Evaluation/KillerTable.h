//
// Created by Jlisowskyy on 3/18/24.
//

#ifndef KILLERTABLE_H
#define KILLERTABLE_H

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"

/*
 *      Class used to implement so-called killed heuristic, which depends on observation
 *      that in the same ply depth some moves remain good no matter what was previously player.
 *      KillerTable is used to save all moves that caused a beta cutoff during search in sibling's nodes.
 *
 *      Resources: https://www.chessprogramming.org/Killer_Heuristic
 */

class KillerTable
{
    // Controls how many moves per ply can be saved
    static constexpr size_t MovesPerPly = 3;
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    constexpr KillerTable() = default;
    constexpr ~KillerTable() = default;

    KillerTable(KillerTable&&) = delete;
    KillerTable(const KillerTable&) = delete;

    KillerTable& operator=(const KillerTable&) = delete;
    KillerTable& operator=(KillerTable&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // simply clears previously saved moves
    void ClearPlyFloor(const int depthLeft)
    {
        _kTable[depthLeft] = {};
    }

    // saves move to the table if possible
    void SaveKillerMove(const Move kMove, const int depthLeft)
    {
        _kTable[depthLeft].Push(kMove);
    }

    // checks whether actual move is a "killer" move
    [[nodiscard]] bool IsKillerMove(const Move move, const int depthLeft) const
    {
        return _kTable[depthLeft].Contains(move);
    }

    private:
    // ------------------------------
    // Inner types
    // ------------------------------

    // structure used to save killer moves with MovesPerPly limit
    struct _killerFloor_t
    {
        _killerFloor_t() = default;

        // Saves up to MovePerPly moves. Only first ones, no replacement policy, no duplicates saved.
        void Push(Move mv);

        // simply iterates through _killerMovesTable and compares given move to all inside
        [[nodiscard]] bool Contains(Move mv) const;

        PackedMove _killerMovesTable[MovesPerPly]{};
        uint8_t last{};
    };

    // ------------------------------
    // Class fields
    // ------------------------------

    _killerFloor_t _kTable[MaxSearchDepth]{};
};

#endif //KILLERTABLE_H

