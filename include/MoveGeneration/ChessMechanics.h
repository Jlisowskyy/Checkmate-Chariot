//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include <exception>
#include <array>

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "BishopMap.h"
#include "KingMap.h"
#include "KnightMap.h"
#include "PawnMap.h"
#include "RookMap.h"

/*              General optimizations TODO:
 *      - test all msb and lsb extractions
 *      - add and test AVX support
 *      - test other ideas for hashing
 *
 */

struct ChessMechanics {
    // ------------------------------
    // Class inner types
    // ------------------------------

    enum checkType {
        slidingFigCheck,
        simpleFigCheck
    };

    // ------------------------------
    // Class Creation
    // ------------------------------

    ChessMechanics() = delete;
    explicit ChessMechanics(Board& bd): board(bd) {}

    ChessMechanics(ChessMechanics& other) = default;
    ChessMechanics& operator=(ChessMechanics&) = delete;
    ~ChessMechanics() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] uint64_t GetFullMap() const{
        uint64_t map = 0;
        for(const auto m : board.boards) map |= m;
        return map;
    }

    [[nodiscard]] uint64_t GetColMap(const int col) const {
        uint64_t map = 0;
        for (size_t i = 0; i < Board::BoardsPerCol; ++i) map |= board.boards[Board::BoardsPerCol*col + i];
        return map;
    }

    // Todo: test later wheter accumulation of blocked maps is faster?
    // Todo: AVX applied here?
    // [blockedFigMap, checksCount, checkType]
    [[nodiscard]] std::tuple<uint64_t, uint8_t, uint8_t> GetBlockedFieldMap(const int col, const uint64_t fullMap) const {
        uint8_t checksCount{};
        uint8_t chT{};

        const size_t allyFigInd = col*Board::BoardsPerCol;
        const uint64_t enemyKingMap = board.boards[SwapColor(col)*Board::BoardsPerCol + kingIndex];
        const int enemyKingShift = ConvertToReversedPos(board.kingMSBPositions[SwapColor(col)]);

        // King attacks generation.
        const uint64_t kingBlockedMap = KingMap::GetMoves(board.kingMSBPositions[col]);

        // Pawns attacks generation.
        const uint64_t pawnsMap = board.boards[col*Board::BoardsPerCol + pawnsIndex];
        const uint64_t pawnBlockedMap = col == WHITE
                ? PawnMap::GetWhiteAttackFields(pawnsMap)
                : PawnMap::GetBlackAttackFields(pawnsMap);
        const uint8_t wasCheckedByPawnFlag = (pawnBlockedMap & enemyKingMap) >> enemyKingShift; // = 1 or 0 depending whether hits or not
        checksCount += wasCheckedByPawnFlag;
        chT = simpleFigCheck * wasCheckedByPawnFlag;

        // Knight attacks generation.
        const uint64_t knighBlockedMap = _blockIterativeGenerator(board.boards[allyFigInd + knightsIndex],
            [&](const int pos) { return KnightMap::GetMoves(pos, 0); }
        );
        const uint8_t wasCheckedByKnightFlag = (knighBlockedMap & enemyKingMap) >> enemyKingShift; // = 1 or 0 depending whether hits or not
        checksCount += wasCheckedByKnightFlag;
        chT = simpleFigCheck * wasCheckedByKnightFlag;

        // Rook attacks generation.
        const uint64_t rookBlockedMap = _blockIterativeGenerator(board.boards[allyFigInd + rooksIndex] | board.boards[allyFigInd + queensIndex],
            [&](const int pos) { return RookMap::GetMoves(pos, fullMap); }
        );
        const uint8_t wasCheckedByRookFlag = (rookBlockedMap & enemyKingMap) >> enemyKingShift; // = 1 or 0 depending whether hits or not
        checksCount += wasCheckedByRookFlag;
        chT = slidingFigCheck * wasCheckedByRookFlag;

        // Bishop attacks generation.
        const uint64_t bishopBlockedMap = _blockIterativeGenerator(board.boards[allyFigInd + bishopsIndex] | board.boards[allyFigInd + queensIndex],
            [&](const int pos) { return BishopMap::GetMoves(pos, fullMap); }
        );
        const uint8_t wasCheckedByBishopFlag = (bishopBlockedMap & enemyKingMap) >> enemyKingShift; // = 1 or 0 depending whether hits or not
        checksCount += wasCheckedByBishopFlag;
        chT = slidingFigCheck * wasCheckedByBishopFlag;

        const uint64_t blockedMap = kingBlockedMap | pawnBlockedMap | knighBlockedMap | rookBlockedMap | bishopBlockedMap;
        return { blockedMap, checksCount, chT };
    }

    // TODO: here pinned figs could be processed first to get some slight speed up
    // should only be used when no check is on board
    [[nodiscard]] uint64_t GetPinnedFigsMapWoutCheck(const int col, const uint64_t fullMap) const {
        const size_t enemyCord = SwapColor(col) * Board::BoardsPerCol;
        const size_t allyCord = col * Board::BoardsPerCol;
        const int allyKingShift = ConvertToReversedPos(board.kingMSBPositions[col]);

        const uint64_t rookLinesOnKing = KingMap::pinMasks[board.kingMSBPositions[col]].rookMask;
        const uint64_t suspectedRooks = (board.boards[enemyCord + rooksIndex] | board.boards[enemyCord + queensIndex]) & rookLinesOnKing;
        const uint64_t figsPinnedByRookMoves = _getPinnedFigsWoutCheckGenerator<RookMap>(suspectedRooks, fullMap, rookLinesOnKing, allyCord, allyKingShift);

        const uint64_t bishopLinesOnKing = KingMap::pinMasks[board.kingMSBPositions[col]].bishopMask;
        const uint64_t suspectedBishops = (board.boards[enemyCord + bishopsIndex] | board.boards[enemyCord + queensIndex]) & bishopLinesOnKing;
        const uint64_t figsPinnedByBishopMoves = _getPinnedFigsWoutCheckGenerator<BishopMap>(suspectedBishops, fullMap, bishopLinesOnKing, allyCord, allyKingShift);

        return figsPinnedByRookMoves | figsPinnedByBishopMoves;
    }

    // Todo: check wheter interrupting search when checking figure is found boosts up performance here
    // returns [ pinnedFigMap, allowedTilesMap ]
    [[nodiscard]] std::pair<uint64_t, uint64_t> GetPinnedFigsMapWithCheck(const int col, const uint64_t fullMap) const {
        const size_t enemyCord = SwapColor(col) * Board::BoardsPerCol;
        const size_t allyCord = col * Board::BoardsPerCol;
        const int allyKingShift = ConvertToReversedPos(board.kingMSBPositions[col]);

        // Rook lines search
        const uint64_t rookLinesOnKing = KingMap::pinMasks[board.kingMSBPositions[col]].rookMask;
        const uint64_t suspectedRooks = (board.boards[enemyCord + rooksIndex] | board.boards[enemyCord + queensIndex]) & rookLinesOnKing;
        const auto [figsPinnedByRookMoves, allowedTileRook] =
            _getPinnedFigsWithCheckGenerator<RookMap>(suspectedRooks, fullMap, rookLinesOnKing, allyCord, allyKingShift);

        // TODO: what I meant: if (!allowedTileRook) { ... }
        // Bishop lines search
        const uint64_t bishopLinesOnKing = KingMap::pinMasks[board.kingMSBPositions[col]].bishopMask;
        const uint64_t suspectedBishops = (board.boards[enemyCord + bishopsIndex] | board.boards[enemyCord + queensIndex]) & bishopLinesOnKing;
        const auto [figsPinnedByBishopMoves, allowedTileBishop] =
            _getPinnedFigsWithCheckGenerator<BishopMap>(suspectedBishops, fullMap, bishopLinesOnKing, allyCord, allyKingShift);

        return { figsPinnedByRookMoves | figsPinnedByBishopMoves, allowedTileRook | allowedTileBishop };
    }

    template<
        class ActionT
    >void IterativeBoardTraversal(ActionT action, int depth) {
        if (depth == -1) return;

        const uint64_t fullMap = GetFullMap();
        const auto [ blockedFigMap, checksCount, checkType ] = GetBlockedFieldMap(board.movColor, fullMap);

        switch (checksCount) {
            case 0:
                _noCheckIterativeTraversal(action, depth, fullMap, blockedFigMap);
                break;
            case 1:
                _singleCheckIterativeTraversal(action, depth, fullMap, blockedFigMap, checkType);
                break;
            case 2:
                _doubleCheckIterativeTraversal(action, depth, blockedFigMap);
                break;
#ifndef NDEBUG
            default:
                throw std::runtime_error("[ ERROR ] IterateiveBoardTraversal encountered unrecognized checksCount!\n");
#endif
        }
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    template<
        class ActionT
    >void _noCheckIterativeTraversal(ActionT action, const int depth,
        const uint64_t fullMap, const uint64_t blockedFigMap)
    {
        const uint64_t pinnedFigsMap = GetPinnedFigsMapWoutCheck(board.movColor, fullMap);
        const uint64_t enemyMap = GetColMap(SwapColor(board.movColor));
    }

    template<
        class ActionT
    >void _singleCheckIterativeTraversal(ActionT action, const int depth, const uint64_t fullMap,
        const uint64_t blockedFigMap, const uint8_t checkType)
    {

    }

    template<
        class ActionT
    >void _doubleCheckIterativeTraversal(ActionT action, const int depth, const uint64_t blockedFigMap)
    {
        const uint64_t allyMap = GetColMap(board.movColor);
        _processPlainKingMoves(action, depth, blockedFigMap, allyMap);
    }

    // TODO: Compare with simple if searching loop
    template<
        class ActionT,
        class MapT
    >void _processUnpinnedFigMovesWoutCheck(ActionT action, const int depth, uint64_t& figMap,
        const uint64_t enemyMap, const uint64_t allyMap, const uint64_t pinnedFigMap) {
        const uint64_t fullMap = enemyMap | allyMap;
        uint64_t pinnedOnes = pinnedFigMap & figMap;
        uint64_t unpinnedOnes = figMap ^ pinnedOnes;

        std::array<uint64_t, 5> mapsBackup{};
        for(size_t i = 0; i < 5; ++i) mapsBackup[i] = board.boards[board.movColor*Board::BoardsPerCol + i];

        while(unpinnedOnes) {
            const uint8_t figPos = ExtractMsbPos(unpinnedOnes);
            const uint64_t figBoard = maxMsbPossible >> figPos;
            const uint64_t figMoves = MapT::GetMoves(figPos, fullMap) & ~allyMap;

            uint64_t attackMoves = figMoves & enemyMap;
            uint64_t nonAttackingMoves = figMoves ^ attackMoves;

            while (nonAttackingMoves) {
                const uint8_t movePos = ExtractMsbPos(unpinnedOnes);
                const uint64_t moveBoard = maxMsbPossible >> movePos;

                figMap ^= figBoard;
                figMap |= moveBoard;

                

                nonAttackingMoves ^= moveBoard;
            }


            unpinnedOnes ^= figBoard;
        }
    }

    template<
       class ActionT
    >void _processPlainKingMoves(ActionT action, const int depth, const uint64_t blockedFigMap, const uint64_t allyMap)
    {
        uint64_t kingMoves = KingMap::GetMoves(board.kingMSBPositions[board.movColor]) & ~blockedFigMap & ~allyMap;

        // saving old parameters
        const uint8_t oldKingPos = board.kingMSBPositions[board.movColor];
        const auto oldCastlings = board.Castlings;

        // prohibiting castlings
        board.Castlings[2*board.movColor + KingCastlingIndex] = false;
        board.Castlings[2*board.movColor + QueenCastlingIndex] = false;

        while(kingMoves) {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(kingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // preparing board changes
            board.kingMSBPositions[board.movColor] = newPos;
            board.boards[board.movColor*Board::BoardsPerCol + kingIndex] = newKingBoard;
            board.ChangePlayingColor();

            // performing core actions
            action();
            IterativeBoardTraversal(action, depth-1);
            board.ChangePlayingColor();

            kingMoves ^= newKingBoard;
        }

        // reverting changes
        board.Castlings = oldCastlings;
        board.kingMSBPositions[board.movColor] = oldKingPos;
        board.boards[board.movColor*Board::BoardsPerCol + kingIndex] = maxMsbPossible >> oldKingPos;
    }

    // TODO: simplify ifs??
    template<
        class ActionT
    >void _processKingCastlings(ActionT action, const int depth, const uint64_t blockedFigMap, const uint64_t fullMap) {
        for(size_t i = 0; i < Board::CastlingsPerColor; ++i)
            if (const size_t castlingIndex = board.Castlings[board.movColor*Board::CastlingsPerColor + i];
                board.Castlings[castlingIndex] && (Board::CastlingSensitiveFields[castlingIndex] & blockedFigMap) == 0)
            {
                // processing mvoe and performing cleanup
                board.Castlings[castlingIndex] = false;
                board.kingMSBPositions[board.movColor] = Board::CastlingNewKingPos[castlingIndex];
                board.boards[board.movColor*Board::BoardsPerCol + kingIndex] = maxMsbPossible >> Board::CastlingNewKingPos[castlingIndex];
                board.boards[board.movColor*Board::BoardsPerCol + rooksIndex] ^= Board::CastlingsRookMaps[castlingIndex];
                board.boards[board.movColor*Board::BoardsPerCol + rooksIndex] |= Board::CastlingNewRookMaps[castlingIndex];
                board.ChangePlayingColor();

                // processiong main actions
                action();
                IterativeBoardTraversal(action, depth-1);
                board.ChangePlayingColor();

                // cleaning up after last move
                board.Castlings[castlingIndex] = true;
                board.kingMSBPositions[board.movColor] = Board::DefaultKingPos[board.movColor];
                board.boards[board.movColor*Board::BoardsPerCol + kingIndex] = maxMsbPossible >> Board::DefaultKingPos[board.movColor];
                board.boards[board.movColor*Board::BoardsPerCol + rooksIndex] |= Board::CastlingsRookMaps[castlingIndex];
                board.boards[board.movColor*Board::BoardsPerCol + rooksIndex] ^= Board::CastlingNewRookMaps[castlingIndex];
            }
    }


    template<
        class MoveGeneratorT
    >[[nodiscard]] static uint64_t _blockIterativeGenerator(uint64_t board, MoveGeneratorT mGen)
    {
        uint64_t blockedMap = 0;

        while (board != 0) {
            const int figPos = ExtractMsbPos(board);
            board ^= (minMsbPossible << figPos);

            blockedMap |= mGen(figPos);
        }

        return blockedMap;
    }

    template<
        class MoveMapT
    >[[nodiscard]] uint64_t _getPinnedFigsWoutCheckGenerator(uint64_t suspectedFigs, const uint64_t fullMap,
        const uint64_t suspectedLines, const size_t allyCord, const int allyKingShift) const
    {
        uint64_t pinnedFigMap{};

        while(suspectedFigs != 0) {
            const int msbPos = ExtractMsbPos(suspectedFigs);
            const uint64_t attackedFig = MoveMapT::GetMoves(msbPos, fullMap) & fullMap & suspectedLines;
            const uint64_t mapWoutAttackedFig = fullMap ^ attackedFig;
            const uint64_t secondAttackedFig = MoveMapT::GetMoves(msbPos, mapWoutAttackedFig) & fullMap & suspectedLines;

            const uint64_t pinnedFigFlag = ((secondAttackedFig & board.boards[allyCord + kingIndex]) >> allyKingShift) * attackedFig; // 0 or attackedFig
            pinnedFigMap |= pinnedFigFlag;

            suspectedFigs ^= (maxMsbPossible >> msbPos);
        }

        return pinnedFigMap;
    }

    // returns [ pinnedFigMap, allowedTilesMap ]
    template<
        class MoveMapT
    >[[nodiscard]] std::pair<uint64_t, uint64_t> _getPinnedFigsWithCheckGenerator(uint64_t suspectedFigs, const uint64_t fullMap,
        const uint64_t suspectedLines, const size_t allyCord, const int allyKingShift) const
    {
        uint64_t pinnedFigMap{};
        uint64_t allowedTilesFigMap{};

        while(suspectedFigs != 0) {
            const int msbPos = ExtractMsbPos(suspectedFigs);
            const uint64_t suspectedFigAttackFields = MoveMapT::GetMoves(msbPos, fullMap);
            const uint64_t attackedFig = suspectedFigAttackFields & fullMap & suspectedLines;

            if (attackedFig == board.boards[allyCord + kingIndex]) {
                allowedTilesFigMap |= suspectedFigAttackFields | (maxMsbPossible >> msbPos);
            }

            const uint64_t mapWoutAttackedFig = fullMap ^ attackedFig;
            const uint64_t secondAttackedFig = MoveMapT::GetMoves(msbPos, mapWoutAttackedFig) & fullMap & suspectedLines;

            const uint64_t pinnedFigFlag = ((secondAttackedFig & board.boards[allyCord + kingIndex]) >> allyKingShift) * attackedFig; // 0 or attackedFig
            pinnedFigMap |= pinnedFigFlag;

            suspectedFigs ^= (maxMsbPossible >> msbPos);
        }

        return { pinnedFigMap, allowedTilesFigMap };
    }

    // ------------------------------
    // inner class types
    // ------------------------------



    // ------------------------------
    // Class fields
    // ------------------------------

    Board& board;
};



#endif //CHESSMECHANICS_H
