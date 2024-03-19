//
// Created by Jlisowskyy on 3/18/24.
//

#ifndef KILLERTABLE_H
#define KILLERTABLE_H

#include "../EngineTypeDefs.h"
#include "../MoveGeneration/Move.h"


/*
 *      Class used to implement so called killed heuristic, which depends on observation
 *      that in same ply depth some moves remains good no matter what was previously player.
 *      KillerTable is used to save all moves which caused a beta cutoff during search in siblings nodes.
 */

class KillerTable
{
    static constexpr size_t MovesPerPly = 4;
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

    // checks wheter actual move is a "killer" move
    [[nodiscard]] bool IsKillerMove(const Move move, const int depthLeft) const
    {
        return _kTable[depthLeft].Contains(move);
    }

private:
    // ------------------------------
    // Inner types
    // ------------------------------

    struct _killerFloor_t
    {
        _killerFloor_t() = default;

        void Push(const Move mv)
        {
            // all possible slots are used
            if (last == MovesPerPly) return;

            // ensuring that no same moves are stored twice
            for(size_t i = 0; i < last; ++i)
                if (_killerMovesTable[i] == mv.GetPackedMove()) return;

            // saving move
            _killerMovesTable[last++] = mv.GetPackedMove();
        }

        [[nodiscard]] bool Contains(const Move mv) const
        {
            for (size_t i = 0; i < MovesPerPly; ++i)
                if (_killerMovesTable[i] == mv.GetPackedMove())
                    return true;
            return false;
        }

        PackedMove _killerMovesTable[MovesPerPly]{};
        uint8_t last{};
    };

    // ------------------------------
    // Class fields
    // ------------------------------

    _killerFloor_t _kTable[MaxSearchDepth]{};
};

#endif //KILLERTABLE_H
