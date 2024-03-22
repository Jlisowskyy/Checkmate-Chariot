//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "../Evaluation/MoveSortEval.h"
#include "../Evaluation/KillerTable.h"
#include "../ThreadManagement/stack.h"
#include "BishopMap.h"
#include "BlackPawnMap.h"
#include "ChessMechanics.h"
#include "KingMap.h"
#include "KnightMap.h"
#include "Move.h"
#include "QueenMap.h"
#include "RookMap.h"
#include "WhitePawnMap.h"

#include <array>
#include <exception>
#include <map>

struct  MoveGenerator
{
    using stck = stack<Move, DefaultStackSize>;
    using payload = stck::stackPayload;

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

    MoveGenerator() = delete;

    explicit MoveGenerator(
        Board& bd,
        stack<Move, DefaultStackSize>& s,
        const HistoricTable& ht = {},
        const KillerTable& kt = {},
        const PackedMove counterMove = {},
        const int depthLeft = 0,
        const int mostRecentMovedSquare = 0
    ) :
        _mechanics(bd),
        _threadStack(s),
        _board(bd),
        _counterMove(counterMove),
        _kTable(kt),
        _hTable(ht),
        _depthLeft(depthLeft),
        _mostRecentSq(mostRecentMovedSquare){}

    MoveGenerator(MoveGenerator& other) = delete;

    MoveGenerator& operator=(MoveGenerator&) = delete;

    ~MoveGenerator() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] bool IsCheck() const { return _mechanics.IsCheck(); }

    template <bool GenOnlyAttackMoves = false, bool ApplyHeuristicEval = true>
    payload GetMovesFast()
    {
        const uint64_t fullMap = _mechanics.GetFullMap();
        const auto [blockedFigMap, checksCount, checkType] = _mechanics.GetBlockedFieldMap(fullMap);
        payload results = _threadStack.GetPayload();

        switch (checksCount)
        {
            case 0:
                _noCheckGen<GenOnlyAttackMoves, ApplyHeuristicEval>(results, fullMap, blockedFigMap);
                break;
            case 1:
                _singleCheckGen<GenOnlyAttackMoves, ApplyHeuristicEval>(results, fullMap, blockedFigMap, checkType);
                break;
            case 2:
                _doubleCheckGen<GenOnlyAttackMoves, ApplyHeuristicEval>(results, blockedFigMap);
                break;
#ifndef NDEBUG
            default:
                throw std::runtime_error("[ ERROR ] GetMovesFast didnt detect correct checks count!");
#endif
        }

        return results;
    }

    std::map<std::string, uint64_t> GetCountedMoves(int depth);
    uint64_t CountMoves(int depth);

    // ------------------------------
    // private methods
    // ------------------------------
   private:
    template <bool GenOnlyAttackMoves, bool ApplyHeuristicEval>
    void _noCheckGen(payload& results, const uint64_t fullMap, const uint64_t blockedFigMap)
    {
        const uint64_t pinnedFigsMap = _mechanics.GetPinnedFigsMapWoutCheck(_board.movColor, fullMap);
        const uint64_t enemyMap = _mechanics.GetColMap(SwapColor(_board.movColor));
        const uint64_t allyMap = _mechanics.GetColMap(_board.movColor);

        const uint64_t pawnAttacks =
            _board.movColor == WHITE
                ? BlackPawnMap::GetAttackFields(
                      _board.boards[Board::BoardsPerCol * SwapColor(_board.movColor) + pawnsIndex])
                : WhitePawnMap::GetAttackFields(
                      _board.boards[Board::BoardsPerCol * SwapColor(_board.movColor) + pawnsIndex]);

        _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, KnightMap>(results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, BishopMap>(results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, RookMap, true>(results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap);

        _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, QueenMap>(results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap);

        if (_board.movColor == WHITE)
            _processPawnMoves<GenOnlyAttackMoves, ApplyHeuristicEval, WhitePawnMap>(results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap);
        else
            _processPawnMoves<GenOnlyAttackMoves, ApplyHeuristicEval, BlackPawnMap>(results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap);

        _processPlainKingMoves<GenOnlyAttackMoves, ApplyHeuristicEval>(results, blockedFigMap, allyMap, enemyMap);

        if constexpr (GenOnlyAttackMoves == false)
            _processKingCastlings<ApplyHeuristicEval>(results, blockedFigMap, fullMap);
    }

    template <bool GenOnlyAttackMoves, bool ApplyHeuristicEval>
    void _singleCheckGen(payload& results, const uint64_t fullMap, const uint64_t blockedFigMap, const int checkType)
    {
        static constexpr uint64_t UNUSED = 0;

        // simplifying figure search by distinguishing check types
        const auto [pinnedFigsMap, allowedTilesMap] = [&]() -> std::pair<uint64_t, uint64_t>
        {
            if (checkType == slidingFigCheck)
                return _mechanics.GetPinnedFigsMapWithCheck(_board.movColor, fullMap);

            auto pinned = _mechanics.GetPinnedFigsMapWoutCheck(_board.movColor, fullMap);
            return {pinned, _mechanics.GetAllowedTilesWhenCheckedByNonSliding()};
        }();

        // helping variable preparation
        const uint64_t enemyMap = _mechanics.GetColMap(SwapColor(_board.movColor));
        const uint64_t allyMap = _mechanics.GetColMap(_board.movColor);

        const uint64_t pawnAttacks =
            _board.movColor == WHITE
                ? BlackPawnMap::GetAttackFields(
                      _board.boards[Board::BoardsPerCol * SwapColor(_board.movColor) + pawnsIndex])
                : WhitePawnMap::GetAttackFields(
                      _board.boards[Board::BoardsPerCol * SwapColor(_board.movColor) + pawnsIndex]);

        // Specific figure processing
        _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, KnightMap, false, false, false, true>(
            results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap, UNUSED, allowedTilesMap);

        _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, BishopMap, false, false, false, true>(
            results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap, UNUSED, allowedTilesMap);

        _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, RookMap, true, false, false, true>(results, pawnAttacks, enemyMap, allyMap,
                                                                                pinnedFigsMap, UNUSED, allowedTilesMap);

        _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, QueenMap, false, false, false, true>(
            results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap, UNUSED, allowedTilesMap);

        if (_board.movColor == WHITE)
            _processPawnMoves<GenOnlyAttackMoves, ApplyHeuristicEval, WhitePawnMap, true>(results, pawnAttacks, enemyMap, allyMap,
                                                                      pinnedFigsMap, allowedTilesMap);
        else
            _processPawnMoves<GenOnlyAttackMoves, ApplyHeuristicEval, BlackPawnMap, true>(results, pawnAttacks, enemyMap, allyMap,
                                                                      pinnedFigsMap, allowedTilesMap);

        _processPlainKingMoves<GenOnlyAttackMoves, ApplyHeuristicEval>(results, blockedFigMap, allyMap, enemyMap);
    }

    template <bool GenOnlyAttackMoves, bool ApplyHeuristicEval>
    void _doubleCheckGen(payload& results, const uint64_t blockedFigMap) const
    {
        const uint64_t allyMap = _mechanics.GetColMap(_board.movColor);
        const uint64_t enemyMap = _mechanics.GetColMap(SwapColor(_board.movColor));
        _processPlainKingMoves<GenOnlyAttackMoves, ApplyHeuristicEval>(results, blockedFigMap, allyMap, enemyMap);
    }

    template <bool GenOnlyAttackMoves, bool ApplyHeuristicEval, class MapT, bool isCheck = false>
    void _processPawnMoves(payload& results, const uint64_t pawnAttacks, const uint64_t enemyMap,
                           const uint64_t allyMap, const uint64_t pinnedFigMap,
                           [[maybe_unused]] const uint64_t allowedMoveFillter = 0)
    {
        const uint64_t promotingPawns = _board.boards[MapT::GetBoardIndex(0)] & MapT::PromotingMask;
        const uint64_t nonPromotingPawns = _board.boards[MapT::GetBoardIndex(0)] ^ promotingPawns;

        _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, MapT, false, false, true, isCheck, MapT::GetElPassantField>(
            results, pawnAttacks, enemyMap, allyMap, pinnedFigMap, nonPromotingPawns, allowedMoveFillter);

        if (promotingPawns)
            _processFigMoves<GenOnlyAttackMoves, ApplyHeuristicEval, MapT, false, true, true, isCheck>(
                results, pawnAttacks, enemyMap, allyMap, pinnedFigMap, promotingPawns, allowedMoveFillter);

        _processElPassantMoves<ApplyHeuristicEval, MapT, isCheck>(results, allyMap | enemyMap, pinnedFigMap, allowedMoveFillter);
    }

    // TODO: Consider different soluition?
    template <bool ApplyHeuristicEval, class MapT, bool isCheck = false>
    void _processElPassantMoves(payload& results, const uint64_t fullMap, const uint64_t pinnedFigMap,
                                [[maybe_unused]] const uint64_t allowedMoveFillter = 0)
    {
        if (_board.elPassantField == Board::InvalidElPassantBoard)
            return;

        // calculation preparation
        const uint64_t suspectedFields = MapT::GetElPassantSuspectedFields(_board.elPassantField);
        const size_t enemyCord = SwapColor(_board.movColor) * Board::BoardsPerCol;
        const uint64_t enemyRookFigs = _board.boards[enemyCord + queensIndex] | _board.boards[enemyCord + rooksIndex];
        uint64_t possiblePawnsToMove = _board.boards[MapT::GetBoardIndex(0)] & suspectedFields;

        while (possiblePawnsToMove)
        {
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
            if ((processedPawns & pinnedFigMap) != 0)
            {
                // two separate situations thats need to be considered, every pawn that participate in el passant move
                // should be unblocked on specific lines

                if ((pawnMap & pinnedFigMap) != 0 &&
                    (_mechanics.GenerateAllowedTilesForPrecisedPinnedFig(pawnMap, fullMap) & moveMap) == 0)
                {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }
                if ((_mechanics.GenerateAllowedTilesForPrecisedPinnedFig(_board.elPassantField, fullMap) & moveMap) ==
                    0)
                {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }
            }

            // When king is checked only if move is going to allowed tile el passant is correct
            if constexpr (isCheck)
                if ((moveMap & allowedMoveFillter) == 0 && (_board.elPassantField & allowedMoveFillter) == 0)
                {
                    possiblePawnsToMove ^= pawnMap;
                    continue;
                }

            // preparing basic move information
            Move mv{};
            mv.SetStartField(ExtractMsbPos(pawnMap));
            mv.SetStartBoardIndex(MapT::GetBoardIndex(0));
            mv.SetTargetField(ExtractMsbPos(moveMap));
            mv.SetTargetBoardIndex(MapT::GetBoardIndex(0));
            mv.SetKilledBoardIndex(MapT::GetEnemyPawnBoardIndex());
            mv.SetKilledFigureField(ExtractMsbPos(_board.elPassantField));
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetCasltingRights(_board.Castlings);
            mv.SetMoveType(PackedMove::CaptureFlag);

            // preparing heuristic evaluation

            if constexpr (ApplyHeuristicEval)
            {
                int16_t eval = MoveSortEval::ApplyAttackFieldEffects(0, 0, pawnMap, moveMap);
                eval = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, ExtractMsbPos(moveMap));
                mv.SetEval(eval);
            }

            results.Push(_threadStack, mv);

            possiblePawnsToMove ^= pawnMap;
        }
    }

    // TODO: Compare with simple if searching loop
    // TODO: propagate checkForCastling?
    template <bool GenOnlyAttackMoves, bool ApplyHeuristicEval, class MapT, bool checkForCastling = false, bool promotePawns = false,
              bool selectFigures = false, bool isCheck = false,
              uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr>
    void _processFigMoves(payload& results, const uint64_t pawnAttacks, const uint64_t enemyMap, const uint64_t allyMap,
                          const uint64_t pinnedFigMap, [[maybe_unused]] const uint64_t figureSelector = 0,
                          [[maybe_unused]] const uint64_t allowedMovesSelector = 0)
    {
        const uint64_t fullMap = enemyMap | allyMap;
        uint64_t pinnedOnes = pinnedFigMap & _board.boards[MapT::GetBoardIndex(_board.movColor)];
        uint64_t unpinnedOnes = _board.boards[MapT::GetBoardIndex(_board.movColor)] ^ pinnedOnes;

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
            const int figPos = ExtractMsbPos(unpinnedOnes);
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
            std::bitset<Board::CastlingCount + 1> updatedCastlings = _board.Castlings;
            if constexpr (checkForCastling)
                updatedCastlings[RookMap::GetMatchingCastlingIndex(_board, figBoard)] = false;

            // preparing moves
            const uint64_t attackMoves = figMoves & enemyMap;
            [[maybe_unused]] const uint64_t nonAttackingMoves = figMoves ^ attackMoves;

            // processing move consequneces

            if constexpr (GenOnlyAttackMoves == false)
                _processNonAttackingMoves<ApplyHeuristicEval, promotePawns, elPassantFieldDeducer>(results, pawnAttacks, nonAttackingMoves,
                                                                               MapT::GetBoardIndex(_board.movColor),
                                                                               figBoard, updatedCastlings);

            _processAttackingMoves<ApplyHeuristicEval, promotePawns>(results, pawnAttacks, attackMoves,
                                                 MapT::GetBoardIndex(_board.movColor), figBoard, updatedCastlings);

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
                _processNonAttackingMoves<ApplyHeuristicEval, promotePawns, elPassantFieldDeducer>(results, pawnAttacks, nonAttackingMoves,
                                                                               MapT::GetBoardIndex(_board.movColor),
                                                                               figBoard, _board.Castlings);

            // TODO: There is exactly one move possible
            _processAttackingMoves<ApplyHeuristicEval, promotePawns>(results, pawnAttacks, attackMoves,
                                                 MapT::GetBoardIndex(_board.movColor), figBoard, _board.Castlings);

            pinnedOnes ^= figBoard;
        }
    }

    // TODO: improve readability of code below
    template <bool ApplyHeuristicEval, bool promotePawns, uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr>
    void _processNonAttackingMoves(payload& results, const uint64_t pawnAttacks, uint64_t nonAttackingMoves,
                                   const size_t figBoardIndex, const uint64_t startField,
                                   const std::bitset<Board::CastlingCount + 1> castlings) const
    {
        while (nonAttackingMoves)
        {
            // extracting moves
            const int movePos = ExtractMsbPos(nonAttackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;

            if constexpr (!promotePawns)
            // simple figure case
            {
                Move mv{};

                // preparing basic move info
                mv.SetStartField(ExtractMsbPos(startField));
                mv.SetStartBoardIndex(figBoardIndex);
                mv.SetTargetField(movePos);
                mv.SetTargetBoardIndex(figBoardIndex);
                mv.SetKilledBoardIndex(Board::SentinelBoardIndex);

                // if el passant line is passed when figure moved to these line flag will turned on
                if constexpr (elPassantFieldDeducer != nullptr)
                {
                    // TODO: CHANGED TEMPORALRALKSFLAJSF TEMP
                    if (const auto result = elPassantFieldDeducer(moveBoard, startField); result == 0)
                        mv.SetElPassantField(Board::InvalidElPassantField);
                    else
                        mv.SetElPassantField(ExtractMsbPos(result));
                }
                else
                    mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(castlings);

                // preparing heuristic evaluation

                if constexpr (ApplyHeuristicEval)
                {
                    int16_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, startField, moveBoard);
                    eval = MoveSortEval::ApplyKillerMoveEffect(eval, _kTable, mv, _depthLeft);
                    eval = MoveSortEval::ApplyCounterMoveEffect(eval, _counterMove, mv);
                    eval = MoveSortEval::ApplyHistoryTableBonus(eval, mv, _hTable);
                    mv.SetEval(eval);
                }

                results.Push(_threadStack, mv);
            }
            if constexpr (promotePawns)
            // upgrading pawn case
            {
                // iterating through upgradable pieces
                for (size_t i = knightsIndex; i < kingIndex; ++i)
                {
                    const auto TargetBoard = _board.movColor * Board::BoardsPerCol + i;

                    Move mv{};

                    // preparing basic move info
                    mv.SetStartField(ExtractMsbPos(startField));
                    mv.SetStartBoardIndex(figBoardIndex);
                    mv.SetTargetField(movePos);
                    mv.SetTargetBoardIndex(TargetBoard);
                    mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
                    mv.SetElPassantField(Board::InvalidElPassantField);
                    mv.SetCasltingRights(castlings);
                    mv.SetMoveType(PromotingMove | PromoFlags[i]);

                    // preparing heuristic eval info
                    if constexpr (ApplyHeuristicEval)
                    {
                        int16_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks,
                                                                          startField, moveBoard);
                        eval = MoveSortEval::ApplyPromotionEffects(eval, TargetBoard);
                        mv.SetEval(eval);
                    }

                    results.Push(_threadStack, mv);
                }
            }

            nonAttackingMoves ^= moveBoard;
        }
    }

    template <bool ApplyHeuristicEval, bool promotePawns>
    void _processAttackingMoves(payload& results, const uint64_t pawnAttacks, uint64_t attackingMoves,
                                const size_t figBoardIndex, const uint64_t startField,
                                const std::bitset<Board::CastlingCount + 1> castlings) const
    {
        while (attackingMoves)
        {
            // extracting moves
            const int movePos = ExtractMsbPos(attackingMoves);
            const uint64_t moveBoard = maxMsbPossible >> movePos;
            const size_t attackedFigBoardIndex =
                _mechanics.GetIndexOfContainingBoard(moveBoard, SwapColor(_board.movColor));

            if constexpr (!promotePawns)
            // simple figure case
            {
                Move mv{};

                // preparing basic move info
                mv.SetStartField(ExtractMsbPos(startField));
                mv.SetStartBoardIndex(figBoardIndex);
                mv.SetTargetField(movePos);
                mv.SetTargetBoardIndex(figBoardIndex);
                mv.SetKilledBoardIndex(attackedFigBoardIndex);
                mv.SetKilledFigureField(movePos);
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(castlings);
                mv.SetMoveType(PackedMove::CaptureFlag);

                // preparing heuristic eval info
                if constexpr (ApplyHeuristicEval)
                {
                    int16_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks,
                                                       startField, moveBoard);
                    eval = MoveSortEval::ApplyKilledFigEffect(eval, figBoardIndex, attackedFigBoardIndex);
                    eval = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, movePos);
                    mv.SetEval(eval);
                }

                results.Push(_threadStack, mv);
            }
            if constexpr (promotePawns)
            // upgrading pawn case
            {
                // iterating through upgradable pieces
                for (size_t i = knightsIndex; i < kingIndex; ++i)
                {
                    const auto targetBoard = _board.movColor * Board::BoardsPerCol + i;

                    Move mv{};

                    // preparing basic move info
                    mv.SetStartField(ExtractMsbPos(startField));
                    mv.SetStartBoardIndex(figBoardIndex);
                    mv.SetTargetField(movePos);
                    mv.SetTargetBoardIndex(targetBoard);
                    mv.SetKilledBoardIndex(attackedFigBoardIndex);
                    mv.SetKilledFigureField(movePos);
                    mv.SetElPassantField(Board::InvalidElPassantField);
                    mv.SetCasltingRights(castlings);
                    mv.SetMoveType(PackedMove::CaptureFlag | PackedMove::PromoFlag | PromoFlags[i]);

                    // prapring heuristic eval info

                    if constexpr (ApplyHeuristicEval)
                    {
                        int16_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks,
                                                      startField, moveBoard);
                        eval = MoveSortEval::ApplyKilledFigEffect(eval, figBoardIndex, attackedFigBoardIndex);
                        eval = MoveSortEval::ApplyPromotionEffects(eval, targetBoard);
                        eval = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, movePos);
                        mv.SetEval(eval);
                    }

                    results.Push(_threadStack, mv);
                }
            }

            attackingMoves ^= moveBoard;
        }
    }

    // TODO: test copying all old castlings
    template <bool GenOnlyAttackMoves, bool ApplyHeuristicEval>
    void _processPlainKingMoves(payload& results, const uint64_t blockedFigMap, const uint64_t allyMap,
                                const uint64_t enemyMap) const
    {
        static constexpr size_t CastlingPerColor = 2;

        // generating moves
        const uint64_t kingMoves = KingMap::GetMoves(_board.GetKingMsbPos(_board.movColor)) & ~blockedFigMap & ~allyMap;
        uint64_t attackingMoves = kingMoves & enemyMap;
        [[maybe_unused]] uint64_t nonAttackingMoves = kingMoves ^ attackingMoves;

        // preparing variables
        auto castlings = _board.Castlings;
        castlings[CastlingPerColor * _board.movColor + KingCastlingIndex] = false;
        castlings[CastlingPerColor * _board.movColor + QueenCastlingIndex] = false;

        const int oldKingPos = ExtractMsbPos(_board.boards[_board.movColor * Board::BoardsPerCol + kingIndex]);

        // processing simple non attacking moves
        if constexpr (GenOnlyAttackMoves == false)
            while (nonAttackingMoves)
            {
                // extracting new king position data
                const int newPos = ExtractMsbPos(nonAttackingMoves);

                Move mv{};

                // preparing basic move info
                mv.SetStartField(oldKingPos);
                mv.SetStartBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetTargetField(newPos);
                mv.SetTargetBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(castlings);

                // preparing heuristic eval info

                if constexpr (ApplyHeuristicEval)
                {
                    int16_t eval = MoveSortEval::ApplyKillerMoveEffect(0, _kTable, mv, _depthLeft);
                    eval = MoveSortEval::ApplyCounterMoveEffect(eval, _counterMove, mv);
                    eval = MoveSortEval::ApplyHistoryTableBonus(eval, mv, _hTable);
                    mv.SetEval(eval);
                }

                results.Push(_threadStack, mv);

                nonAttackingMoves ^= (maxMsbPossible >> newPos);
            }

        // processing slightly more complicated attacking moves
        while (attackingMoves)
        {
            // extracting new king position data
            const int newPos = ExtractMsbPos(attackingMoves);
            const uint64_t newKingBoard = maxMsbPossible >> newPos;

            // finding attacked figure
            const size_t attackedFigBoardIndex =
                _mechanics.GetIndexOfContainingBoard(newKingBoard, SwapColor(_board.movColor));

            Move mv{};

            // preparing basic move info
            mv.SetStartField(oldKingPos);
            mv.SetStartBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
            mv.SetTargetField(newPos);
            mv.SetTargetBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
            mv.SetKilledBoardIndex(attackedFigBoardIndex);
            mv.SetKilledFigureField(newPos);
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetCasltingRights(castlings);
            mv.SetMoveType(PackedMove::CaptureFlag);

            // preparing heuristic eval info

            if constexpr (ApplyHeuristicEval)
            {
                int16_t eval = MoveSortEval::ApplyCaptureMostRecentSquareEffect(0, _mostRecentSq, newPos);
                eval += MoveSortEval::FigureEval[attackedFigBoardIndex]; // adding value of killed figure
                mv.SetEval(eval);
            }

            results.Push(_threadStack, mv);

            attackingMoves ^= newKingBoard;
        }
    }

    // TODO: simplify ifs??
    // TODO: cleanup left castling available when rook is dead then propagate no castling checking?
    template<bool ApplyHeuristicEval>
    void _processKingCastlings(payload& results, const uint64_t blockedFigMap, const uint64_t fullMap) const
    {
        for (size_t i = 0; i < Board::CastlingsPerColor; ++i)
            if (const size_t castlingIndex = _board.movColor * Board::CastlingsPerColor + i;
                _board.Castlings[castlingIndex] &&
                (Board::CastlingsRookMaps[castlingIndex] &
                 _board.boards[_board.movColor * Board::BoardsPerCol + rooksIndex]) != 0 &&
                (Board::CastlingSensitiveFields[castlingIndex] & blockedFigMap) == 0 &&
                (Board::CastlingTouchedFields[castlingIndex] & fullMap) == 0)
            {
                auto castlings = _board.Castlings;
                castlings[_board.movColor * Board::CastlingsPerColor + KingCastlingIndex] = false;
                castlings[_board.movColor * Board::CastlingsPerColor + QueenCastlingIndex] = false;

                Move mv{};

                // preparing basic move info
                mv.SetStartField(ExtractMsbPos(Board::DefaultKingBoards[_board.movColor]));
                mv.SetStartBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetTargetField(Board::CastlingNewKingPos[castlingIndex]);
                mv.SetTargetBoardIndex(_board.movColor * Board::BoardsPerCol + kingIndex);
                mv.SetKilledBoardIndex(_board.movColor * Board::BoardsPerCol + rooksIndex);
                mv.SetKilledFigureField(ExtractMsbPos(Board::CastlingsRookMaps[castlingIndex]));
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(castlings);
                mv.SetCastlingType(1 + castlingIndex);
                mv.SetMoveType(PackedMove::CastlingFlag);

                // preapring heuristic eval

                if constexpr (ApplyHeuristicEval)
                {
                    int16_t eval = MoveSortEval::ApplyKillerMoveEffect(0, _kTable, mv, _depthLeft);
                    eval = MoveSortEval::ApplyCounterMoveEffect(eval, _counterMove, mv);
                    eval = MoveSortEval::ApplyHistoryTableBonus(eval, mv, _hTable);
                    mv.SetEval(eval);
                }

                results.Push(_threadStack, mv);
            }
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint16_t PromoFlags[]
    {
        0,
        PackedMove::KnightFlag,
        PackedMove::BishopFlag,
        PackedMove::RookFlag,
        PackedMove::QueenFlag,
    };

    // Move generation componentss
    ChessMechanics _mechanics;
    stack<Move, DefaultStackSize>& _threadStack;
    Board& _board;

    // Heuristic evaluation components
    const PackedMove _counterMove;
    const KillerTable& _kTable;
    const HistoricTable& _hTable;
    int _depthLeft;
    int _mostRecentSq;
};

#endif  // MOVEGENERATOR_H
