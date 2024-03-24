//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include "../BitOperations.h"
#include "../Board.h"

#include "BishopMap.h"
#include "RookMap.h"

#include <cassert>

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

    enum class PinnedFigGen
    {
        WAllowedTiles,
        WoutAllowedTiles
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

    [[nodiscard]] uint64_t GenerateAllowedTilesForPrecisedPinnedFig(uint64_t figBoard, uint64_t fullMap) const;

    // returns [ pinnedFigMap, allowedTilesMap ]
    template<PinnedFigGen genType>
    [[nodiscard]] std::pair<uint64_t, uint64_t> GetPinnedFigsMap(int col, uint64_t fullMap) const;

    [[nodiscard]] uint64_t GetAllowedTilesWhenCheckedByNonSliding() const;

    // ------------------------------
    // private methods
    // ------------------------------


    static std::pair<uint64_t, uint8_t> _getRookBlockedMap(uint64_t rookMap, uint64_t fullMapWoutKing, uint64_t kingMap);

    template <class MoveGeneratorT>
    [[nodiscard]] static uint64_t _blockIterativeGenerator(uint64_t board, MoveGeneratorT mGen);

    // returns [ pinnedFigMap, allowedTilesMap ]
    template <class MoveMapT, PinnedFigGen type>
    [[nodiscard]] std::pair<uint64_t, uint64_t> _getPinnedFigMaps(uint64_t fullMap, uint64_t possiblePinningFigs) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    Board& board;
};

template<ChessMechanics::PinnedFigGen genType>
std::pair<uint64_t, uint64_t> ChessMechanics::GetPinnedFigsMap(const int col, const uint64_t fullMap) const
{
    assert(fullMap != 0);
    assert(col == 1 || col == 0);

    const size_t enemyCord = SwapColor(col) * Board::BoardsPerCol;

    const auto [pinnedByRooks, allowedRooks] =
            _getPinnedFigMaps<RookMap, genType>(fullMap,
                                                board.boards[enemyCord + rooksIndex] | board.boards[enemyCord + queensIndex]);

    const auto [pinnedByBishops, allowedBishops] =
            _getPinnedFigMaps<BishopMap, genType>(fullMap,
                                                  board.boards[enemyCord + bishopsIndex] | board.boards[enemyCord + queensIndex]);

    return {pinnedByBishops | pinnedByRooks, allowedBishops | allowedRooks};
}

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

template<class MoveMapT, ChessMechanics::PinnedFigGen type>
std::pair<uint64_t, uint64_t> ChessMechanics::_getPinnedFigMaps(const uint64_t fullMap, const uint64_t possiblePinningFigs) const
{
    uint64_t allowedTilesFigMap{};
    [[maybe_unused]] uint64_t pinnedFigMap{};

    const int kingPos = board.GetKingMsbPos(board.movColor);
    // generating figs seen from king's rook perpective
    const uint64_t kingFigPerspectiveAttackedFields = MoveMapT::GetMoves(kingPos, fullMap);
    const uint64_t kingFigPerspectiveAttackedFigs = kingFigPerspectiveAttackedFields & fullMap;

    // this functions should be called only in case of single check so the value below can only be either null or the map of checking figure
    if constexpr (type == PinnedFigGen::WAllowedTiles)
        if (const uint64_t kingSeenEnemyFigs = kingFigPerspectiveAttackedFigs & possiblePinningFigs; kingSeenEnemyFigs != 0)
        {
            const int msbPos = ExtractMsbPos(kingSeenEnemyFigs);
            const uint64_t moves = MoveMapT::GetMoves(msbPos, fullMap);

            allowedTilesFigMap = (moves & kingFigPerspectiveAttackedFields) | kingSeenEnemyFigs;
        }

    // removing figs seen by king
    const uint64_t cleanedMap = fullMap ^ kingFigPerspectiveAttackedFigs;

    // generating figs, which stayed behid first ones and are actually pinnig ones
    const uint64_t kingSecondRookPerspective = MoveMapT::GetMoves(kingPos, cleanedMap);
    uint64_t pinningFigs = possiblePinningFigs & kingSecondRookPerspective;

    // generating fields which are both seen by king and pinning figure = field on which pinned figure stays
    while(pinningFigs != 0)
    {
        const int msbPos = ExtractMsbPos(pinningFigs);
        pinnedFigMap |= MoveMapT::GetMoves(msbPos, fullMap) & kingFigPerspectiveAttackedFigs;
        pinningFigs ^= maxMsbPossible >> msbPos;
    }

    return {pinnedFigMap, allowedTilesFigMap};
}


#endif  // CHESSMECHANICS_H
