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
    static constexpr size_t MovesPerPly = 2;

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    constexpr KillerTable()  = default;
    constexpr ~KillerTable() = default;

    KillerTable(KillerTable &&)      = delete;
    KillerTable(const KillerTable &) = delete;

    KillerTable &operator=(const KillerTable &) = delete;
    KillerTable &operator=(KillerTable &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // simply clears previously saved moves
    INLINE void ClearPlyFloor(int ply) { _kTable[ply] = {}; }

    // saves move to the table if possible
    INLINE void SaveKillerMove(const Move kMove, const int ply) { _kTable[ply].Push(kMove); }

    // checks whether actual move is a "killer" move
    [[nodiscard]] INLINE bool IsKillerMove(const Move move, const int ply) const { return _kTable[ply].Contains(move); }

    private:
    // ------------------------------
    // Inner types
    // ------------------------------

    // structure used to save killer moves with MovesPerPly limit
    struct _killerFloor_t
    {
        _killerFloor_t() = default;

        // Saves up to MovePerPly moves. Only first ones, no replacement policy, no duplicates saved.
        INLINE void Push(const Move mv)
        {
            // all possible slots are used
            if (last == MovesPerPly)
                return;

            // ensuring that no same moves are stored twice
            for (size_t i = 0; i < last; ++i)
                if (_killerMovesTable[i] == mv.GetPackedMove())
                    return;

            // saving move
            _killerMovesTable[last++] = mv.GetPackedMove();
        }

        // simply iterates through _killerMovesTable and compares given move to all inside
        [[nodiscard]] INLINE bool Contains(const Move mv) const
        {
            for (auto killerMove : _killerMovesTable)
                if (killerMove == mv.GetPackedMove())
                    return true;
            return false;
        }

        PackedMove _killerMovesTable[MovesPerPly]{};
        uint8_t last{};
    };

    // ------------------------------
    // Class fields
    // ------------------------------

    _killerFloor_t _kTable[MAX_SEARCH_DEPTH + 1]{};
};

// table used mainly to avoid passing new objects every time
inline KillerTable DummyKillerTable{};

#endif // KILLERTABLE_H
