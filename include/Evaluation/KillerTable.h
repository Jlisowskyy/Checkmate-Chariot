//
// Created by Jlisowskyy on 3/18/24.
//

#ifndef KILLERTABLE_H
#define KILLERTABLE_H

#include "../EngineUtils.h"
#include "../MoveGeneration/Move.h"
#include <cassert>

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

    // Ensure fast modulo operation
    static_assert(MovesPerPly % 2 == 0);

    public:
    // Used to provide smooth way to save garbage moves on root
    static constexpr size_t SentinelWriteFloor = MAX_SEARCH_DEPTH;

    // Used to provide clean null move floor for root node
    static constexpr size_t SentinelReadFloor = SentinelWriteFloor + 1;

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
    INLINE void ClearPlyFloor(const int ply) { _kTable[ply] = {}; }

    // saves move to the table if possible
    INLINE void SaveKillerMove(const Move kMove, const int ply)
    {
        // Note: write to sentinel on root
        const int floor = ply - 1 >= 0 ? ply - 1 : static_cast<int>(SentinelWriteFloor);

        _kTable[floor].Push(kMove);
    }

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
            // ensuring that no same moves are stored twice
            for (const auto storedMove : _killerMovesTable)
                if (storedMove == mv.GetPackedMove())
                    return;

            // saving move
            _killerMovesTable[last++] = mv.GetPackedMove();
            last %= MovesPerPly;
        }

        // simply iterates through _killerMovesTable and compares given move to all inside
        [[nodiscard]] INLINE bool Contains(const Move mv) const
        {
            bool rv {};

            for (const auto killerMove : _killerMovesTable)
                rv |= (killerMove == mv.GetPackedMove());

            return rv;
        }

        PackedMove _killerMovesTable[MovesPerPly]{};
        size_t last{};
    };

    // ------------------------------
    // Class fields
    // ------------------------------

    _killerFloor_t _kTable[MAX_SEARCH_DEPTH + 2]{};
};

// table used mainly to avoid passing new objects every time
inline KillerTable DummyKillerTable{};

#endif // KILLERTABLE_H
