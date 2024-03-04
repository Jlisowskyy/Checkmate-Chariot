//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

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

    explicit MoveGenerator(Board&bd): board(bd)
    {
    }

    MoveGenerator(MoveGenerator&other) = delete;

    MoveGenerator& operator=(MoveGenerator&) = delete;

    ~MoveGenerator() = default;

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

    // returns [ pinnedFigMap, allowedTilesMap ]
    [[nodiscard]] std::pair<uint64_t, uint64_t> GetPinnedFigsMapWithCheck(int col, uint64_t fullMap) const;

    [[nodiscard]] uint64_t GetAllowedTilesWhenCheckedByNonSliding() const;

    std::vector<Board> GetMovesFast()
    {
        const uint64_t fullMap = GetFullMap();
        const auto [blockedFigMap, checksCount, checkType] = GetBlockedFieldMap(fullMap);

        std::vector<Board> results{};
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
    [[nodiscard]] uint64_t _generateAllowedTilesForPrecisedPinnedFig(uint64_t figBoard, uint64_t fullMap) const;

    void _noCheckGen(std::vector<Board>& results, const uint64_t fullMap, const uint64_t blockedFigMap)
    {
        const uint64_t pinnedFigsMap = GetPinnedFigsMapWoutCheck(board.movColor, fullMap);
        const uint64_t enemyMap = GetColMap(SwapColor(board.movColor));
        const uint64_t allyMap = GetColMap(board.movColor);

        _processFigMoves<RookMap, true>(results, board.boards[Board::BoardsPerCol * board.movColor + rooksIndex],
                                                 enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<BishopMap>(results, board.boards[Board::BoardsPerCol * board.movColor + bishopsIndex],
                                             enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<QueenMap>(results, board.boards[Board::BoardsPerCol * board.movColor + queensIndex], enemyMap,
                                            allyMap, pinnedFigsMap);

        _processFigMoves<KnightMap>(results, board.boards[Board::BoardsPerCol * board.movColor + knightsIndex],
                                             enemyMap, allyMap, pinnedFigsMap);

        if (board.movColor == WHITE)
            _processPawnMoves<WhitePawnMap>(results, board.boards[Board::BoardsPerCol * board.movColor + pawnsIndex],
                                                     enemyMap, allyMap, pinnedFigsMap);
        else
            _processPawnMoves<BlackPawnMap>(results, board.boards[Board::BoardsPerCol * board.movColor + pawnsIndex],
                                                     enemyMap, allyMap, pinnedFigsMap);

        _processPlainKingMoves(results, blockedFigMap, allyMap, enemyMap);

        _processKingCastlings(results, blockedFigMap, fullMap);
    }

    void _singleCheckGen(std::vector<Board>& results, const uint64_t fullMap, const uint64_t blockedFigMap, const uint8_t checkType)
    {
        static constexpr uint64_t UNUSED = 0;

        // simplifying figure search by distinguishing check types
        const auto [pinnedFigsMap, allowedTilesMap] = [&]() -> std::pair<uint64_t, uint64_t>
        {
            if (checkType == slidingFigCheck)
                return GetPinnedFigsMapWithCheck(board.movColor, fullMap);

            auto pinned = GetPinnedFigsMapWoutCheck(board.movColor, fullMap);
            return {pinned, GetAllowedTilesWhenCheckedByNonSliding()};
        }();

        // helping variable preparation
        const uint64_t enemyMap = GetColMap(SwapColor(board.movColor));
        const uint64_t allyMap = GetColMap(board.movColor);

        // Specific figure processing
        _processFigMoves<RookMap, true, false, false, true>(results, board.boards[Board::BoardsPerCol * board.movColor + rooksIndex],
                                                                     enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                                     allowedTilesMap);

        _processFigMoves<BishopMap, false, false, false, true>(results, board.boards[Board::BoardsPerCol * board.movColor +
                                                                            bishopsIndex],
                                                                        enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                                        allowedTilesMap);

        _processFigMoves<QueenMap, false, false, false, true>(results, board.boards[Board::BoardsPerCol * board.movColor +
                                                                           queensIndex],
                                                                       enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                                       allowedTilesMap);

        _processFigMoves<KnightMap, false, false, false, true>(results, board.boards[
                                                                            Board::BoardsPerCol * board.movColor +
                                                                            knightsIndex],
                                                                        enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                                        allowedTilesMap);

        if (board.movColor == WHITE)
            _processPawnMoves<WhitePawnMap, true>(results, board.boards[Board::BoardsPerCol * board.movColor + pawnsIndex],
                                                           enemyMap, allyMap, pinnedFigsMap, allowedTilesMap);
        else
            _processPawnMoves<BlackPawnMap, true>(results, board.boards[Board::BoardsPerCol * board.movColor + pawnsIndex],
                                                           enemyMap, allyMap, pinnedFigsMap, allowedTilesMap);


        _processKingMovesWhenChecked(results, blockedFigMap, allyMap, enemyMap, allowedTilesMap);
    }

    void _doubleCheckGen(std::vector<Board>& results, const uint64_t blockedFigMap)
    {
        const uint64_t allyMap = GetColMap(board.movColor);
        const uint64_t enemyMap = GetColMap(SwapColor(board.movColor));
        _processPlainKingMoves(results, blockedFigMap, allyMap, enemyMap);
    }

    template<
        class MapT,
        bool isCheck = false
    >
    void _processPawnMoves(std::vector<Board>& results, uint64_t&figMap,
                           const uint64_t enemyMap, const uint64_t allyMap, const uint64_t pinnedFigMap,
                           [[maybe_unused]] const uint64_t allowedMoveFillter = 0)
    {
        const uint64_t promotingPawns = figMap & MapT::PromotingMask;
        const uint64_t nonPromotingPawns = figMap ^ promotingPawns;

        _processFigMoves<MapT, false, false, true, isCheck, MapT::GetElPassantField>(results, figMap, enemyMap,
            allyMap, pinnedFigMap, nonPromotingPawns, allowedMoveFillter);

        if (promotingPawns)
            _processFigMoves<MapT, false, true, true, isCheck>(results, figMap, enemyMap,
                                                                    allyMap, pinnedFigMap, promotingPawns,
                                                                    allowedMoveFillter);

        _processElPassantMoves<MapT, isCheck>(results, allyMap | enemyMap, pinnedFigMap,
            figMap, allowedMoveFillter);
    }

    // TODO: Consider different soluition?
    template<
        class MapT,
        bool isCheck = false
    >
    void _processElPassantMoves(std::vector<Board>& results, const uint64_t fullMap, const uint64_t pinnedFigMap, uint64_t&figMap,
                                [[maybe_unused]] const uint64_t allowedMoveFillter = 0)
    {
        if (board.elPassantField == INVALID) return;

        // calculation preparation
        const uint64_t suspectedFields = MapT::GetElPassantSuspectedFields(board.elPassantField);
        const size_t enemyCord = SwapColor(board.movColor) * Board::BoardsPerCol;
        const uint64_t enemyRookFigs = board.boards[enemyCord + queensIndex] | board.boards[enemyCord + rooksIndex];
        uint64_t possiblePawnsToMove = figMap & suspectedFields;

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
                        _generateAllowedTilesForPrecisedPinnedFig(pawnMap, fullMap) & moveMap) == 0) {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }
                if ((_generateAllowedTilesForPrecisedPinnedFig(board.elPassantField, fullMap) & moveMap) == 0) {
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

            // applying changes on board
            const uint64_t oldElPassantField = board.elPassantField;
            figMap ^= pawnMap;
            figMap |= moveMap;
            board.boards[enemyCord + pawnsIndex] ^= board.elPassantField;
            board.elPassantField = INVALID;
            board.ChangePlayingColor();

            results.push_back(board);

            // reverting changes on board
            board.ChangePlayingColor();
            board.elPassantField = oldElPassantField;
            board.boards[enemyCord + pawnsIndex] |= oldElPassantField;
            figMap ^= moveMap;
            figMap |= pawnMap;

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
        Field (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr
    >
    void _processFigMoves(std::vector<Board>& results, uint64_t&figMap, const uint64_t enemyMap, const uint64_t allyMap, const uint64_t pinnedFigMap,
                          [[maybe_unused]] const uint64_t figureSelector = 0,
                          [[maybe_unused]] const uint64_t allowedMovesSelector = 0)
    {
        static constexpr size_t invalidCastlingIndex = 99;

        const uint64_t fullMap = enemyMap | allyMap;
        uint64_t pinnedOnes = pinnedFigMap & figMap;
        uint64_t unpinnedOnes = figMap ^ pinnedOnes;

        // applying filter if needed
        if constexpr (selectFigures)
        {
            pinnedOnes &= figureSelector;
            unpinnedOnes &= figureSelector;
        }

        // saving results of previous el passant field, used only when figure is not a pawn
        const uint64_t oldElpassant = board.elPassantField;
        board.elPassantField = INVALID;

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
            // TODO: test in future performance affections
            [[maybe_unused]] size_t castlingIndex = invalidCastlingIndex;
            if constexpr (checkForCastling)
            {
                for (size_t i = 0; i < Board::CastlingsPerColor; ++i)
                    if (const size_t index = board.movColor * Board::CastlingsPerColor + i;
                        board.Castlings[index] && (Board::CastlingsRookMaps[index] & figBoard) != 0)
                    {
                        board.Castlings[index] = false;
                        castlingIndex = index;
                    }
            }

            // preparing moves
            const uint64_t attackMoves = figMoves & enemyMap;
            const uint64_t nonAttackingMoves = figMoves ^ attackMoves;
            figMap ^= figBoard;

            // processing move consequneces
            _processNonAttackingMoves<promotePawns, elPassantFieldDeducer>(results, figMap, nonAttackingMoves, figBoard);
            _processAttackingMoves<promotePawns>(results, figMap, attackMoves);

            // cleaning up
            figMap |= figBoard;
            if constexpr (checkForCastling)
                if (castlingIndex != invalidCastlingIndex)
                    board.Castlings[castlingIndex] = true;

            unpinnedOnes ^= figBoard;
        }

        // if check is detected pinned figure stays in place
        if constexpr (isCheck)
        {
            // previous el passant state should be restored
            board.elPassantField = oldElpassant;
            return;
        }

        // procesing pinned moves
        // Note: corner Rook possibly applicable to castling cannot be pinned
        while (pinnedOnes)
        {
            // processing moves
            const uint8_t figPos = ExtractMsbPos(pinnedOnes);
            const uint64_t figBoard = maxMsbPossible >> figPos;
            const uint64_t allowedTiles = _generateAllowedTilesForPrecisedPinnedFig(figBoard, fullMap);
            const uint64_t figMoves = MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap & allowedTiles;
            // TODO: ischeck applid here?
            // TODO: breaking if there?

            // preparing moves
            const uint64_t attackMoves = figMoves & enemyMap;
            const uint64_t nonAttackingMoves = figMoves ^ attackMoves;
            figMap ^= figBoard;

            // processing move consequences
            _processNonAttackingMoves<promotePawns, elPassantFieldDeducer>(results, figMap, nonAttackingMoves, figBoard);
            // TODO: There is exactly one move possible
            _processAttackingMoves<promotePawns>(results, figMap, attackMoves);

            figMap |= figBoard;
            pinnedOnes ^= figBoard;
        }

        // previous el passant state should be restored
        board.elPassantField = oldElpassant;
    }

    // TODO: improve readability of code below
    template<
        bool promotePawns,
        Field (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr
    >
    void _processNonAttackingMoves(std::vector<Board>& results, uint64_t&figMap, uint64_t nonAttackingMoves,[[maybe_unused]] const uint64_t startPos = 0)  /* used only for pawns to check el passant*/
    {
        while (nonAttackingMoves)
        {
            // extracting moves
            const uint8_t movePos = ExtractMsbPos(nonAttackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;

            if constexpr (!promotePawns)
            // simple figure case
            {
                // if el passant line is passed when figure moved to these line flag will turned on
                if constexpr (elPassantFieldDeducer != nullptr)
                    board.elPassantField = elPassantFieldDeducer(moveBoard, startPos);

                // applying moves
                figMap |= moveBoard;

                // performing core actions
                board.ChangePlayingColor();
                results.push_back(board);
                board.ChangePlayingColor();

                // reverting flag changes
                if constexpr (elPassantFieldDeducer != nullptr)
                    board.elPassantField = INVALID;

                // cleaning up
                figMap ^= moveBoard;
            }
            if constexpr (promotePawns)
            // upgrading pawn case
            {
                const size_t colIndex = board.movColor * Board::BoardsPerCol;
                for (size_t i = knightsIndex; i < kingIndex; ++i)
                {
                    // applying moves
                    board.boards[colIndex + i] |= moveBoard;

                    // performing core actions
                    board.ChangePlayingColor();
                    results.push_back(board);
                    board.ChangePlayingColor();

                    // cleaning up
                    board.boards[colIndex + i] ^= moveBoard;
                }
            }

            nonAttackingMoves ^= moveBoard;
        }
    }

    template<
        bool promotePawns
    >
    void _processAttackingMoves(std::vector<Board>& results, uint64_t&figMap, uint64_t attackingMoves)
    {
        while (attackingMoves)
        {
            // extracting moves
            const uint8_t movePos = ExtractMsbPos(attackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;
            const size_t attackedFigBoardIndex = GetIndexOfContainingBoard(moveBoard, SwapColor(board.movColor));

            if constexpr (!promotePawns)
            // simple figure case
            {
                // applying moves
                figMap |= moveBoard;
                board.boards[attackedFigBoardIndex] ^= moveBoard;

                // performing core actions
                board.ChangePlayingColor();
                results.push_back(board);
                board.ChangePlayingColor();

                // cleaning
                figMap ^= moveBoard;
                board.boards[attackedFigBoardIndex] |= moveBoard;
            }
            if constexpr (promotePawns)
            // upgrading pawn case
            {
                const size_t colIndex = board.movColor * Board::BoardsPerCol;

                // removing attacked piece
                board.boards[attackedFigBoardIndex] ^= moveBoard;

                for (size_t i = knightsIndex; i < kingIndex; ++i)
                {
                    // applying moves
                    board.boards[colIndex + i] |= moveBoard;

                    // performing core actions
                    board.ChangePlayingColor();
                    results.push_back(board);
                    board.ChangePlayingColor();

                    // cleaning up
                    board.boards[colIndex + i] ^= moveBoard;
                }

                // replacing attacked fig
                board.boards[attackedFigBoardIndex] |= moveBoard;
            }

            attackingMoves ^= moveBoard;
        }
    }

    // TODO: test copying all old castlings
    void _processPlainKingMoves(std::vector<Board>& results, const uint64_t blockedFigMap, const uint64_t allyMap, const uint64_t enemyMap)
    {
        static constexpr size_t CastlingPerColor = 2;

        // simple helping variables
        const size_t movingColorIndex = board.movColor * Board::BoardsPerCol;

        // generating moves
        const uint64_t kingMoves = KingMap::GetMoves(board.GetKingMsbPos(board.movColor)) & ~blockedFigMap & ~
                                   allyMap;
        uint64_t attackingMoves = kingMoves & enemyMap;
        uint64_t nonAttackingMoves = kingMoves ^ attackingMoves;

        // saving old parameters
        const uint64_t oldKingBoard = board.boards[movingColorIndex + kingIndex];
        const auto oldCastlings = board.Castlings;
        const uint64_t oldElPassant = board.elPassantField;

        // prohibiting castlings
        board.Castlings[CastlingPerColor * board.movColor + KingCastlingIndex] = false;
        board.Castlings[CastlingPerColor * board.movColor + QueenCastlingIndex] = false;

        //prohibiting elPassant
        board.elPassantField = INVALID;

        // processing simple non attacking moves
        while (nonAttackingMoves)
        {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(nonAttackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // preparing board changes
            board.boards[movingColorIndex + kingIndex] = newKingBoard;

            // performing core actions
            board.ChangePlayingColor();
            results.push_back(board);
            board.ChangePlayingColor();

            nonAttackingMoves ^= newKingBoard;
        }

        // processing slightly more complicated attacking moves
        while (attackingMoves)
        {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(attackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // finding attacked figure
            const size_t attackedFigBoardIndex = GetIndexOfContainingBoard(newKingBoard, SwapColor(board.movColor));

            // preparing board changes
            board.boards[movingColorIndex + kingIndex] = newKingBoard;
            board.boards[attackedFigBoardIndex] ^= newKingBoard;
            board.ChangePlayingColor();

            // performing core actions
            results.push_back(board);

            // cleaning up
            board.ChangePlayingColor();
            board.boards[attackedFigBoardIndex] |= newKingBoard;

            attackingMoves ^= newKingBoard;
        }

        // reverting changes
        board.Castlings = oldCastlings;
        board.boards[board.movColor * Board::BoardsPerCol + kingIndex] = oldKingBoard;
        board.elPassantField = oldElPassant;
    }

    void _processKingMovesWhenChecked(std::vector<Board>& results, const uint64_t blockedFigMap, const uint64_t allyMap, const uint64_t enemyMap, const uint64_t hallowedTilesMaps)
    {
        static constexpr size_t CastlingPerColor = 2;

        // simple helping variables
        const size_t movingColorIndex = board.movColor * Board::BoardsPerCol;

        // generating moves
        const uint64_t kingMoves = KingMap::GetMoves(board.GetKingMsbPos(board.movColor)) & ~blockedFigMap & ~
                                   allyMap;
        uint64_t nonAttackingMoves = kingMoves & ~enemyMap;
        uint64_t attackingMoves = kingMoves ^ nonAttackingMoves;

        // saving old parameters
        const auto oldCastlings = board.Castlings;
        const uint64_t oldElPassant = board.elPassantField;
        const uint64_t oldKingBoard = board.boards[movingColorIndex + kingIndex];

        // prohibiting castlings
        board.Castlings[CastlingPerColor * board.movColor + KingCastlingIndex] = false;
        board.Castlings[CastlingPerColor * board.movColor + QueenCastlingIndex] = false;

        // prohibiting elPassant
        board.elPassantField = INVALID;

        // processing simple non attacking moves
        while (nonAttackingMoves)
        {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(nonAttackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // preparing board changes
            board.boards[movingColorIndex + kingIndex] = newKingBoard;

            // performing core actions
            board.ChangePlayingColor();
            results.push_back(board);
            board.ChangePlayingColor();

            nonAttackingMoves ^= newKingBoard;
        }

        // processing slightly more complicated attacking moves
        while (attackingMoves)
        {
            // extracting new king position data
            const uint8_t newPos = ExtractMsbPos(attackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // finding attacked figure
            const size_t attackedFigBoardIndex = GetIndexOfContainingBoard(newKingBoard, SwapColor(board.movColor));

            // preparing board changes
            board.boards[movingColorIndex + kingIndex] = newKingBoard;
            board.boards[attackedFigBoardIndex] ^= newKingBoard;
            board.ChangePlayingColor();

            // performing core actions
            results.push_back(board);

            // cleaning up
            board.ChangePlayingColor();
            board.boards[attackedFigBoardIndex] |= newKingBoard;

            attackingMoves ^= newKingBoard;
        }

        // reverting changes
        board.Castlings = oldCastlings;
        board.boards[board.movColor * Board::BoardsPerCol + kingIndex] = oldKingBoard;
        board.elPassantField = oldElPassant;
    }

    // TODO: simplify ifs??
    // TODO: cleanup left castling available when rook is dead then propagate no castling checking?
    void _processKingCastlings(std::vector<Board>& results, const uint64_t blockedFigMap, const uint64_t fullMap)
    {
        for (size_t i = 0; i < Board::CastlingsPerColor; ++i)
            if (const size_t castlingIndex = board.movColor * Board::CastlingsPerColor + i;
                board.Castlings[castlingIndex]
                && (Board::CastlingsRookMaps[castlingIndex] & board.boards[board.movColor*Board::BoardsPerCol + rooksIndex]) != 0
                && (Board::CastlingSensitiveFields[castlingIndex] & blockedFigMap) == 0
                && (Board::CastlingTouchedFields[castlingIndex] & fullMap) == 0)
            {
                // processing mvoe and performing cleanup
                const size_t otherCastlingIndex = board.movColor * Board::CastlingsPerColor + (1 - i);
                const bool otherCastling = board.Castlings[otherCastlingIndex]; // saving up the other castling state
                board.Castlings[castlingIndex] = false;
                board.Castlings[otherCastlingIndex] = false;

                board.boards[board.movColor * Board::BoardsPerCol + kingIndex] =
                        maxMsbPossible >> Board::CastlingNewKingPos[castlingIndex];
                board.boards[board.movColor * Board::BoardsPerCol + rooksIndex] ^= Board::CastlingsRookMaps[
                    castlingIndex];
                board.boards[board.movColor * Board::BoardsPerCol + rooksIndex] |= Board::CastlingNewRookMaps[
                    castlingIndex];
                const uint64_t oldElPassant = board.elPassantField;
                board.elPassantField = INVALID;

                // processiong main actions
                board.ChangePlayingColor();
                results.push_back(board);
                board.ChangePlayingColor();

                // cleaning up after last move
                board.Castlings[castlingIndex] = true;
                board.Castlings[otherCastlingIndex] = otherCastling;
                board.boards[board.movColor * Board::BoardsPerCol + kingIndex] =
                        Board::DefaultKingBoards[board.movColor];
                board.boards[board.movColor * Board::BoardsPerCol + rooksIndex] |= Board::CastlingsRookMaps[
                    castlingIndex];
                board.boards[board.movColor * Board::BoardsPerCol + rooksIndex] ^= Board::CastlingNewRookMaps[
                    castlingIndex];
                board.elPassantField = oldElPassant;
            }
    }


    template<
        class MoveGeneratorT
    >
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

    template<
        class MoveMapT
    >
    [[nodiscard]] uint64_t _getPinnedFigsWoutCheckGenerator(uint64_t suspectedFigs, const uint64_t fullMap,
        const size_t allyCord, const int allyKingShift) const
    {
        uint64_t pinnedFigMap{};

        while (suspectedFigs != 0)
        {
            const int msbPos = ExtractMsbPos(suspectedFigs);
            const uint64_t attackedFig = MoveMapT::GetMoves(msbPos, fullMap) & fullMap
                                         & MoveMapT::GetMoves(ConvertToReversedPos(allyKingShift), fullMap);

            const uint64_t mapWoutAttackedFig = fullMap ^ attackedFig;
            const uint64_t isKingAttacked = MoveMapT::GetMoves(msbPos, mapWoutAttackedFig) & mapWoutAttackedFig & board.
                                            boards[allyCord + kingIndex];

            const uint64_t pinnedFigFlag = (isKingAttacked >> allyKingShift) * attackedFig; // 0 or attackedFig
            pinnedFigMap |= pinnedFigFlag;

            suspectedFigs ^= (maxMsbPossible >> msbPos);
        }

        return pinnedFigMap;
    }

    // returns [ pinnedFigMap, allowedTilesMap ]
    template<
        class MoveMapT
    >
    [[nodiscard]] std::pair<uint64_t, uint64_t> _getPinnedFigsWithCheckGenerator(
        uint64_t suspectedFigs, const uint64_t fullMap, const size_t allyCord, const int allyKingShift) const
    {
        uint64_t pinnedFigMap{};
        uint64_t allowedTilesFigMap{};

        while (suspectedFigs != 0)
        {
            const int msbPos = ExtractMsbPos(suspectedFigs);
            const uint64_t suspectedFigAttackFields = MoveMapT::GetMoves(msbPos, fullMap);
            const uint64_t kingsPerspective = MoveMapT::GetMoves(ConvertToReversedPos(allyKingShift), fullMap);

            // TODO: ERROR HERE
            if (const uint64_t attackedFig = suspectedFigAttackFields & fullMap &
                                                                (kingsPerspective | board.boards[allyCord + kingIndex]);
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

    Board&board;
};

#endif //MOVEGENERATOR_H
