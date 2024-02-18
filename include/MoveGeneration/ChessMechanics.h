//
// Created by Jlisowskyy on 12/31/23.
//

#ifndef CHESSMECHANICS_H
#define CHESSMECHANICS_H

#include <array>

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "BishopMap.h"
#include "KingMap.h"
#include "KnightMap.h"
#include "WhitePawnMap.h"
#include "BlackPawnMap.h"
#include "QueenMap.h"
#include "RookMap.h"


/*              General optimizations TODO:
 *     - test all msb and lsb extractions
 *     - add and test AVX support
 *     - test other ideas for hashing
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

    [[nodiscard]] uint64_t GetFullMap() const;
    [[nodiscard]] uint64_t GetColMap(int col) const;

    // [blockedFigMap, checksCount, checkType]
    [[nodiscard]] std::tuple<uint64_t, uint8_t, uint8_t> GetBlockedFieldMap(uint64_t fullMap) const;

    // should only be used when no check is on board
    [[nodiscard]] uint64_t GetPinnedFigsMapWoutCheck(int col, uint64_t fullMap) const;

    // returns [ pinnedFigMap, allowedTilesMap ]
    [[nodiscard]] std::pair<uint64_t, uint64_t> GetPinnedFigsMapWithCheck(int col, uint64_t fullMap) const;

    [[nodiscard]] std::vector<Board> GetPossibleMoveSlow();

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
    
    template<
        class ActionT
    >void IterativeBoardTraversal(ActionT action, int depth) {
        if (depth == 0) {
            action(board);
            return;
        }

        const uint64_t fullMap = GetFullMap();
        const auto [ blockedFigMap, checksCount, checkType ] = GetBlockedFieldMap(fullMap);

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

    // Todo: el passnt
    template<
        class ActionT
    >void _noCheckIterativeTraversal(ActionT action, const int depth,
        const uint64_t fullMap, const uint64_t blockedFigMap)
    {
        const uint64_t pinnedFigsMap = GetPinnedFigsMapWoutCheck(board.movColor, fullMap);
        const uint64_t enemyMap = GetColMap(SwapColor(board.movColor));
        const uint64_t allyMap = GetColMap(board.movColor);

        _processFigMoves<ActionT, RookMap, true>(action, depth,
            board.boards[Board::BoardsPerCol*board.movColor + rooksIndex], enemyMap, allyMap,  pinnedFigsMap);

        _processFigMoves<ActionT, BishopMap>(action, depth,
            board.boards[Board::BoardsPerCol*board.movColor + bishopsIndex], enemyMap, allyMap,  pinnedFigsMap);

        _processFigMoves<ActionT, QueenMap>(action, depth,
            board.boards[Board::BoardsPerCol*board.movColor + queensIndex], enemyMap, allyMap,  pinnedFigsMap);

        _processFigMoves<ActionT, KnightMap>(action, depth,
            board.boards[Board::BoardsPerCol*board.movColor + knightsIndex], enemyMap, allyMap,  pinnedFigsMap);

        if (board.movColor == WHITE)
            _processPawnMoves<ActionT, WhitePawnMap>(action, depth,
                board.boards[Board::BoardsPerCol*board.movColor + pawnsIndex], enemyMap, allyMap, pinnedFigsMap);
        else
            _processPawnMoves<ActionT, BlackPawnMap>(action, depth,
                board.boards[Board::BoardsPerCol*board.movColor + pawnsIndex], enemyMap, allyMap, pinnedFigsMap);

        _processPlainKingMoves(action, depth, blockedFigMap, allyMap, enemyMap);

        _processKingCastlings(action, depth, blockedFigMap, fullMap);
    }

    template<
        class ActionT
    >void _singleCheckIterativeTraversal(ActionT action, const int depth, const uint64_t fullMap,
        const uint64_t blockedFigMap, const uint8_t checkType)
    {
        // TODO: IMPLEMENT CHECKTYPE !!! THERE ARE MASSIVE UNIMPLEMENTED IDEAS
        const auto [pinnedFigsMap, allowedTilesMap] = GetPinnedFigsMapWithCheck(board.movColor, fullMap);
        const uint64_t enemyMap = GetColMap(SwapColor(board.movColor));
        const uint64_t allyMap = GetColMap(board.movColor);

        _processFigMoves<ActionT, RookMap, true, false, false, true>(action, depth,
            board.boards[Board::BoardsPerCol*board.movColor + rooksIndex],
            enemyMap, allyMap,  pinnedFigsMap, allowedTilesMap);

        _processFigMoves<ActionT, BishopMap, false, false, false, true>(action, depth,
            board.boards[Board::BoardsPerCol*board.movColor + bishopsIndex],
            enemyMap, allyMap,  pinnedFigsMap, allowedTilesMap);

        _processFigMoves<ActionT, QueenMap, false, false, false, true>(action, depth,
            board.boards[Board::BoardsPerCol*board.movColor + queensIndex],
            enemyMap, allyMap,  pinnedFigsMap, allowedTilesMap);

        _processFigMoves<ActionT, KnightMap, false, false, false, true>(action, depth,
            board.boards[Board::BoardsPerCol*board.movColor + knightsIndex],
            enemyMap, allyMap,  pinnedFigsMap, allowedTilesMap);

        if (board.movColor == WHITE)
            _processPawnMoves<ActionT, WhitePawnMap, true>(action, depth,
                board.boards[Board::BoardsPerCol*board.movColor + pawnsIndex],
                enemyMap, allyMap, pinnedFigsMap, allowedTilesMap);
        else
            _processPawnMoves<ActionT, BlackPawnMap, true>(action, depth,
                board.boards[Board::BoardsPerCol*board.movColor + pawnsIndex],
                enemyMap, allyMap, pinnedFigsMap, allowedTilesMap);

        _processPlainKingMoves(action, depth, blockedFigMap, allyMap, enemyMap);
    }

    template<
        class ActionT
    >void _doubleCheckIterativeTraversal(ActionT action, const int depth, const uint64_t blockedFigMap)
    {
        const uint64_t allyMap = GetColMap(board.movColor);
        const uint64_t enemyMap = GetColMap(SwapColor(board.movColor));
        _processPlainKingMoves(action, depth, blockedFigMap, allyMap, enemyMap);
    }

    template<
        class ActionT,
        class MapT,
        bool isCheck = false
    >void _processPawnMoves(ActionT action, const int depth, uint64_t& figMap,
        const uint64_t enemyMap, const uint64_t allyMap, const uint64_t pinnedFigMap, [[maybe_unused]] const uint64_t allowedMoveFillter = 0)
    {
        const uint64_t promotingPawns = figMap & MapT::PromotingMask;
        const uint64_t nonPromotingPawns = figMap ^ promotingPawns;

        _processFigMoves<ActionT, MapT, false, false, true, isCheck, MapT::ElPassantMask>(action, depth, figMap, enemyMap,
            allyMap,  pinnedFigMap, nonPromotingPawns, allowedMoveFillter);

        _processFigMoves<ActionT, MapT, false, true, true, isCheck, MapT::ElPassantMask>(action, depth, figMap, enemyMap,
            allyMap,  pinnedFigMap, promotingPawns, allowedMoveFillter);
    }

    template<
        class ActionT,
        class MapT,
        bool isCheck = false
    >void _processElPassantMoves(ActionT action, const int depth, uint64_t& figMap) {
        if (board.elPassantField == INVALID) return;


    }

    // TODO: Compare with simple if searching loop
    // TODO: propagate checkForCastling?
    template<
        class ActionT,
        class MapT,
        bool checkForCastling = false,
        bool promotePawns = false,
        bool selectFigures = false,
        bool isCheck = false,
        uint64_t elPassantLine = 0LLU
    >void _processFigMoves(ActionT action, const int depth, uint64_t& figMap,
        const uint64_t enemyMap, const uint64_t allyMap, const uint64_t pinnedFigMap,
        [[maybe_unused]] const uint64_t figureSelector = 0, [[maybe_unused]] const uint64_t allowedMovesSelector = 0)
    {
        const uint64_t fullMap = enemyMap | allyMap;
        uint64_t pinnedOnes = pinnedFigMap & figMap;
        uint64_t unpinnedOnes = figMap ^ pinnedOnes;

        if constexpr (selectFigures) {
            pinnedOnes &= figureSelector;
            unpinnedOnes &= figureSelector;
        }

        // to restore destroyed figures during attack moves
        std::array<uint64_t, 5> mapsBackup{};
        for(size_t i = 0; i < 5; ++i) mapsBackup[i] = board.boards[SwapColor(board.movColor)*Board::BoardsPerCol + i];

        // saving results of previous el passant field, used only when figure is not a pawn
        const Field oldElpassant = board.elPassantField;
        board.elPassantField = INVALID;

        // procesing unpinned moves
        while(unpinnedOnes) {
            // processing moves
            const uint8_t figPos = ExtractMsbPos(unpinnedOnes);
            const uint64_t figBoard = maxMsbPossible >> figPos;
            const uint64_t figMoves = [&]() constexpr{
                if constexpr (!isCheck)
                    return MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap;
                if constexpr (isCheck)
                    return MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap & allowedMovesSelector;
            }();

            // Performing checks for castlings
            // TODO: test in future performance affections
            [[maybe_unused]] ssize_t castlingIndex = -1;
            if constexpr (checkForCastling) {
                for (size_t i = 0; i < Board::CastlingsPerColor; ++i)
                    if (const size_t index = board.movColor*Board::CastlingsPerColor + i;
                        board.Castlings[index] && (Board::CastlingsRookMaps[index] & figBoard) != 0)
                    {
                        board.Castlings[index] = false;
                        castlingIndex = reinterpret_cast<size_t>(index);
                    }
            }

            // preparing moves
            const uint64_t attackMoves = figMoves & enemyMap;
            const uint64_t nonAttackingMoves = figMoves ^ attackMoves;
            figMap ^= figBoard;

            // processing move consequneces
            _processNonAttackingMoves<ActionT, promotePawns, elPassantLine>(action, depth, figMap, nonAttackingMoves);
            if (attackMoves) _processAttackingMoves<ActionT, promotePawns>(action ,depth, figMap, attackMoves, mapsBackup);

            // cleaning up
            figMap |= figBoard;
            if constexpr (checkForCastling)
                if (castlingIndex != -1)
                    board.Castlings[castlingIndex] = true;

            unpinnedOnes ^= figBoard;
        }

        // if check is detected pinned figure stays in place
        if constexpr (isCheck) {
            // previous el passant state should be restored
            board.elPassantField = oldElpassant;
            return;
        }

        // procesing pinned moves
        while(pinnedOnes) {
            // processing moves
            const uint8_t figPos = ExtractMsbPos(pinnedOnes);
            const uint64_t figBoard = maxMsbPossible >> figPos;
            const uint64_t allowedTiles = _generateAllowedTilesForPrecisedPinnedFig(figBoard, fullMap);
            const uint64_t figMoves = MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap & allowedTiles;

            // preparing moves
            const uint64_t attackMoves = figMoves & enemyMap;
            const uint64_t nonAttackingMoves = figMoves ^ attackMoves;
            figMap ^= figBoard;

            // processing move consequences
            _processNonAttackingMoves<ActionT, promotePawns, elPassantLine>(action, depth, figMap, nonAttackingMoves);
            if (attackMoves) _processAttackingMoves<ActionT, promotePawns>(action ,depth, figMap, attackMoves, mapsBackup); // TODO: There is exactly one move possible

            figMap |= figBoard;
            pinnedOnes ^= figBoard;
        }

        // previous el passant state should be restored
        board.elPassantField = oldElpassant;
    }

    [[nodiscard]] uint64_t _generateAllowedTilesForPrecisedPinnedFig(uint64_t figBoard, uint64_t fullMap) const;

    // TODO: improve readability of code below
    template<
        class ActionT,
        bool promotePawns,
        uint64_t elPassantLine = 0LLU
    >void _processNonAttackingMoves(ActionT action, const int depth, uint64_t& figMap, uint64_t nonAttackingMoves)
    {
        while (nonAttackingMoves) {
            // extracting moves
            const uint8_t movePos = ExtractMsbPos(nonAttackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;

            // TODO: could be shortened with lambda but dont like them sry
            if constexpr (!promotePawns)
                // simple figure case
            {
                // if el passant line is passed when figure moved to these line flag will turned on
                if constexpr (elPassantLine != 0LLU) {
                    if ((elPassantLine & moveBoard) != 0)
                        board.elPassantField = static_cast<Field>(moveBoard);
                }

                // applying moves
                figMap |= moveBoard;

                // performing core actions
                board.ChangePlayingColor();
                IterativeBoardTraversal(action, depth-1);
                board.ChangePlayingColor();

                // reverting flag changes
                if constexpr (elPassantLine != 0LLU) {
                    board.elPassantField = INVALID;
                }

                // cleaning up
                figMap ^= moveBoard;
            }
            if constexpr (promotePawns)
                // upgrading pawn case
            {
                const size_t colIndex = board.movColor*Board::BoardsPerCol;
                for (size_t i = knightsIndex; i < kingIndex; ++i) {

                    // applying moves
                    board.boards[colIndex + i] |= moveBoard;

                    // performing core actions
                    board.ChangePlayingColor();
                    IterativeBoardTraversal(action, depth-1);
                    board.ChangePlayingColor();

                    // cleaning up
                    board.boards[colIndex + i] ^= moveBoard;
                }
            }

            nonAttackingMoves ^= moveBoard;
        }
    }

    template<
        class ActionT,
        bool promotePawns
    >void _processAttackingMoves(ActionT action, const int depth, uint64_t& figMap,
        uint64_t attackingMoves, const std::array<uint64_t, 5>& backup)
    {
        while (attackingMoves) {
            // extracting moves
            const uint8_t movePos = ExtractMsbPos(attackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;
            const size_t enemyColInd = SwapColor(board.movColor)*Board::BoardsPerCol;

            // removing enemy pieces
            for(size_t i = 0; i < 5; ++i) board.boards[enemyColInd + i] &= ~moveBoard;

            if constexpr (!promotePawns)
                // simple figure case
            {
                // applying moves
                figMap |= moveBoard;

                // performing core actions
                board.ChangePlayingColor();
                IterativeBoardTraversal(action, depth-1);
                board.ChangePlayingColor();

                // cleaning up
                figMap ^= moveBoard;
            }
            if constexpr (promotePawns)
                // upgrading pawn case
            {
                const size_t colIndex = board.movColor*Board::BoardsPerCol;
                for (size_t i = knightsIndex; i < kingIndex; ++i) {

                    // applying moves
                    board.boards[colIndex + i] |= moveBoard;

                    // performing core actions
                    board.ChangePlayingColor();
                    IterativeBoardTraversal(action, depth-1);
                    board.ChangePlayingColor();

                    // cleaning up
                    board.boards[colIndex + i] ^= moveBoard;
                }
            }

            for(size_t i = 0; i < 5; ++i) board.boards[enemyColInd + i] = backup[i];
            attackingMoves ^= moveBoard;
        }
    }

    // TODO: test copying all old castlings
    template<
       class ActionT
    >void _processPlainKingMoves(ActionT action, const int depth, const uint64_t blockedFigMap,
        const uint64_t allyMap, const uint64_t enemyMap)
    {
        static constexpr size_t CastlingPerColor = 2;

        // generating moves
        const uint64_t kingMoves = KingMap::GetMoves(board.kingMSBPositions[board.movColor]) & ~blockedFigMap & ~allyMap;
        const size_t movingColorIndex = board.movColor*Board::BoardsPerCol;
        const size_t enemyColorIndex = SwapColor(board.movColor)*Board::BoardsPerCol;
        uint64_t attackingMoves = kingMoves & enemyMap;
        uint64_t nonAttackingMoves = kingMoves ^ attackingMoves;

        // saving old parameters
        const uint8_t oldKingPos = board.kingMSBPositions[board.movColor];
        const auto oldCastlings = board.Castlings;
        const Field oldElPassant = board.elPassantField;

        // prohibiting castlings
        board.Castlings[CastlingPerColor*board.movColor + KingCastlingIndex] = false;
        board.Castlings[CastlingPerColor*board.movColor + QueenCastlingIndex] = false;

        //prohibiting elPassant
        board.elPassantField = INVALID;

        // processing simple non attacking moves
        while(nonAttackingMoves) {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(nonAttackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // preparing board changes
            board.kingMSBPositions[board.movColor] = newPos;
            board.boards[movingColorIndex + kingIndex] = newKingBoard;


            // performing core actions
            board.ChangePlayingColor();
            IterativeBoardTraversal(action, depth-1);
            board.ChangePlayingColor();

            nonAttackingMoves ^= newKingBoard;
        }

        if (!attackingMoves) return;

        // processing slightly more complicated attacking moves
        std::array<uint64_t, 5> mapsBackup{};
        for(size_t i = 0; i < 5; ++i) mapsBackup[i] = board.boards[enemyColorIndex + i];
        while(attackingMoves) {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(attackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // preparing board changes
            for(size_t i = 0; i < 5; ++i) board.boards[enemyColorIndex + i] &= ~newKingBoard;
            board.kingMSBPositions[board.movColor] = newPos;
            board.boards[movingColorIndex + kingIndex] = newKingBoard;

            // performing core actions
            board.ChangePlayingColor();
            IterativeBoardTraversal(action, depth-1);

            // cleaning up
            board.ChangePlayingColor();
            for(size_t i = 0; i < 5; ++i) board.boards[enemyColorIndex + i] = mapsBackup[i];

            attackingMoves ^= newKingBoard;
        }

        // reverting changes
        board.Castlings = oldCastlings;
        board.kingMSBPositions[board.movColor] = oldKingPos;
        board.boards[board.movColor*Board::BoardsPerCol + kingIndex] = maxMsbPossible >> oldKingPos;
        board.elPassantField = oldElPassant;
    }

    // TODO: simplify ifs??
    template<
        class ActionT
    >void _processKingCastlings(ActionT action, const int depth, const uint64_t blockedFigMap, const uint64_t fullMap) {
        for(size_t i = 0; i < Board::CastlingsPerColor; ++i)
            if (const size_t castlingIndex = board.movColor*Board::CastlingsPerColor + i;
                board.Castlings[castlingIndex] && (Board::CastlingSensitiveFields[castlingIndex] & blockedFigMap) == 0
                && (Board::CastlingSensitiveFields[castlingIndex] & fullMap) == 0)
            {
                // processing mvoe and performing cleanup
                board.Castlings[castlingIndex] = false;
                board.kingMSBPositions[board.movColor] = Board::CastlingNewKingPos[castlingIndex];
                board.boards[board.movColor*Board::BoardsPerCol + kingIndex] = maxMsbPossible >> Board::CastlingNewKingPos[castlingIndex];
                board.boards[board.movColor*Board::BoardsPerCol + rooksIndex] ^= Board::CastlingsRookMaps[castlingIndex];
                board.boards[board.movColor*Board::BoardsPerCol + rooksIndex] |= Board::CastlingNewRookMaps[castlingIndex];
                const Field oldElPassant = board.elPassantField;
                board.elPassantField = INVALID;

                // processiong main actions
                board.ChangePlayingColor();
                IterativeBoardTraversal(action, depth-1);
                board.ChangePlayingColor();

                // cleaning up after last move
                board.Castlings[castlingIndex] = true;
                board.kingMSBPositions[board.movColor] = Board::DefaultKingPos[board.movColor];
                board.boards[board.movColor*Board::BoardsPerCol + kingIndex] = maxMsbPossible >> Board::DefaultKingPos[board.movColor];
                board.boards[board.movColor*Board::BoardsPerCol + rooksIndex] |= Board::CastlingsRookMaps[castlingIndex];
                board.boards[board.movColor*Board::BoardsPerCol + rooksIndex] ^= Board::CastlingNewRookMaps[castlingIndex];
                board.elPassantField = oldElPassant;
            }
    }


    template<
        class MoveGeneratorT
    >[[nodiscard]] static uint64_t _blockIterativeGenerator(uint64_t board, MoveGeneratorT mGen)
    {
        uint64_t blockedMap = 0;

        while (board != 0) {
            const int figPos = ExtractMsbPos(board);
            board ^= (maxMsbPossible >> figPos);

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

            const uint64_t pinnedFigFlag = ((secondAttackedFig & board.boards[allyCord + kingIndex]) >> allyKingShift)*attackedFig; // 0 or attackedFig
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

            const uint64_t pinnedFigFlag = ((secondAttackedFig & board.boards[allyCord + kingIndex]) >> allyKingShift)*attackedFig; // 0 or attackedFig
            pinnedFigMap |= pinnedFigFlag;

            suspectedFigs ^= (maxMsbPossible >> msbPos);
        }

        return { pinnedFigMap, allowedTilesFigMap };
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    Board& board;
};

#endif //CHESSMECHANICS_H
