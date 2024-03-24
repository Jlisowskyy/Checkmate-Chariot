//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include "../BitOperations.h"
#include "../Board.h"


#include <cassert>

/*              General optimizations TODO:
 *     - test all msb and lsb extractions
 *     - add and test AVX support
 *     - test other ideas for hashing
 *
 */

/*  There is general bug in the logic
 *  double check can also occur by two same type figures
 *  in result of pawn upgrade
 *  what is not predicted in procedure
 *  that generates check count
 */

struct ChessMechanics
{
    // ------------------------------
    // Class inner types
    // ------------------------------

    enum checkType
    {
        slidingFigCheck,
        simpleFigCheck
    };

    // ------------------------------
    // Class Creation
    // ------------------------------

    ChessMechanics() = delete;

    explicit ChessMechanics(Board& bd) : board(bd) {}

    ChessMechanics(ChessMechanics& other) = default;

    ChessMechanics& operator=(ChessMechanics&) = delete;

    ~ChessMechanics() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] bool IsCheck() const;

    [[nodiscard]] uint64_t GetFullMap() const;

    [[nodiscard]] uint64_t GetColMap(int col) const;

    // does not check kings boards!!!
    [[nodiscard]] size_t GetIndexOfContainingBoard(uint64_t map, int col) const;

    // [blockedFigMap, checksCount, checkType]
    [[nodiscard]] std::tuple<uint64_t, uint8_t, uint8_t> GetBlockedFieldMap(uint64_t fullMap) const;

    // should only be used when no check is on board
    [[nodiscard]] uint64_t GetPinnedFigsMapWoutCheck(int col, uint64_t fullMap) const;

    [[nodiscard]] uint64_t GenerateAllowedTilesForPrecisedPinnedFig(uint64_t figBoard, uint64_t fullMap) const;

    // returns [ pinnedFigMap, allowedTilesMap ]
    [[nodiscard]] std::pair<uint64_t, uint64_t> GetPinnedFigsMapWithCheck(int col, uint64_t fullMap) const;

    [[nodiscard]] uint64_t GetAllowedTilesWhenCheckedByNonSliding() const;

    /*                  Important notes:
     *
     *  ActionT must follow this scheme:
     *  void Action(Board& board)
     *  Where:
     *      - board - represents actual state of the board, in current iteration,
     *
     *  Iteration follow DFS scheme, and action is only invoked on leafs.
     *
     */

    // ------------------------------
    // private methods
    // ------------------------------


    static std::pair<uint64_t, uint8_t> _getRookBlockedMap(uint64_t rookMap, uint64_t fullMapWoutKing, uint64_t kingMap);

    template <class MoveGeneratorT>
    [[nodiscard]] static uint64_t _blockIterativeGenerator(uint64_t board, MoveGeneratorT mGen);

    template <class MoveMapT>
    [[nodiscard]] uint64_t _getPinnedFigsWoutCheckGenerator(uint64_t fullMap, uint64_t possiblePinningFigs) const;

    // returns [ pinnedFigMap, allowedTilesMap ]
    template <class MoveMapT>
    [[nodiscard]] std::pair<uint64_t, uint64_t> _getPinnedFigsWithCheckGenerator(uint64_t suspectedFigs,
                                                                                 uint64_t fullMap,
                                                                                 size_t allyCord,
                                                                                 int allyKingShift) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    Board& board;
};

template<class MoveGeneratorT>
uint64_t ChessMechanics::_blockIterativeGenerator(uint64_t board, MoveGeneratorT mGen)
{
    uint64_t blockedMap = 0;

    while (board != 0)
    {
        const int figPos = ExtractMsbPos(board);
        board ^= (maxMsbPossible >> figPos);

        blockedMap |= mGen(figPos);
    }

    return blockedMap;
}

template<class MoveMapT>
uint64_t ChessMechanics::_getPinnedFigsWoutCheckGenerator(const uint64_t fullMap, const uint64_t possiblePinningFigs) const
{
    assert(fullMap != 0);

    uint64_t pinnedFigMap{};
    const int kingPos = board.GetKingMsbPos(board.movColor);
    const uint64_t kingFigPerspectiveAttackedFigs = MoveMapT::GetMoves(kingPos, fullMap) & fullMap;
    const uint64_t cleanedMap = fullMap ^ kingFigPerspectiveAttackedFigs;
    const uint64_t kingSecondRookPerspective = MoveMapT::GetMoves(kingPos, cleanedMap);
    uint64_t pinningFigs = possiblePinningFigs & kingSecondRookPerspective;

    while(pinningFigs != 0)
    {
        const int msbPos = ExtractMsbPos(pinningFigs);
        pinnedFigMap |= MoveMapT::GetMoves(msbPos, fullMap) & kingFigPerspectiveAttackedFigs;
        pinningFigs ^= maxMsbPossible >> msbPos;
    }

    return pinnedFigMap;
}

template<class MoveMapT>
std::pair<uint64_t, uint64_t> ChessMechanics::_getPinnedFigsWithCheckGenerator(uint64_t suspectedFigs,
    const uint64_t fullMap, const size_t allyCord, const int allyKingShift) const
{
    uint64_t pinnedFigMap{};
    uint64_t allowedTilesFigMap{};

    while (suspectedFigs != 0)
    {
        const int msbPos = ExtractMsbPos(suspectedFigs);
        const uint64_t suspectedFigAttackFields = MoveMapT::GetMoves(msbPos, fullMap);
        const uint64_t kingsPerspective = MoveMapT::GetMoves(ConvertToReversedPos(allyKingShift), fullMap);

        // TODO: ERROR HERE
        if (const uint64_t attackedFig =
                    suspectedFigAttackFields & fullMap & (kingsPerspective | board.boards[allyCord + kingIndex]);
            attackedFig == board.boards[allyCord + kingIndex])
        {
            allowedTilesFigMap |= (suspectedFigAttackFields | (maxMsbPossible >> msbPos)) & kingsPerspective;
        }
        else
        {
            const uint64_t mapWoutAttackedFig = fullMap ^ attackedFig;
            const uint64_t isKingAttacked = MoveMapT::GetMoves(msbPos, mapWoutAttackedFig) & mapWoutAttackedFig &
                                            board.boards[allyCord + kingIndex];
            const uint64_t pinnedFigFlag = (isKingAttacked >> allyKingShift) * attackedFig;

            pinnedFigMap |= pinnedFigFlag;
        }

        suspectedFigs ^= (maxMsbPossible >> msbPos);
    }

    return {pinnedFigMap, allowedTilesFigMap};
}

#endif  // CHESSMECHANICS_H
