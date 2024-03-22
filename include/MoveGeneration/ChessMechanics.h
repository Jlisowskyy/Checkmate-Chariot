//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include <array>

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "BishopMap.h"
#include "BlackPawnMap.h"
#include "KingMap.h"
#include "KnightMap.h"
#include "QueenMap.h"
#include "RookMap.h"
#include "WhitePawnMap.h"

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
    [[nodiscard]] static uint64_t _blockIterativeGenerator(uint64_t board, MoveGeneratorT mGen)
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

    template <class MoveMapT>
    [[nodiscard]] uint64_t _getPinnedFigsWoutCheckGenerator(uint64_t suspectedFigs, const uint64_t fullMap,
                                                            const size_t allyCord, const int allyKingShift) const
    {
        uint64_t pinnedFigMap{};

        while (suspectedFigs != 0)
        {
            const int msbPos = ExtractMsbPos(suspectedFigs);
            const uint64_t attackedFig = MoveMapT::GetMoves(msbPos, fullMap) & fullMap &
                                         MoveMapT::GetMoves(ConvertToReversedPos(allyKingShift), fullMap);

            const uint64_t mapWoutAttackedFig = fullMap ^ attackedFig;
            const uint64_t isKingAttacked = MoveMapT::GetMoves(msbPos, mapWoutAttackedFig) & mapWoutAttackedFig &
                                            board.boards[allyCord + kingIndex];

            const uint64_t pinnedFigFlag = (isKingAttacked >> allyKingShift) * attackedFig;  // 0 or attackedFig
            pinnedFigMap |= pinnedFigFlag;

            suspectedFigs ^= (maxMsbPossible >> msbPos);
        }

        return pinnedFigMap;
    }

    // returns [ pinnedFigMap, allowedTilesMap ]
    template <class MoveMapT>
    [[nodiscard]] std::pair<uint64_t, uint64_t> _getPinnedFigsWithCheckGenerator(uint64_t suspectedFigs,
                                                                                 const uint64_t fullMap,
                                                                                 const size_t allyCord,
                                                                                 const int allyKingShift) const
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

    // ------------------------------
    // Class fields
    // ------------------------------

    Board& board;
};

#endif  // CHESSMECHANICS_H
