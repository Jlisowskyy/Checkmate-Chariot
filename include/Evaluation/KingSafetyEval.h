//
// Created by Jlisowskyy on 3/26/24.
//

#ifndef KINGSAFETYFIELDS_H
#define KINGSAFETYFIELDS_H

#include "../MoveGeneration/MoveGenerationUtils.h"

struct KingSafetyEval
{
    struct _kingSafetyInfo_t
    {
        int32_t attackCounts;
        int32_t attackPoints;
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    KingSafetyEval()  = delete;
    ~KingSafetyEval() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    static uint64_t GetSafetyFields(const Board &bd, int col);

    static void __attribute__((always_inline))
    UpdateKingAttacks(_kingSafetyInfo_t &info, const uint64_t attacks, const uint64_t ring, const int32_t points)
    {
        const int kintAttackingCount = CountOnesInBoard(attacks & ring);

        info.attackCounts += kintAttackingCount > 0;
        info.attackPoints += kintAttackingCount * points;
    }

    static uint64_t GetFrontlineMask(const int col, const int msb) { return _kingPawnDefenseFields[col][msb]; }

    static int32_t EvalKingShelter(const Board &bd);

    static int32_t EvalKingOpenFiles(const Board &bd);

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    static constexpr std::array<std::array<uint64_t, Board::BitBoardFields>, 2> _kingPawnDefenseFields = []() constexpr
    {
        std::array<std::array<uint64_t, Board::BitBoardFields>, 2> rv{};

        for (int col = WHITE; col <= BLACK; ++col)
        {
            const int offset = col == WHITE ? 8 : -8;

            for (int msb = 0; msb < static_cast<int>(Board::BitBoardFields); ++msb)
            {
                const int boardIndex = ConvertToReversedPos(msb);
                const int startInd   = boardIndex + offset;
                const int xOff       = boardIndex % 8;

                if (startInd < 0 || startInd >= static_cast<int>(Board::BitBoardFields))
                    continue;

                uint64_t leftMask{};
                uint64_t rightMask{};

                if (xOff - 1 >= 0)
                    leftMask =
                        col == WHITE
                            ? GenMask(startInd - 1, std::min(static_cast<int>(Board::BitBoardFields), startInd + 8), 8)
                            : GenMask(std::max(xOff - 1, startInd - 8 - 1), startInd, 8);

                if (xOff + 1 < 8)
                    rightMask = col == WHITE
                                    ? GenMask(
                                          startInd + 1,
                                          std::min(static_cast<int>(Board::BitBoardFields), startInd + 8 + 1 + 1), 8
                                      )
                                    : GenMask(std::max(xOff + 1, startInd + 1 - 8), startInd + 2, 8);

                const uint64_t midMask =
                    col == WHITE
                        ? GenMask(startInd, std::min(static_cast<int>(Board::BitBoardFields), startInd + 8 + 1), 8)
                        : GenMask(std::max(xOff, startInd - 8), startInd + 1, 8);

                rv[col][msb] = leftMask | rightMask | midMask;
            }
        }

        return rv;
    }
    ();

    public:
    static constexpr int32_t KingNoShelterPenalty = -50;
    static constexpr int32_t KingOpenFilePenalty  = -30;
};

#endif // KINGSAFETYFIELDS_H
