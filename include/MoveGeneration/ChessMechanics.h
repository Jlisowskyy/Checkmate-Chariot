//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include "../BitOperations.h"
#include "../Board.h"

#include "BishopMap.h"
#include "Move.h"
#include "RookMap.h"

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

    explicit ChessMechanics(const Board &bd) : _board(bd) {}

    ChessMechanics(ChessMechanics &other)       = delete;
    ChessMechanics &operator=(ChessMechanics &) = delete;

    ~ChessMechanics() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] bool IsCheck() const;

    [[nodiscard]] INLINE bool IsDrawByReps(const uint64_t hash)
    {
        return _board.Repetitions.at(hash) >= 3 || _board.HalfMoves >= 50;
    }

    // Gets occupancy maps, which simply indicates whether some field is occupied or not. Does not distinguish colors.
    [[nodiscard]] INLINE uint64_t GetFullBitMap() const
    {
        uint64_t map = 0;
        for (const auto m : _board.BitBoards) map |= m;
        return map;
    }

    // Gets occupancy maps, which simply indicates whether some field is occupied or not, by desired color figures.
    [[nodiscard]] INLINE uint64_t GetColBitMap(const int col) const
    {
        TraceIfFalse(col == 1 || col == 0, "Invalid color!");

        uint64_t map = 0;
        for (size_t i = 0; i < Board::BitBoardsPerCol; ++i) map |= _board.BitBoards[Board::BitBoardsPerCol * col + i];
        return map;
    }

    // does not check kings BitBoards!!!
    [[nodiscard]] INLINE size_t GetIndexOfContainingBitBoard(const uint64_t map, const int col) const
    {
        const size_t colIndex = col * Board::BitBoardsPerCol;
        size_t rv             = 0;
        for (size_t i = 0; i < Board::BitBoardsPerCol; ++i)
        {
            rv += ((_board.BitBoards[colIndex + i] & map) != 0) * i;
        }
        return colIndex + rv;
    }

    // [blockedFigMap, checksCount, checkType]
    [[nodiscard]] std::tuple<uint64_t, uint8_t, uint8_t> GetBlockedFieldBitMap(uint64_t fullMap) const;

    [[nodiscard]] uint64_t GenerateAllowedTilesForPrecisedPinnedFig(uint64_t figBoard, uint64_t fullMap) const;

    // returns [ pinnedFigMap, allowedTilesMap ]
    template <PinnedFigGen genType>
    [[nodiscard]] std::pair<uint64_t, uint64_t> GetPinnedFigsMap(int col, uint64_t fullMap) const;

    [[nodiscard]] uint64_t GetAllowedTilesWhenCheckedByNonSliding() const;

    /* Simply picks the least valuable figure from 'pieces' set with given 'color'.
     * Returns bitboard containing position of that figure and index of that figure to 'pieceIndOut'
     * */
    [[nodiscard]] INLINE uint64_t GetLeastValuablePiece(uint64_t pieces, int color, int &pieceIndOut) const
    {
        const int start = color * static_cast<int>(Board::BitBoardsPerCol);
        const int range = start + static_cast<int>(kingIndex);
        for (int ind = start; ind < range; ++ind)
        {
            const uint64_t intersection = pieces & _board.BitBoards[ind];

            if (intersection)
            {
                pieceIndOut = ind;
                return ExtractLsbBit(intersection);
            }
        }

        return 0;
    }

    /* Simply picks the least valuable figure from 'pieces' set with given 'color'.
     * Checks whether figure can legally attack given field.
     * Returns bitboard containing position of that figure and index of that figure to 'pieceIndOut'.
     *
     * This function assumes that on given field by 'msbPos' for sure there is no king placed.
     * */
    [[nodiscard]] uint64_t getLeastValuablePieceFromLegalToSquare(uint64_t allPieces, uint64_t pieces, int color, int &pieceIndOut) const;

    /*
     * SEE - Static Exchange Evaluation - function used to get approximated gain
     * after making given move, that is: it performs every exchange on field given by the move
     * */
    [[nodiscard]] int SEE(Move mv) const;

    /* Function finds index of figure type based on given single bit BitBoard */
    static INLINE int FindFigType(const uint64_t BitBoard, const Board &bd)
    {
        int rv              = 0;
        constexpr int range = static_cast<int>(Board::BitBoardsPerCol);
        for (int i = 0; i < range; ++i)
        {
            rv += ((bd.BitBoards[i] & BitBoard) != 0) * i;
            rv += ((bd.BitBoards[wPawnsIndex + i] & BitBoard) != 0) * i;
        }
        return rv;
    }

    // ------------------------------
    // private methods
    // ------------------------------

    private:
    [[nodiscard]] INLINE uint64_t _updateAttackers(const uint64_t fullMap, const int msbPos) const
    {
        const uint64_t bishops = (_board.BitBoards[wQueensIndex] | _board.BitBoards[bQueensIndex] |
                                  _board.BitBoards[wBishopsIndex] | _board.BitBoards[bBishopsIndex]) &
                                 fullMap;
        const uint64_t rooks = (_board.BitBoards[wQueensIndex] | _board.BitBoards[bQueensIndex] |
                                _board.BitBoards[wRooksIndex] | _board.BitBoards[bRooksIndex]) &
                               fullMap;

        uint64_t attackers = 0;
        attackers |= (BishopMap::GetMoves(msbPos, fullMap) & bishops) | (RookMap::GetMoves(msbPos, fullMap) & rooks);

        return attackers;
    }

    struct _seePackage
    {
        uint64_t attackersBitBoard;
        uint64_t fullMap;
        uint64_t xrayMap;
    };

    /*  Function collects information used inside the SEE algorithm it contains:
     *  - attackersBitBoard - contains every type of figure that in current state of the board could attack given field
     *  - fullMap - contains every figure on the board
     *  - xrayMap - contains every figure that attack could be potentially unlocked after other figures move,
     *              that is: queens, bishops, rooks and pawns
     * */
    [[nodiscard]] inline INLINE _seePackage _prepareForSEE(int msbPos) const;

    static std::pair<uint64_t, uint8_t>
    _getRookBlockedMap(uint64_t rookMap, uint64_t fullMapWoutKing, uint64_t kingMap);

    template <class MoveGeneratorT>
    [[nodiscard]] INLINE static uint64_t _blockIterativeGenerator(uint64_t board, MoveGeneratorT mGen)
    {
        uint64_t blockedMap = 0;

        while (board != 0)
        {
            const int figPos = ExtractMsbPos(board);
            board ^= (MaxMsbPossible >> figPos);

            blockedMap |= mGen(figPos);
        }

        return blockedMap;
    }

    // returns [ pinnedFigMap, allowedTilesMap ]
    template <class MoveMapT, PinnedFigGen type>
    [[nodiscard]] std::pair<uint64_t, uint64_t> _getPinnedFigMaps(uint64_t fullMap, uint64_t possiblePinningFigs) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    const Board &_board;
};

template <ChessMechanics::PinnedFigGen genType>
std::pair<uint64_t, uint64_t> ChessMechanics::GetPinnedFigsMap(const int col, const uint64_t fullMap) const
{
    TraceIfFalse(fullMap != 0, "Full map is empty!");
    TraceIfFalse(col == 1 || col == 0, "Invalid color!");

    const size_t enemyCord = SwapColor(col) * Board::BitBoardsPerCol;

    const auto [pinnedByRooks, allowedRooks] = _getPinnedFigMaps<RookMap, genType>(
        fullMap, _board.BitBoards[enemyCord + rooksIndex] | _board.BitBoards[enemyCord + queensIndex]
    );

    const auto [pinnedByBishops, allowedBishops] = _getPinnedFigMaps<BishopMap, genType>(
        fullMap, _board.BitBoards[enemyCord + bishopsIndex] | _board.BitBoards[enemyCord + queensIndex]
    );

    return {pinnedByBishops | pinnedByRooks, allowedBishops | allowedRooks};
}

template <class MoveMapT, ChessMechanics::PinnedFigGen type>
std::pair<uint64_t, uint64_t>
ChessMechanics::_getPinnedFigMaps(const uint64_t fullMap, const uint64_t possiblePinningFigs) const
{
    uint64_t allowedTilesFigMap{};
    [[maybe_unused]] uint64_t pinnedFigMap{};

    const int kingPos = _board.GetKingMsbPos(_board.MovingColor);
    // generating figs seen from king's rook perpective
    const uint64_t kingFigPerspectiveAttackedFields = MoveMapT::GetMoves(kingPos, fullMap);
    const uint64_t kingFigPerspectiveAttackedFigs   = kingFigPerspectiveAttackedFields & fullMap;

    // this functions should be called only in case of single check so the value below can only be either null or the
    // map of checking figure
    if constexpr (type == PinnedFigGen::WAllowedTiles)
        if (const uint64_t kingSeenEnemyFigs = kingFigPerspectiveAttackedFigs & possiblePinningFigs;
            kingSeenEnemyFigs != 0)
        {
            const int msbPos     = ExtractMsbPos(kingSeenEnemyFigs);
            const uint64_t moves = MoveMapT::GetMoves(msbPos, fullMap);

            allowedTilesFigMap = (moves & kingFigPerspectiveAttackedFields) | kingSeenEnemyFigs;
        }

    // removing figs seen by king
    const uint64_t cleanedMap = fullMap ^ kingFigPerspectiveAttackedFigs;

    // generating figs, which stayed behid first ones and are actually pinnig ones
    const uint64_t kingSecondRookPerspective = MoveMapT::GetMoves(kingPos, cleanedMap);
    uint64_t pinningFigs                     = possiblePinningFigs & kingSecondRookPerspective;

    // generating fields which are both seen by king and pinning figure = field on which pinned figure stays
    while (pinningFigs != 0)
    {
        const int msbPos = ExtractMsbPos(pinningFigs);
        pinnedFigMap |= MoveMapT::GetMoves(msbPos, fullMap) & kingFigPerspectiveAttackedFigs;
        pinningFigs ^= MaxMsbPossible >> msbPos;
    }

    return {pinnedFigMap, allowedTilesFigMap};
}

#endif // CHESSMECHANICS_H
