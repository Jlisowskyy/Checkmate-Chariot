//
// Created by Jlisowskyy on 3/26/24.
//

#ifndef KINGSAFETYFIELDS_H
#define KINGSAFETYFIELDS_H

#include "../MoveGeneration/MoveGenerationUtils.h"
#include "../MoveGeneration/KingMap.h"
#include "../MoveGeneration/FileMap.h"

/*
 * Structure below gathers all the necessary information about the king safety evaluation.
 * In short king's safety evaluation depends on:
 *  - the weighted number of fields that are attacked by the enemy inside the 'king ring'
 *  - there is penalty for open files around the king
 *  - king's shelter evaluation, which is based on the number of pawns that are in front of the king
 *
 *  To precisely use the above ones, they should be paired with tapered eval!
 */

struct KingSafetyEval
{
    // Structure below is used to return king's safety information.
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

    // This method is used to return fields that controlling by enemy imposes danger to the king.
    // TODO: Currently its simple ring around the king, reconsider and improve.
    [[nodiscard]] static uint64_t GetSafetyFields(const Board &bd, const int col) __attribute__((always_inline))
    {
        const uint64_t kingMap  = bd.BitBoards[KingMap::GetBoardIndex(col)];
        const uint64_t kingRing = KingMap::GetMoves(ExtractMsbPos(kingMap)) | kingMap;
        return kingRing;
    }

    static void UpdateKingAttacks(_kingSafetyInfo_t &info, const uint64_t attacks, const uint64_t kingRing, const int32_t pointsPerAttack) __attribute__((always_inline))
    {
        const int32_t kingAttackingCount = CountOnesInBoard(attacks & kingRing);

        info.attackCounts += kingAttackingCount > 0;
        info.attackPoints += kingAttackingCount * pointsPerAttack;
    }

    // Returns mask that defines the shelter in front of the king;

    [[nodiscard]] static uint64_t GetFrontLineMask(const int col, const int msbPos) __attribute__((always_inline))
    { return _kingPawnDefenseFields[col][msbPos]; }

    // Returns summed shelter penalty points for both kings.
    [[nodiscard]] static int32_t EvalKingShelter(const Board &bd)
    {
        int32_t eval{};
        if ((bd.BitBoards[wKingIndex] & KingMap::ShelterLocationMask[WHITE]) != 0 &&
            CountOnesInBoard(bd.BitBoards[wPawnsIndex] & GetFrontLineMask(WHITE, ExtractMsbPos(bd.BitBoards[wKingIndex]))) <
                3)
            eval += KingNoShelterPenalty;

        if ((bd.BitBoards[bKingIndex] & KingMap::ShelterLocationMask[BLACK]) != 0 &&
            CountOnesInBoard(bd.BitBoards[bPawnsIndex] & GetFrontLineMask(BLACK, ExtractMsbPos(bd.BitBoards[bKingIndex]))) <
                3)
            eval += -KingNoShelterPenalty;

        return eval;
    }

    // Returns summed open files penalty points for both kings.
    [[nodiscard]] static int32_t EvalKingOpenFiles(const Board &bd)
    {
        int32_t eval{};

        const auto wSep = FileMap::GetSepFiles(ExtractMsbPos(bd.BitBoards[wKingIndex]));
        for (size_t i = 0; i < FileMap::FileSepSize; ++i)
            eval += ((bd.BitBoards[wPawnsIndex] & wSep[i]) == 0) * KingOpenFilePenalty;

        const auto bSep = FileMap::GetSepFiles(ExtractMsbPos(bd.BitBoards[bKingIndex]));
        for (size_t i = 0; i < FileMap::FileSepSize; ++i)
            eval -= ((bd.BitBoards[bPawnsIndex] & bSep[i]) == 0) * KingOpenFilePenalty;

        return eval;
    }

    // Returns score for the king ring control.
    [[nodiscard]] static int32_t ScoreKingRingControl(const _kingSafetyInfo_t& whiteInfo, const _kingSafetyInfo_t& blackInfo) __attribute__((always_inline))
    {
        int32_t bonus {};

        bonus += (whiteInfo.attackCounts > 2) * (-_kingSafetyValues[whiteInfo.attackPoints]);
        bonus += (blackInfo.attackCounts > 2) * (_kingSafetyValues[blackInfo.attackPoints]);

        return bonus;
    }
    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    static constexpr std::array<std::array<uint64_t, Board::BitBoardFields>, 2> _kingPawnDefenseFields = []() constexpr
    {
        std::array<std::array<uint64_t, Board::BitBoardFields>, 2> rv{};

        for (int col = WHITE; col <= BLACK; ++col)
        {
            const int32_t offset = col == WHITE ? 8 : -8;

            for (int32_t msb = 0; msb < static_cast<int32_t>(Board::BitBoardFields); ++msb)
            {
                const int32_t boardIndex = ConvertToReversedPos(msb);
                const int32_t startInd   = boardIndex + offset;
                const int32_t xOff       = boardIndex % 8;

                if (startInd < 0 || startInd >= static_cast<int32_t>(Board::BitBoardFields))
                    continue;

                uint64_t leftMask{};
                uint64_t rightMask{};

                if (xOff - 1 >= 0)
                    leftMask =
                        col == WHITE
                            ? GenMask(startInd - 1, std::min(static_cast<int32_t>(Board::BitBoardFields), startInd + 8), 8)
                            : GenMask(std::max(xOff - 1, startInd - 8 - 1), startInd, 8);

                if (xOff + 1 < 8)
                    rightMask = col == WHITE
                                    ? GenMask(
                                          startInd + 1,
                                          std::min(static_cast<int32_t>(Board::BitBoardFields), startInd + 8 + 1 + 1), 8
                                      )
                                    : GenMask(std::max(xOff + 1, startInd + 1 - 8), startInd + 2, 8);

                const uint64_t midMask =
                    col == WHITE
                        ? GenMask(startInd, std::min(static_cast<int32_t>(Board::BitBoardFields), startInd + 8 + 1), 8)
                        : GenMask(std::max(xOff, startInd - 8), startInd + 1, 8);

                rv[col][msb] = leftMask | rightMask | midMask;
            }
        }

        return rv;
    }();

    static constexpr int16_t _kingSafetyValues[] = {
        0,   0,   1,   2,   3,   5,   7,   9,   12,  15,  18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
        68,  75,  82,  85,  89,  97,  105, 113, 122, 131, 140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
        260, 272, 283, 295, 307, 319, 330, 342, 354, 366, 377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
        494, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
        500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500
    };

    public:

    // values below are used to construct a key to the king safety lookup table (_kingSafetyValues),
    // which store exponentially scaled king safety values.
    // Key is simply constructed by summing number of attacks * (corresponding coef)
    static constexpr int16_t KingMinorPieceAttackPoints = 2;
    static constexpr int16_t KingRookAttackPoints       = 3;
    static constexpr int16_t KingQueenAttackPoints      = 5;

    // Parameters used to score king's safety.
    static constexpr int32_t KingNoShelterPenalty = -50;
    static constexpr int32_t KingOpenFilePenalty  = -30;
};

#endif // KINGSAFETYFIELDS_H
