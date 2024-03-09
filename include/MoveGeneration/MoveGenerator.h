//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <array>

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "../ThreadManagement/stack.h"
#include "ChessMechanics.h"
#include "BishopMap.h"
#include "KingMap.h"
#include "KnightMap.h"
#include "WhitePawnMap.h"
#include "BlackPawnMap.h"
#include "QueenMap.h"
#include "RookMap.h"
#include "Move.h"
#include "../Evaluation/MoveSortEval.h"

struct MoveGenerator {
    using stck = stack<Move, DefaultStackSize>;
    using payload = stck::stackPayload;

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

    MoveGenerator() = delete;

    explicit MoveGenerator(Board& bd, stack<Move, DefaultStackSize>& s): _mechanics(bd), _threadStack(s), _board(bd) {
    }

    MoveGenerator(MoveGenerator& other) = delete;

    MoveGenerator& operator=(MoveGenerator&) = delete;

    ~MoveGenerator() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] bool IsCheck() const {
        return _mechanics.IsCheck();
    }

    template<
        bool GenOnlyAttackMoves = false
    >payload GetMovesFast() {
        const uint64_t fullMap = _mechanics.GetFullMap();
        const auto [blockedFigMap, checksCount, checkType] = _mechanics.GetBlockedFieldMap(fullMap);
        payload results = _threadStack.GetPayload();

        switch (checksCount) {
            case 0:
                _noCheckGen<GenOnlyAttackMoves>(results, fullMap, blockedFigMap);
                break;
            case 1:
                _singleCheckGen<GenOnlyAttackMoves>(results, fullMap, blockedFigMap, checkType);
                break;
            case 2:
                _doubleCheckGen<GenOnlyAttackMoves>(results, blockedFigMap);
                break;
        }

        return results;
    }

    uint64_t CountMoves(const int depth) {
        if (depth == 0) return 1;

        const auto moves = GetMovesFast();

        if (depth == 1) {
            _threadStack.PopAggregate(moves);
            return moves.size;
        }

        uint64_t sum{};

        const auto oldCastling = _board.Castlings;
        const auto oldElPassant = _board.elPassantField;

        for (size_t i = 0; i < moves.size; ++i) {
            Move::MakeMove(moves[i], _board);
            sum += CountMoves(depth - 1);
            Move::UnmakeMove(moves[i], _board, oldCastling, oldElPassant);
        }

        _threadStack.PopAggregate(moves);
        return sum;
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:
    template<
        bool GenOnlyAttackMoves
    >void _noCheckGen(payload& results, const uint64_t fullMap, const uint64_t blockedFigMap) {
        const uint64_t pinnedFigsMap = _mechanics.GetPinnedFigsMapWoutCheck(_board.movColor, fullMap);
        const uint64_t enemyMap = _mechanics.GetColMap(SwapColor(_board.movColor));
        const uint64_t allyMap = _mechanics.GetColMap(_board.movColor);

        _processFigMoves<GenOnlyAttackMoves, RookMap, true>(results, blockedFigMap, enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<GenOnlyAttackMoves, BishopMap>(results, blockedFigMap, enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<GenOnlyAttackMoves, QueenMap>(results, blockedFigMap, enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<GenOnlyAttackMoves, KnightMap>(results, blockedFigMap, enemyMap, allyMap, pinnedFigsMap);

        if (_board.movColor == WHITE)
            _processPawnMoves<GenOnlyAttackMoves, WhitePawnMap>(results, blockedFigMap,
                                            enemyMap, allyMap, pinnedFigsMap);
        else
            _processPawnMoves<GenOnlyAttackMoves, BlackPawnMap>(results, blockedFigMap,
                                            enemyMap, allyMap, pinnedFigsMap);

        _processPlainKingMoves<GenOnlyAttackMoves>(results, blockedFigMap, allyMap, enemyMap);

        if constexpr (GenOnlyAttackMoves == false)
            _processKingCastlings(results, blockedFigMap, fullMap);
    }

    template<
        bool GenOnlyAttackMoves
    >void _singleCheckGen(payload& results, const uint64_t fullMap, const uint64_t blockedFigMap,
                         const int checkType) {
        static constexpr uint64_t UNUSED = 0;

        // simplifying figure search by distinguishing check types
        const auto [pinnedFigsMap, allowedTilesMap] = [&]() -> std::pair<uint64_t, uint64_t> {
            if (checkType == slidingFigCheck)
                return _mechanics.GetPinnedFigsMapWithCheck(_board.movColor, fullMap);

            auto pinned = _mechanics.GetPinnedFigsMapWoutCheck(_board.movColor, fullMap);
            return {pinned, _mechanics.GetAllowedTilesWhenCheckedByNonSliding()};
        }();

        // helping variable preparation
        const uint64_t enemyMap = _mechanics.GetColMap(SwapColor(_board.movColor));
        const uint64_t allyMap = _mechanics.GetColMap(_board.movColor);

        // Specific figure processing
        _processFigMoves<GenOnlyAttackMoves, RookMap, true, false, false, true>(results, blockedFigMap,
                                                            enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                            allowedTilesMap);

        _processFigMoves<GenOnlyAttackMoves, BishopMap, false, false, false, true>(results, blockedFigMap,
                                                               enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                               allowedTilesMap);

        _processFigMoves<GenOnlyAttackMoves, QueenMap, false, false, false, true>(results, blockedFigMap,
                                                              enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                              allowedTilesMap);

        _processFigMoves<GenOnlyAttackMoves, KnightMap, false, false, false, true>(results, blockedFigMap,
                                                               enemyMap, allyMap, pinnedFigsMap, UNUSED,
                                                               allowedTilesMap);

        if (_board.movColor == WHITE)
            _processPawnMoves<GenOnlyAttackMoves, WhitePawnMap, true>(results, blockedFigMap,
                                                  enemyMap, allyMap, pinnedFigsMap, allowedTilesMap);
        else
            _processPawnMoves<GenOnlyAttackMoves, BlackPawnMap, true>(results, blockedFigMap,
                                                  enemyMap, allyMap, pinnedFigsMap, allowedTilesMap);

        _processPlainKingMoves<GenOnlyAttackMoves>(results, blockedFigMap, allyMap, enemyMap);
    }

    template<
        bool GenOnlyAttackMoves
    >void _doubleCheckGen(payload& results, const uint64_t blockedFigMap) const {
        const uint64_t allyMap = _mechanics.GetColMap(_board.movColor);
        const uint64_t enemyMap = _mechanics.GetColMap(SwapColor(_board.movColor));
        _processPlainKingMoves<GenOnlyAttackMoves>(results, blockedFigMap, allyMap, enemyMap);
    }

    template<
        bool GenOnlyAttackMoves,
        class MapT,
        bool isCheck = false
    >
    void _processPawnMoves(payload& results, const uint64_t blockedFigMap, const uint64_t enemyMap, const uint64_t allyMap,
                           const uint64_t pinnedFigMap,
                           [[maybe_unused]] const uint64_t allowedMoveFillter = 0)
    {
        const uint64_t promotingPawns = _board.boards[MapT::GetBoardIndex(0)] & MapT::PromotingMask;
        const uint64_t nonPromotingPawns = _board.boards[MapT::GetBoardIndex(0)] ^ promotingPawns;

        _processFigMoves<GenOnlyAttackMoves, MapT, false, false, true, isCheck, MapT::GetElPassantField>(results, blockedFigMap, enemyMap,
            allyMap, pinnedFigMap, nonPromotingPawns, allowedMoveFillter);

        if (promotingPawns)
            _processFigMoves<GenOnlyAttackMoves, MapT, false, true, true, isCheck>(results, blockedFigMap, enemyMap,
                                                               allyMap, pinnedFigMap, promotingPawns,
                                                               allowedMoveFillter);

        _processElPassantMoves<MapT, isCheck>(results, blockedFigMap, allyMap | enemyMap, pinnedFigMap,
                                              allowedMoveFillter);
    }

    // TODO: Consider different soluition?
    template<
        class MapT,
        bool isCheck = false
    >
    void _processElPassantMoves(payload& results, const uint64_t blockedFigMap, const uint64_t fullMap, const uint64_t pinnedFigMap,
                                [[maybe_unused]] const uint64_t allowedMoveFillter = 0) {
        if (_board.elPassantField == Board::InvalidElPassantBoard) return;

        // calculation preparation
        const uint64_t suspectedFields = MapT::GetElPassantSuspectedFields(_board.elPassantField);
        const size_t enemyCord = SwapColor(_board.movColor) * Board::BoardsPerCol;
        const uint64_t enemyRookFigs = _board.boards[enemyCord + queensIndex] | _board.boards[enemyCord + rooksIndex];
        uint64_t possiblePawnsToMove = _board.boards[MapT::GetBoardIndex(0)] & suspectedFields;

        while (possiblePawnsToMove) {
            const uint64_t pawnMap = maxMsbPossible >> ExtractMsbPos(possiblePawnsToMove);

            // checking wheteher move would affect horizontal line attacks on king
            const uint64_t processedPawns = pawnMap | _board.elPassantField;
            const uint64_t cleanedFromPawnsMap = fullMap ^ processedPawns;
            if (const uint64_t kingHorizontalLine =
                        RookMap::GetMoves(_board.GetKingMsbPos(_board.movColor), cleanedFromPawnsMap) &
                        MapT::EnemyElPassantMask;
                (kingHorizontalLine & enemyRookFigs) != 0)
                return;

            const uint64_t moveMap = MapT::GetElPassantMoveField(_board.elPassantField);

            // checking wheter moving some pawns would undercover king on some line -
            if ((processedPawns & pinnedFigMap) != 0) {
                // two separate situations thats need to be considered, every pawn that participate in el passant move
                // should be unblocked on specific lines

                if ((pawnMap & pinnedFigMap) != 0 && (
                        _mechanics.GenerateAllowedTilesForPrecisedPinnedFig(pawnMap, fullMap) & moveMap) == 0) {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }
                if ((_mechanics.GenerateAllowedTilesForPrecisedPinnedFig(_board.elPassantField, fullMap) & moveMap) ==
                    0) {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }
            }

            // When king is checked only if move is going to allowed tile el passant is correct
            if constexpr (isCheck)
                if ((moveMap & allowedMoveFillter) == 0 && (_board.elPassantField & allowedMoveFillter) == 0) {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }

            // preparing and sending move
            Move mv{};
            mv.SetEval(MoveSortEval::ApplyAttackFieldEffects(MoveSortEval::DefaultValue, blockedFigMap, pawnMap, moveMap));
            mv.SetStartField(ExtractMsbPos(pawnMap));
            mv.SetStartBoardIndex(MapT::GetBoardIndex(0));
            mv.SetTargetField(ExtractMsbPos(moveMap));
            mv.SetTargetBoardIndex(MapT::GetBoardIndex(0));
            mv.SetKilledBoardIndex(MapT::GetEnemyPawnBoardIndex());
            mv.SetKilledFigureField(ExtractMsbPos(_board.elPassantField));
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetCasltingRights(_board.Castlings);

            results.Push(_threadStack, mv);

            possiblePawnsToMove ^= pawnMap;
        }
    }

    // TODO: Compare with simple if searching loop
    // TODO: propagate checkForCastling?
    template<
        bool GenOnlyAttackMoves,
        class MapT,
        bool checkForCastling = false,
        bool promotePawns = false,
        bool selectFigures = false,
        bool isCheck = false,
        uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr
    >
    void _processFigMoves(payload& results, const uint64_t blockedFigMap, const uint64_t enemyMap, const uint64_t allyMap,
                          const uint64_t pinnedFigMap,
                          [[maybe_unused]] const uint64_t figureSelector = 0,
                          [[maybe_unused]] const uint64_t allowedMovesSelector = 0) {
        const uint64_t fullMap = enemyMap | allyMap;
        uint64_t pinnedOnes = pinnedFigMap & _board.boards[MapT::GetBoardIndex(_board.movColor)];
        uint64_t unpinnedOnes = _board.boards[MapT::GetBoardIndex(_board.movColor)] ^ pinnedOnes;

        // applying filter if needed
        if constexpr (selectFigures) {
            pinnedOnes &= figureSelector;
            unpinnedOnes &= figureSelector;
        }

        // saving results of previous el passant field, used only when figure is not a pawn


        // procesing unpinned moves
        while (unpinnedOnes) {
            // processing moves
            const int figPos = ExtractMsbPos(unpinnedOnes);
            const uint64_t figBoard = maxMsbPossible >> figPos;

            // selecting allowed moves if in check
            const uint64_t figMoves = [&]() constexpr {
                if constexpr (!isCheck)
                    return MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap;
                if constexpr (isCheck)
                    return MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap & allowedMovesSelector;
            }();

            // Performing checks for castlings
            std::array<bool, Board::CastlingCount + 1> updatedCastlings = _board.Castlings;
            if constexpr (checkForCastling)
                updatedCastlings[RookMap::GetMatchingCastlingIndex(_board, figBoard)] = false;

            // preparing moves
            const uint64_t attackMoves = figMoves & enemyMap;
            [[maybe_unused]]const uint64_t nonAttackingMoves = figMoves ^ attackMoves;

            // processing move consequneces

            if constexpr (GenOnlyAttackMoves == false)
                _processNonAttackingMoves<promotePawns, elPassantFieldDeducer>(
                    results,
                    blockedFigMap,
                    nonAttackingMoves,
                    MapT::GetBoardIndex(_board.movColor),
                    figBoard,
                    updatedCastlings
                );

            _processAttackingMoves<promotePawns>(
                results,
                blockedFigMap,
                attackMoves,
                MapT::GetBoardIndex(_board.movColor),
                figBoard,
                updatedCastlings
            );

            unpinnedOnes ^= figBoard;
        }

        // if check is detected pinned figure stays in place
        if constexpr (isCheck)
            return;

        // procesing pinned moves
        // Note: corner Rook possibly applicable to castling cannot be pinned
        while (pinnedOnes) {
            // processing moves
            const int figPos = ExtractMsbPos(pinnedOnes);
            const uint64_t figBoard = maxMsbPossible >> figPos;
            const uint64_t allowedTiles = _mechanics.GenerateAllowedTilesForPrecisedPinnedFig(figBoard, fullMap);
            const uint64_t figMoves = MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap & allowedTiles;
            // TODO: ischeck applid here?
            // TODO: breaking if there?

            // preparing moves
            const uint64_t attackMoves = figMoves & enemyMap;
            [[maybe_unused]] const uint64_t nonAttackingMoves = figMoves ^ attackMoves;

            // processing move consequences

            if constexpr (GenOnlyAttackMoves == false)
                _processNonAttackingMoves<promotePawns, elPassantFieldDeducer>(
                    results,
                    blockedFigMap,
                    nonAttackingMoves,
                    MapT::GetBoardIndex(_board.movColor),
                    figBoard,
                    _board.Castlings
                );

            // TODO: There is exactly one move possible
            _processAttackingMoves<promotePawns>(
                results,
                blockedFigMap,
                attackMoves,
                MapT::GetBoardIndex(_board.movColor),
                figBoard,
                _board.Castlings
            );

            pinnedOnes ^= figBoard;
        }
    }

    // TODO: improve readability of code below
    template<
        bool promotePawns,
        uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr
    >
    void _processNonAttackingMoves(payload& results, const uint64_t blockedFigMap, uint64_t nonAttackingMoves, const size_t figBoardIndex,
                                   const uint64_t startField,
                                   const std::array<bool, Board::CastlingCount + 1>& castlings
    ) const {
        while (nonAttackingMoves) {
            // extracting moves
            const int movePos = ExtractMsbPos(nonAttackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;

            if constexpr (!promotePawns)
            // simple figure case
            {
                Move mv{};
                mv.SetEval(MoveSortEval::ApplyAttackFieldEffects(MoveSortEval::DefaultValue, blockedFigMap, startField, moveBoard));
                mv.SetStartField(ExtractMsbPos(startField));
                mv.SetStartBoardIndex(figBoardIndex);
                mv.SetTargetField(movePos);
                mv.SetTargetBoardIndex(figBoardIndex);
                mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
                // if el passant line is passed when figure moved to these line flag will turned on
                if constexpr (elPassantFieldDeducer != nullptr) {
                    // TODO: CHANGED TEMPORALRALKSFLAJSF TEMP
                    if (const auto result = elPassantFieldDeducer(moveBoard, startField); result == 0)
                        mv.SetElPassantField(Board::InvalidElPassantField);
                    else mv.SetElPassantField(ExtractMsbPos(result));
                } else
                    mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(castlings);

                results.Push(_threadStack, mv);
            }
            if constexpr (promotePawns)
            // upgrading pawn case
            {
                // iterating through upgradable pieces
                for (size_t i = knightsIndex; i < kingIndex; ++i) {
                    const size_t targetBoard = _board.movColor * Board::BoardsPerCol + i;

                    Move mv{};
                    uint16_t eval = MoveSortEval::ApplyAttackFieldEffects(MoveSortEval::DefaultValue, blockedFigMap, startField, moveBoard);
                    eval = MoveSortEval::ApplyPromotionEffects(eval, targetBoard);

                    mv.SetEval(eval);
                    mv.SetStartField(ExtractMsbPos(startField));
                    mv.SetStartBoardIndex(figBoardIndex);
                    mv.SetTargetField(movePos);
                    mv.SetTargetBoardIndex(targetBoard);
                    mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
                    mv.SetElPassantField(Board::InvalidElPassantField);
                    mv.SetCasltingRights(castlings);

                    results.Push(_threadStack, mv);
                }
            }

            nonAttackingMoves ^= moveBoard;
        }
    }

    template<
        bool promotePawns
    >
    void _processAttackingMoves(payload& results, const uint64_t blockedFigMap, uint64_t attackingMoves, const size_t figBoardIndex,
                                const uint64_t startField,
                                const std::array<bool, Board::CastlingCount + 1>& castlings) const {
        while (attackingMoves) {
            // extracting moves
            const int movePos = ExtractMsbPos(attackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;
            const size_t attackedFigBoardIndex = _mechanics.GetIndexOfContainingBoard(
                moveBoard, SwapColor(_board.movColor));

            if constexpr (!promotePawns)
            // simple figure case
            {
                Move mv{};
                uint16_t eval = MoveSortEval::ApplyAttackFieldEffects(MoveSortEval::DefaultValue, blockedFigMap, startField, moveBoard);
                eval = MoveSortEval::ApplyKilledFigEffect(eval, figBoardIndex, attackedFigBoardIndex);

                mv.SetEval(eval);
                mv.SetStartField(ExtractMsbPos(startField));
                mv.SetStartBoardIndex(figBoardIndex);
                mv.SetTargetField(movePos);
                mv.SetTargetBoardIndex(figBoardIndex);
                mv.SetKilledBoardIndex(attackedFigBoardIndex);
                mv.SetKilledFigureField(movePos);
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(castlings);

                results.Push(_threadStack, mv);
            }
            if constexpr (promotePawns)
            // upgrading pawn case
            {
                // iterating through upgradable pieces
                for (size_t i = knightsIndex; i < kingIndex; ++i) {
                    const size_t targetBoard = _board.movColor * Board::BoardsPerCol + i;

                    Move mv{};

                    uint16_t eval = MoveSortEval::ApplyAttackFieldEffects(MoveSortEval::DefaultValue, blockedFigMap, startField, moveBoard);
                    eval = MoveSortEval::ApplyKilledFigEffect(eval, figBoardIndex, attackedFigBoardIndex);
                    eval = MoveSortEval::ApplyPromotionEffects(eval, targetBoard);

                    mv.SetEval(eval);
                    mv.SetStartField(ExtractMsbPos(startField));
                    mv.SetStartBoardIndex(figBoardIndex);
                    mv.SetTargetField(movePos);
                    mv.SetTargetBoardIndex(targetBoard);
                    mv.SetKilledBoardIndex(attackedFigBoardIndex);
                    mv.SetKilledFigureField(movePos);
                    mv.SetElPassantField(Board::InvalidElPassantField);
                    mv.SetCasltingRights(castlings);

                    results.Push(_threadStack, mv);
                }
            }

            attackingMoves ^= moveBoard;
        }
    }

    // TODO: test copying all old castlings
    template<
        bool GenOnlyAttackMoves
    >void _processPlainKingMoves(payload& results, const uint64_t blockedFigMap, const uint64_t allyMap,
                                const uint64_t enemyMap) const {
        static constexpr size_t CastlingPerColor = 2;

        // generating moves
        const uint64_t kingMoves = KingMap::GetMoves(_board.GetKingMsbPos(_board.movColor)) & ~blockedFigMap & ~allyMap;
        uint64_t attackingMoves = kingMoves & enemyMap;
        [[maybe_unused]]uint64_t nonAttackingMoves = kingMoves ^ attackingMoves;

        // preparing variables
        auto castlings = _board.Castlings;
        castlings[CastlingPerColor * _board.movColor + KingCastlingIndex] = false;
        castlings[CastlingPerColor * _board.movColor + QueenCastlingIndex] = false;

        const int oldKingPos = ExtractMsbPos(_board.boards[_board.movColor * Board::BoardsPerCol  + kingIndex]);

        // processing simple non attacking moves
        if constexpr (GenOnlyAttackMoves == false)
            while (nonAttackingMoves) {
                // extracting new king position data
                const int newPos = ExtractMsbPos(nonAttackingMoves);

                Move mv{};
                mv.SetEval(MoveSortEval::DefaultValue);
                mv.SetStartField(oldKingPos);
                mv.SetStartBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetTargetField(newPos);
                mv.SetTargetBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(castlings);
                results.Push(_threadStack, mv);

                nonAttackingMoves ^= (maxMsbPossible >> newPos);
            }

        // processing slightly more complicated attacking moves
        while (attackingMoves) {
            // extracting new king position data
            const int newPos = ExtractMsbPos(attackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // finding attacked figure
            const size_t attackedFigBoardIndex = _mechanics.GetIndexOfContainingBoard(
                newKingBoard, SwapColor(_board.movColor));

            Move mv{};
            mv.SetEval(MoveSortEval::DefaultValue);
            mv.SetStartField(oldKingPos);
            mv.SetStartBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
            mv.SetTargetField(newPos);
            mv.SetTargetBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
            mv.SetKilledBoardIndex(attackedFigBoardIndex);
            mv.SetKilledFigureField(newPos);
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetCasltingRights(castlings);
            results.Push(_threadStack, mv);

            attackingMoves ^= newKingBoard;
        }
    }

    // TODO: simplify ifs??
    // TODO: cleanup left castling available when rook is dead then propagate no castling checking?
    void _processKingCastlings(payload& results, const uint64_t blockedFigMap, const uint64_t fullMap) const {
        for (size_t i = 0; i < Board::CastlingsPerColor; ++i)
            if (const size_t castlingIndex = _board.movColor * Board::CastlingsPerColor + i;
                _board.Castlings[castlingIndex]
                && (Board::CastlingsRookMaps[castlingIndex] & _board.boards[
                        _board.movColor * Board::BoardsPerCol + rooksIndex]) != 0
                && (Board::CastlingSensitiveFields[castlingIndex] & blockedFigMap) == 0
                && (Board::CastlingTouchedFields[castlingIndex] & fullMap) == 0) {
                auto castlings = _board.Castlings;
                castlings[_board.movColor * Board::CastlingsPerColor + KingCastlingIndex] = false;
                castlings[_board.movColor * Board::CastlingsPerColor + QueenCastlingIndex] = false;

                Move mv{};
                mv.SetEval(MoveSortEval::DefaultValue);
                mv.SetStartField(ExtractMsbPos(Board::DefaultKingBoards[_board.movColor]));
                mv.SetStartBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetTargetField(Board::CastlingNewKingPos[castlingIndex]);
                mv.SetTargetBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetKilledBoardIndex(_board.movColor * Board::BoardsPerCol + rooksIndex);
                mv.SetKilledFigureField(ExtractMsbPos(Board::CastlingsRookMaps[castlingIndex]));
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(castlings);
                mv.SetCastlingType(1 + castlingIndex);
                results.Push(_threadStack, mv);
            }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    ChessMechanics _mechanics;
    stack<Move, DefaultStackSize>& _threadStack;
    Board& _board;
};

#endif //MOVEGENERATOR_H
