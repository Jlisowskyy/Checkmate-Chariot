//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <array>

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "ChessMechanics.h"
#include "BishopMap.h"
#include "KingMap.h"
#include "KnightMap.h"
#include "WhitePawnMap.h"
#include "BlackPawnMap.h"
#include "QueenMap.h"
#include "RookMap.h"
#include "Move.h"

struct MoveGenerator
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

    static constexpr size_t AverageChessMoves = 40;
    MoveGenerator() = delete;

    explicit MoveGenerator(Board&bd): mechanics(bd), board(bd)
    {
    }

    MoveGenerator(MoveGenerator&other) = delete;

    MoveGenerator& operator=(MoveGenerator&) = delete;

    ~MoveGenerator() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] bool IsCheck() const
    {
        return mechanics.IsCheck();
    }

    std::vector<Move> GetMovesFast()
    {
        const uint64_t fullMap = mechanics.GetFullMap();
        const auto [blockedFigMap, checksCount, checkType] = mechanics.GetBlockedFieldMap(fullMap);

        std::vector<Move> results{};
        results.reserve(AverageChessMoves);

        switch (checksCount)
        {
            case 0:
                _noCheckGen(results, fullMap, blockedFigMap);
            break;
            case 1:
                _singleCheckGen(results, fullMap, blockedFigMap, checkType);
            break;
            case 2:
                _doubleCheckGen(results, blockedFigMap);
            break;
        }

        return results;
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:
    void _noCheckGen(std::vector<Move>& results, const uint64_t fullMap, const uint64_t blockedFigMap)
    {
        const uint64_t pinnedFigsMap = mechanics.GetPinnedFigsMapWoutCheck(board.movColor, fullMap);
        const uint64_t enemyMap = mechanics.GetColMap(SwapColor(board.movColor));
        const uint64_t allyMap = mechanics.GetColMap(board.movColor);

        _processFigMoves<RookMap, true>(results, enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<BishopMap>(results,
                                             enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<QueenMap>(results, enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<KnightMap>(results,
                                             enemyMap, allyMap, pinnedFigsMap);

        if (board.movColor == WHITE)
            _processPawnMoves<WhitePawnMap>(results,
                                                     enemyMap, allyMap, pinnedFigsMap);
        else
            _processPawnMoves<BlackPawnMap>(results,
                                                     enemyMap, allyMap, pinnedFigsMap);

        _processPlainKingMoves(results, blockedFigMap, allyMap, enemyMap);

        _processKingCastlings(results, blockedFigMap, fullMap);
    }

    void _singleCheckGen(std::vector<Move>& results, const uint64_t fullMap, const uint64_t blockedFigMap, const uint8_t checkType)
    {
        static constexpr uint64_t UNUSED = 0;

        // simplifying figure search by distinguishing check types
        const auto [pinnedFigsMap, allowedTilesMap] = [&]() -> std::pair<uint64_t, uint64_t>
        {
            if (checkType == slidingFigCheck)
                return mechanics.GetPinnedFigsMapWithCheck(board.movColor, fullMap);

            auto pinned = mechanics.GetPinnedFigsMapWoutCheck(board.movColor, fullMap);
            return {pinned, mechanics.GetAllowedTilesWhenCheckedByNonSliding()};
        }();

        // helping variable preparation
        const uint64_t enemyMap = mechanics.GetColMap(SwapColor(board.movColor));
        const uint64_t allyMap = mechanics.GetColMap(board.movColor);

        // Specific figure processing
        _processFigMoves<RookMap, true, false, false, true>(results,
                                                                     enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                                     allowedTilesMap);

        _processFigMoves<BishopMap, false, false, false, true>(results,
                                                                        enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                                        allowedTilesMap);

        _processFigMoves<QueenMap, false, false, false, true>(results,
                                                                       enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                                       allowedTilesMap);

        _processFigMoves<KnightMap, false, false, false, true>(results,
                                                                        enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                                        allowedTilesMap);

        if (board.movColor == WHITE)
            _processPawnMoves<WhitePawnMap, true>(results,
                                                           enemyMap, allyMap, pinnedFigsMap, allowedTilesMap);
        else
            _processPawnMoves<BlackPawnMap, true>(results,
                                                           enemyMap, allyMap, pinnedFigsMap, allowedTilesMap);


        _processKingMovesWhenChecked(results, blockedFigMap, allyMap, enemyMap, allowedTilesMap);
    }

    void _doubleCheckGen(std::vector<Move>& results, const uint64_t blockedFigMap)
    {
        const uint64_t allyMap = mechanics.GetColMap(board.movColor);
        const uint64_t enemyMap = mechanics.GetColMap(SwapColor(board.movColor));
        _processPlainKingMoves(results, blockedFigMap, allyMap, enemyMap);
    }

    template<
        class MapT,
        bool isCheck = false
    >
    void _processPawnMoves(std::vector<Move>& results, const uint64_t enemyMap, const uint64_t allyMap, const uint64_t pinnedFigMap,
                           [[maybe_unused]] const uint64_t allowedMoveFillter = 0)
    {
        const uint64_t promotingPawns = board.boards[MapT::GetBoardIndex(0)] & MapT::PromotingMask;
        const uint64_t nonPromotingPawns = board.boards[MapT::GetBoardIndex(0)] ^ promotingPawns;

        _processFigMoves<MapT, false, false, true, isCheck, MapT::GetElPassantField>(results, enemyMap,
            allyMap, pinnedFigMap, nonPromotingPawns, allowedMoveFillter);

        if (promotingPawns)
            _processFigMoves<MapT, false, true, true, isCheck>(results, enemyMap,
                                                                    allyMap, pinnedFigMap, promotingPawns,
                                                                    allowedMoveFillter);

        _processElPassantMoves<MapT, isCheck>(results, allyMap | enemyMap, pinnedFigMap,
             allowedMoveFillter);
    }

    // TODO: Consider different soluition?
    template<
        class MapT,
        bool isCheck = false
    >
    void _processElPassantMoves(std::vector<Move>& results, const uint64_t fullMap, const uint64_t pinnedFigMap,
                                [[maybe_unused]] const uint64_t allowedMoveFillter = 0)
    {
        if (board.elPassantField == Board::InvalidElPassantField) return;

        // calculation preparation
        const uint64_t suspectedFields = MapT::GetElPassantSuspectedFields(board.elPassantField);
        const size_t enemyCord = SwapColor(board.movColor) * Board::BoardsPerCol;
        const uint64_t enemyRookFigs = board.boards[enemyCord + queensIndex] | board.boards[enemyCord + rooksIndex];
        uint64_t possiblePawnsToMove = board.boards[MapT::GetBoardIndex(0)] & suspectedFields;

        while (possiblePawnsToMove)
        {
            const uint64_t pawnMap = maxMsbPossible >> ExtractMsbPos(possiblePawnsToMove);

            // checking wheteher move would affect horizontal line attacks on king
            const uint64_t processedPawns = pawnMap | board.elPassantField;
            const uint64_t cleanedFromPawnsMap = fullMap ^ processedPawns;
            if (const uint64_t kingHorizontalLine =
                        RookMap::GetMoves(board.GetKingMsbPos(board.movColor), cleanedFromPawnsMap) &
                        MapT::EnemyElPassantMask;
                (kingHorizontalLine & enemyRookFigs) != 0)
                return;

            const uint64_t moveMap = MapT::GetElPassantMoveField(board.elPassantField);

            // checking wheter moving some pawns would undercover king on some line -
            if ((processedPawns & pinnedFigMap) != 0)
            {
                // two separate situations thats need to be considered, every pawn that participate in el passant move
                // should be unblocked on specific lines

                if ((pawnMap & pinnedFigMap) != 0 && (
                        mechanics.GenerateAllowedTilesForPrecisedPinnedFig(pawnMap, fullMap) & moveMap) == 0) {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }
                if ((mechanics.GenerateAllowedTilesForPrecisedPinnedFig(board.elPassantField, fullMap) & moveMap) == 0) {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }
            }

            // When king is checked only if move is going to allowed tile el passant is correct
            if constexpr (isCheck)
                if ((moveMap & allowedMoveFillter) == 0 && (board.elPassantField & allowedMoveFillter) == 0) {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }


            // preparing and sending move
            Move mv{};
            mv.SetCasltingRights(board.Castlings);
            mv.SetStartField(ExtractMsbPos(pawnMap));
            mv.SetStartBoardIndex(MapT::GetBoardIndex(0));
            mv.SetTargetBoardIndex(MapT::GetBoardIndex(0));
            mv.SetTargetField(ExtractMsbPos(moveMap));
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetKilledBoardIndex(MapT::GetEnemyPawnBoardIndex());
            mv.SetKilledFigureField(board.elPassantField);
            results.push_back(mv);

            possiblePawnsToMove ^= pawnMap;
        }
    }

    // TODO: Compare with simple if searching loop
    // TODO: propagate checkForCastling?
    template<
        class MapT,
        bool checkForCastling = false,
        bool promotePawns = false,
        bool selectFigures = false,
        bool isCheck = false,
        uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr
    >
    void _processFigMoves(std::vector<Move>& results, const uint64_t enemyMap, const uint64_t allyMap, const uint64_t pinnedFigMap,
                          [[maybe_unused]] const uint64_t figureSelector = 0,
                          [[maybe_unused]] const uint64_t allowedMovesSelector = 0)
    {
        const uint64_t fullMap = enemyMap | allyMap;
        uint64_t pinnedOnes = pinnedFigMap & board.boards[MapT::GetBoardIndex(board.movColor)];
        uint64_t unpinnedOnes = board.boards[MapT::GetBoardIndex(board.movColor)] ^ pinnedOnes;

        // applying filter if needed
        if constexpr (selectFigures)
        {
            pinnedOnes &= figureSelector;
            unpinnedOnes &= figureSelector;
        }

        // saving results of previous el passant field, used only when figure is not a pawn


        // procesing unpinned moves
        while (unpinnedOnes)
        {
            // processing moves
            const uint8_t figPos = ExtractMsbPos(unpinnedOnes);
            const uint64_t figBoard = maxMsbPossible >> figPos;

            // selecting allowed moves if in check
            const uint64_t figMoves = [&]() constexpr
            {
                if constexpr (!isCheck)
                    return MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap;
                if constexpr (isCheck)
                    return MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap & allowedMovesSelector;
            }();

            // Performing checks for castlings
            std::array<bool, Board::CastlingCount+1> updatedCastlings = board.Castlings;
            if constexpr (checkForCastling)
                updatedCastlings[RookMap::GetMatchingCastlingIndex(board, figBoard)] = false;

            // preparing moves
            const uint64_t attackMoves = figMoves & enemyMap;
            const uint64_t nonAttackingMoves = figMoves ^ attackMoves;

            // processing move consequneces
            _processNonAttackingMoves<promotePawns, elPassantFieldDeducer>(results, nonAttackingMoves, MapT::GetBoardIndex(board.movColor), figBoard, updatedCastlings);
            _processAttackingMoves<promotePawns>(results, attackMoves, MapT::GetBoardIndex(board.movColor), figBoard, updatedCastlings);

            unpinnedOnes ^= figBoard;
        }

        // if check is detected pinned figure stays in place
        if constexpr (isCheck)
            return;

        // procesing pinned moves
        // Note: corner Rook possibly applicable to castling cannot be pinned
        while (pinnedOnes)
        {
            // processing moves
            const uint8_t figPos = ExtractMsbPos(pinnedOnes);
            const uint64_t figBoard = maxMsbPossible >> figPos;
            const uint64_t allowedTiles = mechanics.GenerateAllowedTilesForPrecisedPinnedFig(figBoard, fullMap);
            const uint64_t figMoves = MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap & allowedTiles;
            // TODO: ischeck applid here?
            // TODO: breaking if there?

            // preparing moves
            const uint64_t attackMoves = figMoves & enemyMap;
            const uint64_t nonAttackingMoves = figMoves ^ attackMoves;

            // processing move consequences
            _processNonAttackingMoves<promotePawns, elPassantFieldDeducer>(results, nonAttackingMoves, MapT::GetBoardIndex(board.movColor), figBoard, board.Castlings);
            // TODO: There is exactly one move possible
            _processAttackingMoves<promotePawns>(results, attackMoves, MapT::GetBoardIndex(board.movColor), figBoard, board.Castlings);

            pinnedOnes ^= figBoard;
        }
    }

    // TODO: improve readability of code below
    template<
        bool promotePawns,
        uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr
    >
    void _processNonAttackingMoves(std::vector<Move>& results, uint64_t nonAttackingMoves, const size_t figBoardIndex, const uint64_t startField,
        const std::array<bool, Board::CastlingCount+1>& castlings
        ) const
    {
        while (nonAttackingMoves)
        {
            // extracting moves
            const uint8_t movePos = ExtractMsbPos(nonAttackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;

            if constexpr (!promotePawns)
            // simple figure case
            {
                Move mv{};
                mv.SetCasltingRights(castlings);
                mv.SetStartField(ExtractMsbPos(startField));
                mv.SetStartBoardIndex(figBoardIndex);
                mv.SetTargetBoardIndex(figBoardIndex);
                mv.SetTargetField(movePos);
                mv.SetKilledBoardIndex(Board::SentinelBoardIndex);

                // if el passant line is passed when figure moved to these line flag will turned on
                if constexpr (elPassantFieldDeducer != nullptr)
                    mv.SetElPassantField(ExtractMsbPos(elPassantFieldDeducer(moveBoard, startField)));
                else
                    mv.SetElPassantField(Board::InvalidElPassantField);

                results.push_back(mv);
            }
            if constexpr (promotePawns)
            // upgrading pawn case
            {
                // iterating through upgradable pieces
                for (size_t i = knightsIndex; i < kingIndex; ++i)
                {
                    Move mv{};
                    mv.SetCasltingRights(castlings);
                    mv.SetStartField(ExtractMsbPos(startField));
                    mv.SetStartBoardIndex(figBoardIndex);
                    mv.SetTargetBoardIndex(board.movColor * Board::BoardsPerCol + i);
                    mv.SetTargetField(movePos);
                    mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
                    mv.SetElPassantField(Board::InvalidElPassantField);

                    results.push_back(mv);
                }
            }

            nonAttackingMoves ^= moveBoard;
        }
    }

    template<
        bool promotePawns
    >
    void _processAttackingMoves(std::vector<Move>& results, uint64_t attackingMoves, const size_t figBoardIndex, const uint64_t startField,
        const std::array<bool, Board::CastlingCount+1>& castlings) const
    {
        while (attackingMoves)
        {
            // extracting moves
            const uint8_t movePos = ExtractMsbPos(attackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;
            const size_t attackedFigBoardIndex = mechanics.GetIndexOfContainingBoard(moveBoard, SwapColor(board.movColor));

            if constexpr (!promotePawns)
            // simple figure case
            {
                Move mv{};
                mv.SetCasltingRights(castlings);
                mv.SetStartField(ExtractMsbPos(startField));
                mv.SetStartBoardIndex(figBoardIndex);
                mv.SetTargetBoardIndex(figBoardIndex);
                mv.SetTargetField(movePos);
                mv.SetKilledBoardIndex(attackedFigBoardIndex);
                mv.SetKilledFigureField(movePos);
                mv.SetElPassantField(Board::InvalidElPassantField);

                results.push_back(mv);
            }
            if constexpr (promotePawns)
            // upgrading pawn case
            {

                // iterating through upgradable pieces
                for (size_t i = knightsIndex; i < kingIndex; ++i)
                {
                    Move mv{};
                    mv.SetCasltingRights(castlings);
                    mv.SetStartField(ExtractMsbPos(startField));
                    mv.SetStartBoardIndex(figBoardIndex);
                    mv.SetTargetBoardIndex(board.movColor * Board::BoardsPerCol + i);
                    mv.SetTargetField(movePos);
                    mv.SetKilledBoardIndex(attackedFigBoardIndex);
                    mv.SetKilledFigureField(movePos);
                    mv.SetElPassantField(Board::InvalidElPassantField);

                    results.push_back(mv);
                }
            }

            attackingMoves ^= moveBoard;
        }
    }

    // TODO: test copying all old castlings
    void _processPlainKingMoves(std::vector<Move>& results, const uint64_t blockedFigMap, const uint64_t allyMap, const uint64_t enemyMap) const
    {
        static constexpr size_t CastlingPerColor = 2;

        // generating moves
        const uint64_t kingMoves = KingMap::GetMoves(board.GetKingMsbPos(board.movColor)) & ~blockedFigMap & ~allyMap;
        uint64_t attackingMoves = kingMoves & enemyMap;
        uint64_t nonAttackingMoves = kingMoves ^ attackingMoves;

        // preparing variables
        auto castlings = board.Castlings;
        castlings[CastlingPerColor * board.movColor + KingCastlingIndex] = false;
        castlings[CastlingPerColor * board.movColor + QueenCastlingIndex] = false;

        const int oldKingPos = ExtractMsbPos(board.boards[board.movColor + kingIndex]);

        // processing simple non attacking moves
        while (nonAttackingMoves)
        {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(nonAttackingMoves);

            Move mv{};
            mv.SetCasltingRights(castlings);
            mv.SetStartField(oldKingPos);
            mv.SetStartBoardIndex(board.movColor + kingIndex);
            mv.SetTargetBoardIndex(board.movColor + kingIndex);
            mv.SetTargetField(newPos);
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
            results.push_back(mv);

            nonAttackingMoves ^= (maxMsbPossible >> newPos);
        }

        // processing slightly more complicated attacking moves
        while (attackingMoves)
        {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(attackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // finding attacked figure
            const size_t attackedFigBoardIndex = mechanics.GetIndexOfContainingBoard(newKingBoard, SwapColor(board.movColor));

            Move mv{};
            mv.SetCasltingRights(castlings);
            mv.SetStartField(oldKingPos);
            mv.SetStartBoardIndex(board.movColor + kingIndex);
            mv.SetTargetBoardIndex(board.movColor + kingIndex);
            mv.SetTargetField(newPos);
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetKilledBoardIndex(attackedFigBoardIndex);
            mv.SetKilledFigureField(newPos);
            results.push_back(mv);

            attackingMoves ^= newKingBoard;
        }
    }

    // TODO: GIANT TODO WTF IS HALLOWEDTILES
    void _processKingMovesWhenChecked(std::vector<Move>& results, const uint64_t blockedFigMap, const uint64_t allyMap, const uint64_t enemyMap, [[maybe_unused]]const uint64_t hallowedTilesMaps) const
    {
        static constexpr size_t CastlingPerColor = 2;

        // generating moves
        const uint64_t kingMoves = KingMap::GetMoves(board.GetKingMsbPos(board.movColor)) & ~blockedFigMap & ~allyMap;
        uint64_t nonAttackingMoves = kingMoves & ~enemyMap;
        uint64_t attackingMoves = kingMoves ^ nonAttackingMoves;

        // preparing variables
        auto castlings = board.Castlings;
        castlings[CastlingPerColor * board.movColor + KingCastlingIndex] = false;
        castlings[CastlingPerColor * board.movColor + QueenCastlingIndex] = false;

        const int oldKingPos = ExtractMsbPos(board.boards[board.movColor + kingIndex]);

        // processing simple non attacking moves
        while (nonAttackingMoves)
        {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(nonAttackingMoves);

            Move mv{};
            mv.SetCasltingRights(castlings);
            mv.SetStartField(oldKingPos);
            mv.SetStartBoardIndex(board.movColor + kingIndex);
            mv.SetTargetBoardIndex(board.movColor + kingIndex);
            mv.SetTargetField(newPos);
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
            results.push_back(mv);

            nonAttackingMoves ^= maxMsbPossible >> newPos;
        }

        // processing slightly more complicated attacking moves
        while (attackingMoves)
        {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(attackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // finding attacked figure
            const size_t attackedFigBoardIndex = mechanics.GetIndexOfContainingBoard(newKingBoard, SwapColor(board.movColor));

            Move mv{};
            mv.SetCasltingRights(castlings);
            mv.SetStartField(oldKingPos);
            mv.SetStartBoardIndex(board.movColor + kingIndex);
            mv.SetTargetBoardIndex(board.movColor + kingIndex);
            mv.SetTargetField(newPos);
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetKilledBoardIndex(attackedFigBoardIndex);
            mv.SetKilledFigureField(newPos);
            results.push_back(mv);

            attackingMoves ^= newKingBoard;
        }
    }

    // TODO: simplify ifs??
    // TODO: cleanup left castling available when rook is dead then propagate no castling checking?
    void _processKingCastlings(std::vector<Move>& results, const uint64_t blockedFigMap, const uint64_t fullMap) const
    {
        for (size_t i = 0; i < Board::CastlingsPerColor; ++i)
            if (const size_t castlingIndex = board.movColor * Board::CastlingsPerColor + i;
                board.Castlings[castlingIndex]
                && (Board::CastlingsRookMaps[castlingIndex] & board.boards[board.movColor*Board::BoardsPerCol + rooksIndex]) != 0
                && (Board::CastlingSensitiveFields[castlingIndex] & blockedFigMap) == 0
                && (Board::CastlingTouchedFields[castlingIndex] & fullMap) == 0)
            {
                auto castlings = board.Castlings;
                castlings[board.movColor * Board::CastlingsPerColor + KingCastlingIndex] = false;
                castlings[board.movColor * Board::CastlingsPerColor + QueenCastlingIndex] = false;

                Move mv{};
                mv.SetCasltingRights(castlings);
                mv.SetStartField(ExtractMsbPos(Board::DefaultKingBoards[board.movColor]));
                mv.SetStartBoardIndex(board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetTargetBoardIndex(board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetTargetField(Board::CastlingNewKingPos[castlingIndex]);
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetKilledBoardIndex(board.movColor * Board::BoardsPerCol + rooksIndex);
                mv.SetKilledFigureField(Board::CastlingsRookMaps[castlingIndex]);
                mv.SetCastlingType(1 + castlingIndex);
                results.push_back(mv);
            }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    ChessMechanics mechanics;
    Board&board;
};

#endif //MOVEGENERATOR_H
