//
// Created by Jlisowskyy on 3/4/24.
//

#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "../BitOperations.h"
#include "../EngineUtils.h"
#include "../Evaluation/KillerTable.h"
#include "../Evaluation/MoveSortEval.h"
#include "../ThreadManagement/Stack.h"
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
#include <map>
#include <queue>

struct MoveGenerator : ChessMechanics
{
    using stck    = Stack<Move, DEFAULT_STACK_SIZE>;
    using payload = stck::StackPayload;

    // ------------------------------
    // Class Creation
    // ------------------------------

    MoveGenerator() = delete;

    explicit MoveGenerator(
        const Board &bd, Stack<Move, DEFAULT_STACK_SIZE> &s, const HistoricTable &ht, const KillerTable &kt
    )
        : ChessMechanics(bd), _threadStack(s), _kTable(kt), _hTable(ht)
    {
    }

    MoveGenerator(MoveGenerator &other) = delete;

    MoveGenerator &operator=(MoveGenerator &) = delete;

    ~MoveGenerator() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template <bool GenOnlyTacticalMoves = false, bool ApplyHeuristicEval = true>
    payload GetMovesFast(PackedMove counterMove, int ply, int mostRecentMovedSquare);

    template <bool GenOnlyTacticalMoves = false, bool ApplyHeuristicEval = true>
    payload GetMovesSlow(PackedMove counterMove, int ply, int mostRecentMovedSquare);

    template <bool GenOnlyTacticalMoves = false, bool ApplyHeuristicEval = true>
    INLINE payload GetMovesSlow(){
        return GetMovesSlow<GenOnlyTacticalMoves, ApplyHeuristicEval>({}, MAX_SEARCH_DEPTH, -1);
    }

    template <bool GenOnlyTacticalMoves = false, bool ApplyHeuristicEval = true>
    INLINE payload GetMovesFast()
    {
        return GetMovesFast<GenOnlyTacticalMoves, ApplyHeuristicEval>({}, MAX_SEARCH_DEPTH, -1);
    }

    template <bool GenOnlyTacticalMoves = false, bool ApplyHeuristicEval = true>
    payload GetPseudoLegalMoves(PackedMove counterMove, int ply, int mostRecentMovedSquare);

    template <bool GenOnlyTacticalMoves = false, bool ApplyHeuristicEval = true>
    INLINE payload GetPseudoLegalMoves()
    {
        return GetPseudoLegalMoves<GenOnlyTacticalMoves, ApplyHeuristicEval>({}, MAX_SEARCH_DEPTH, -1);
    }

    [[nodiscard]] static bool IsLegal(Board& bd, Move mv);

    std::map<std::string, uint64_t> GetCountedMoves(int depth);
    uint64_t CountMoves(Board &bd, int depth);

    using ChessMechanics::IsCheck;
    using ChessMechanics::IsDrawByReps;

    // ------------------------------
    // private methods
    // ------------------------------

    private:

    [[nodiscard]] static bool _isCastlingLegal(Board& bd, Move mv);
    [[nodiscard]] static bool _isNormalMoveLegal(Board& bd, Move mv);

    INLINE void _saveKillerPly(const int ply)
    {
        // Note: read from empty floor on root
        _ply = ply - 1 >= 0 ? ply - 1 : static_cast<int>(KillerTable::SentinelReadFloor);
    }

    template <class MapT>
    [[nodiscard]] INLINE bool _isGivingCheck(const int msbPos, const uint64_t fullMap, const int enemyColor) const
    {
        const uint64_t enemyKing = _board.BitBoards[enemyColor * Board::BitBoardsPerCol + kingIndex];
        const uint64_t moves     = MapT::GetMoves(msbPos, fullMap, 0);

        return (enemyKing & moves) != 0;
    }

    template <class MapT> [[nodiscard]] INLINE bool _isPawnGivingCheck(const uint64_t pawnBitMap) const
    {
        const int enemyColor     = SwapColor(MapT::GetColor());
        const uint64_t enemyKing = _board.BitBoards[enemyColor * Board::BitBoardsPerCol + kingIndex];
        const uint64_t moves     = MapT::GetAttackFields(pawnBitMap);

        return (enemyKing & moves) != 0;
    }

    template <class MapT>
    [[nodiscard]] INLINE bool
    _isPromotingPawnGivingCheck(const int msbPos, const uint64_t fullMap, const int targetBitBoardIndex) const
    {
        static constexpr uint64_t (*moveGenerators[])(int, uint64_t, uint64_t){
            nullptr, KnightMap::GetMoves, BishopMap::GetMoves, RookMap::GetMoves, QueenMap::GetMoves,
        };

        const int color          = MapT::GetColor();
        const int enemyColor     = SwapColor(color);
        const uint64_t enemyKing = _board.BitBoards[enemyColor * Board::BitBoardsPerCol + kingIndex];
        auto func                = moveGenerators[targetBitBoardIndex - color * Board::BitBoardsPerCol];
        const uint64_t moves     = func(msbPos, fullMap, 0);

        return (enemyKing & moves) != 0;
    }

    template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval>
    void _noCheckGen(payload &results, uint64_t fullMap, uint64_t blockedFigMap);

    template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval>
    void _singleCheckGen(payload &results, uint64_t fullMap, uint64_t blockedFigMap, int checkType);

    template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval>
    void _doubleCheckGen(payload &results, uint64_t blockedFigMap) const;

    template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT, bool isCheck = false>
    void _processPawnMoves(
        payload &results, uint64_t pawnAttacks, uint64_t enemyMap, uint64_t allyMap, uint64_t pinnedFigMap,
        [[maybe_unused]] uint64_t allowedMoveFilter = 0
    );

    template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT>
    void _processPawnPseudoMoves(
            payload &results, uint64_t pawnAttacks, uint64_t enemyMap, uint64_t allyMap
    );

    template <bool ApplyHeuristicEval, bool IsQSearch, class MapT>
    void _processPawnAttackPseudoMoves(
            payload &results, uint64_t pawnAttacks, uint64_t enemyMap, uint64_t fullMap, uint64_t promoMoves, uint64_t nonPromoMoves
    );

    template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT>
    void _processPawnPlainPseudoMoves(
            payload &results, uint64_t pawnAttacks, uint64_t fullMap, uint64_t promoPieces, uint64_t nonPromoPieces
    );

    template <bool ApplyHeuristicEval, class MapT>
    void _processElPassantPseudoMoves(
            payload &results, uint64_t pawnAttacks, uint64_t pieces
    );


    // TODO: Consider different solution?
    template <bool ApplyHeuristicEval, class MapT, bool isCheck = false>
    void _processElPassantMoves(
        payload &results, uint64_t fullMap, uint64_t pinnedFigMap, [[maybe_unused]] uint64_t allowedMoveFilter = 0
    );

    // TODO: Compare with simple if searching loop
    // TODO: propagate checkForCastling?
    template <
        bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT, bool checkForCastling = false,
        bool promotePawns = false, bool selectFigures = false, bool isCheck = false,
        uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr>
    void _processFigMoves(
        payload &results, uint64_t pawnAttacks, uint64_t enemyMap, uint64_t allyMap, uint64_t pinnedFigMap,
        [[maybe_unused]] uint64_t figureSelector = 0, [[maybe_unused]] uint64_t allowedMoveSelector = 0
    );

    template <
            bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT, bool checkForCastling = false
    >
    void _processFigPseudoMoves(
            payload &results, uint64_t pawnAttacks, uint64_t enemyMap, uint64_t allyMap
    );


    // TODO: improve readability of code below
    template <
        class MapT, bool ApplyHeuristicEval, bool promotePawns,
        uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t) = nullptr>
    void _processNonAttackingMoves(
        payload &results, uint64_t pawnAttacks, uint64_t nonAttackingMoves, size_t figBoardIndex, uint64_t startField,
        std::bitset<Board::CastlingCount + 1> castlings, uint64_t fullMap
    ) const;

    template <class MapT, bool IsQsearch, bool ApplyHeuristicEval, bool promotePawns>
    void _processAttackingMoves(
        payload &results, uint64_t pawnAttacks, uint64_t attackingMoves, size_t figBoardIndex, uint64_t startField,
        std::bitset<Board::CastlingCount + 1> castlings, uint64_t fullMap
    ) const;

    static constexpr uint64_t  KING_NO_BLOCKED_MAP = (~static_cast<uint64_t>(0));

    // TODO: test copying all old castlings
    template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, bool GeneratePseudoMoves = false>
    void _processPlainKingMoves(payload &results, uint64_t blockedFigMap, uint64_t allyMap, uint64_t enemyMap) const;

    static constexpr uint64_t CASTLING_PSEUDO_LEGAL_BLOCKED = 0;

    // TODO: simplify ifs??
    // TODO: cleanup left castling available when rook is dead then propagate no castling checking?
    template <bool ApplyHeuristicEval, bool GeneratePseudoLegalMoves = false>
    void _processKingCastlings(payload &results, uint64_t blockedFigMap, uint64_t fullMap) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint16_t PromoFlags[]{
        0, PackedMove::KnightFlag, PackedMove::BishopFlag, PackedMove::RookFlag, PackedMove::QueenFlag,
    };

    // Move generation components
    Stack<Move, DEFAULT_STACK_SIZE> &_threadStack;

    // Heuristic evaluation components
    const KillerTable &_kTable;
    const HistoricTable &_hTable;
    int _ply = MAX_SEARCH_DEPTH;
    int _mostRecentSq = -1;
    PackedMove _counterMove{};
};

template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval>
MoveGenerator::payload
MoveGenerator::GetMovesFast(const PackedMove counterMove, const int ply, const int mostRecentMovedSquare)
{
    // Init of heuristic components
    _counterMove  = counterMove;
    _saveKillerPly(ply);
    _mostRecentSq = mostRecentMovedSquare;

    // Prepare crucial components and additionally detect whether we are at check and which figure type attacks king
    const uint64_t fullMap                             = GetFullBitMap();
    const auto [blockedFigMap, checksCount, checkType] = GetBlockedFieldBitMap(fullMap);

    TraceIfFalse(blockedFigMap != 0, "Blocked fig map must at least contains fields controlled by king!");
    TraceIfFalse(
        checksCount <= 2,
        "We consider only 3 states: no-check, single-check, double-check -> invalid result was returned!"
    );

    payload results = _threadStack.GetPayload();

    // depending on amount of checks branch to desired reaction
    switch (checksCount)
    {
    case 0:
        _noCheckGen<GenOnlyTacticalMoves, ApplyHeuristicEval>(results, fullMap, blockedFigMap);
        break;
    case 1:
        _singleCheckGen<GenOnlyTacticalMoves, ApplyHeuristicEval>(results, fullMap, blockedFigMap, checkType);
        break;
    case 2:
        _doubleCheckGen<GenOnlyTacticalMoves, ApplyHeuristicEval>(results, blockedFigMap);
        break;
#ifndef NDEBUG
    default:
        throw std::runtime_error("[ ERROR ] GetMovesFast didnt detect correct checks count!");
#endif
    }

    return results;
}
template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval>
MoveGenerator::payload MoveGenerator::GetMovesSlow(PackedMove counterMove, int ply, int mostRecentMovedSquare)
{
    Board bd = _board;

    payload load =
        GetPseudoLegalMoves<GenOnlyTacticalMoves, ApplyHeuristicEval>(counterMove, ply, mostRecentMovedSquare);

    std::queue<Move> que{};
    for (size_t i = 0; i < load.size; ++i)
        if (MoveGenerator::IsLegal(bd, load.data[i]))
            que.push(load.data[i]);

    _threadStack.PopAggregate(load);
    payload loadRV = _threadStack.GetPayload();

    while (!que.empty())
    {
        loadRV.Push(_threadStack, que.front());
        que.pop();
    }

    return loadRV;
}

template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval>
void MoveGenerator::_noCheckGen(payload &results, const uint64_t fullMap, const uint64_t blockedFigMap)
{
    TraceIfFalse(fullMap != 0, "Full map is empty!");

    [[maybe_unused]] const auto [pinnedFigsMap, _] =
        GetPinnedFigsMap<ChessMechanics::PinnedFigGen::WoutAllowedTiles>(_board.MovingColor, fullMap);

    const uint64_t enemyMap = GetColBitMap(SwapColor(_board.MovingColor));
    const uint64_t allyMap  = GetColBitMap(_board.MovingColor);

    const uint64_t pawnAttacks =
        _board.MovingColor == WHITE
            ? BlackPawnMap::GetAttackFields(
                  _board.BitBoards[Board::BitBoardsPerCol * SwapColor(_board.MovingColor) + pawnsIndex]
              )
            : WhitePawnMap::GetAttackFields(
                  _board.BitBoards[Board::BitBoardsPerCol * SwapColor(_board.MovingColor) + pawnsIndex]
              );

    _processFigMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, KnightMap>(
        results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap
    );

    _processFigMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, BishopMap>(
        results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap
    );

    _processFigMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, RookMap, true>(
        results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap
    );

    _processFigMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, QueenMap>(
        results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap
    );

    if (_board.MovingColor == WHITE)
        _processPawnMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, WhitePawnMap>(
            results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap
        );
    else
        _processPawnMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, BlackPawnMap>(
            results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap
        );

    _processPlainKingMoves<GenOnlyTacticalMoves, ApplyHeuristicEval>(results, blockedFigMap, allyMap, enemyMap);

    if constexpr (!GenOnlyTacticalMoves)
        _processKingCastlings<ApplyHeuristicEval>(results, blockedFigMap, fullMap);
}

template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval>
void MoveGenerator::_singleCheckGen(
    payload &results, const uint64_t fullMap, const uint64_t blockedFigMap, const int checkType
)
{
    TraceIfFalse(fullMap != 0, "Full map is empty!");
    TraceIfFalse(checkType == simpleFigCheck || checkType == slidingFigCheck, "Invalid check type!");

    static constexpr uint64_t UNUSED = 0;

    // simplifying figure search by distinguishing check types
    const auto [pinnedFigsMap, allowedTilesMap] = [&]() -> std::pair<uint64_t, uint64_t>
    {
        if (checkType == slidingFigCheck)
            return GetPinnedFigsMap<ChessMechanics::PinnedFigGen::WAllowedTiles>(_board.MovingColor, fullMap);

        [[maybe_unused]] const auto [rv, _] =
            GetPinnedFigsMap<ChessMechanics::PinnedFigGen::WoutAllowedTiles>(_board.MovingColor, fullMap);
        return {rv, GetAllowedTilesWhenCheckedByNonSliding()};
    }();

    // helping variable preparation
    const uint64_t enemyMap = GetColBitMap(SwapColor(_board.MovingColor));
    const uint64_t allyMap  = GetColBitMap(_board.MovingColor);

    const uint64_t pawnAttacks =
        _board.MovingColor == WHITE
            ? BlackPawnMap::GetAttackFields(
                  _board.BitBoards[Board::BitBoardsPerCol * SwapColor(_board.MovingColor) + pawnsIndex]
              )
            : WhitePawnMap::GetAttackFields(
                  _board.BitBoards[Board::BitBoardsPerCol * SwapColor(_board.MovingColor) + pawnsIndex]
              );

    // Specific figure processing
    _processFigMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, KnightMap, false, false, false, true>(
        results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap, UNUSED, allowedTilesMap
    );

    _processFigMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, BishopMap, false, false, false, true>(
        results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap, UNUSED, allowedTilesMap
    );

    _processFigMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, RookMap, true, false, false, true>(
        results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap, UNUSED, allowedTilesMap
    );

    _processFigMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, QueenMap, false, false, false, true>(
        results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap, UNUSED, allowedTilesMap
    );

    if (_board.MovingColor == WHITE)
        _processPawnMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, WhitePawnMap, true>(
            results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap, allowedTilesMap
        );
    else
        _processPawnMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, BlackPawnMap, true>(
            results, pawnAttacks, enemyMap, allyMap, pinnedFigsMap, allowedTilesMap
        );

    _processPlainKingMoves<GenOnlyTacticalMoves, ApplyHeuristicEval>(results, blockedFigMap, allyMap, enemyMap);
}

template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval>
void MoveGenerator::_doubleCheckGen(payload &results, const uint64_t blockedFigMap) const
{
    const uint64_t allyMap  = GetColBitMap(_board.MovingColor);
    const uint64_t enemyMap = GetColBitMap(SwapColor(_board.MovingColor));
    _processPlainKingMoves<GenOnlyTacticalMoves, ApplyHeuristicEval>(results, blockedFigMap, allyMap, enemyMap);
}

template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT, bool isCheck>
void MoveGenerator::_processPawnMoves(
    payload &results, const uint64_t pawnAttacks, const uint64_t enemyMap, const uint64_t allyMap,
    const uint64_t pinnedFigMap, const uint64_t allowedMoveFilter
)
{
    const uint64_t promotingPawns    = _board.BitBoards[MapT::GetBoardIndex(0)] & MapT::PromotingMask;
    const uint64_t nonPromotingPawns = _board.BitBoards[MapT::GetBoardIndex(0)] ^ promotingPawns;

    _processFigMoves<
        GenOnlyTacticalMoves, ApplyHeuristicEval, MapT, false, false, true, isCheck, MapT::GetElPassantField>(
        results, pawnAttacks, enemyMap, allyMap, pinnedFigMap, nonPromotingPawns, allowedMoveFilter
    );

    // During quiesce search we should also check all promotions so GenOnlyTacticalMoves is false
    if (promotingPawns)
        _processFigMoves<false, ApplyHeuristicEval, MapT, false, true, true, isCheck>(
            results, pawnAttacks, enemyMap, allyMap, pinnedFigMap, promotingPawns, allowedMoveFilter
        );

    _processElPassantMoves<ApplyHeuristicEval, MapT, isCheck>(
        results, allyMap | enemyMap, pinnedFigMap, allowedMoveFilter
    );
}

template <bool ApplyHeuristicEval, class MapT, bool isCheck>
void MoveGenerator::_processElPassantMoves(
    payload &results, const uint64_t fullMap, const uint64_t pinnedFigMap, const uint64_t allowedMoveFilter
)
{
    TraceIfFalse(fullMap != 0, "Full map is empty!");

    if (_board.ElPassantField == Board::InvalidElPassantBitBoard)
        return;

    // calculation preparation
    const uint64_t suspectedFields = MapT::GetElPassantSuspectedFields(_board.ElPassantField);
    const size_t enemyCord         = SwapColor(_board.MovingColor) * Board::BitBoardsPerCol;
    const uint64_t enemyRookFigs = _board.BitBoards[enemyCord + queensIndex] | _board.BitBoards[enemyCord + rooksIndex];
    uint64_t possiblePawnsToMove = _board.BitBoards[MapT::GetBoardIndex(0)] & suspectedFields;

    while (possiblePawnsToMove)
    {
        const uint64_t pawnMap = MaxMsbPossible >> ExtractMsbPos(possiblePawnsToMove);

        // checking whether move would affect horizontal line attacks on king
        const uint64_t processedPawns      = pawnMap | _board.ElPassantField;
        const uint64_t cleanedFromPawnsMap = fullMap ^ processedPawns;
        if (const uint64_t kingHorizontalLine =
                RookMap::GetMoves(_board.GetKingMsbPos(_board.MovingColor), cleanedFromPawnsMap) &
                MapT::EnemyElPassantMask;
            (kingHorizontalLine & enemyRookFigs) != 0)
            return;

        const uint64_t moveMap = MapT::GetElPassantMoveField(_board.ElPassantField);

        // checking whether moving some pawns would undercover king on some line
        if ((processedPawns & pinnedFigMap) != 0)
        {
            // two separate situations that's need to be considered, every pawn that participate in el passant move
            // should be unblocked on specific lines

            if ((pawnMap & pinnedFigMap) != 0 &&
                (GenerateAllowedTilesForPrecisedPinnedFig(pawnMap, fullMap) & moveMap) == 0)
            {
                possiblePawnsToMove ^= pawnMap;
                continue;
            }
            if ((GenerateAllowedTilesForPrecisedPinnedFig(_board.ElPassantField, fullMap) & moveMap) == 0)
            {
                possiblePawnsToMove ^= pawnMap;
                continue;
            }
        }

        // When king is checked only if move is going to allow tile el passant is correct
        if constexpr (isCheck)
            if ((moveMap & allowedMoveFilter) == 0 && (_board.ElPassantField & allowedMoveFilter) == 0)
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
        mv.SetKilledFigureField(ExtractMsbPos(_board.ElPassantField));
        mv.SetElPassantField(Board::InvalidElPassantField);
        mv.SetCasltingRights(_board.Castlings);
        mv.SetMoveType(PackedMove::CaptureFlag);

        if (_isPawnGivingCheck<MapT>(moveMap))
            mv.SetCheckType();

        // preparing heuristic evaluation

        if constexpr (ApplyHeuristicEval)
        {
            int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, 0, pawnMap, moveMap);
            eval = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, ExtractMsbPos(moveMap));
            mv.SetEval(static_cast<int16_t>(eval));
        }

        results.Push(_threadStack, mv);

        possiblePawnsToMove ^= pawnMap;
    }
}

template <
    bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT, bool checkForCastling, bool promotePawns,
    bool selectFigures, bool isCheck, uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t)>
void MoveGenerator::_processFigMoves(
    payload &results, const uint64_t pawnAttacks, const uint64_t enemyMap, const uint64_t allyMap,
    const uint64_t pinnedFigMap, const uint64_t figureSelector, const uint64_t allowedMoveSelector
)
{
    TraceIfFalse(enemyMap != 0, "Enemy map is empty!");
    TraceIfFalse(allyMap != 0, "Ally map is empty!");

    const uint64_t fullMap = enemyMap | allyMap;
    uint64_t pinnedOnes    = pinnedFigMap & _board.BitBoards[MapT::GetBoardIndex(_board.MovingColor)];
    uint64_t unpinnedOnes  = _board.BitBoards[MapT::GetBoardIndex(_board.MovingColor)] ^ pinnedOnes;

    // applying filter if needed
    if constexpr (selectFigures)
    {
        pinnedOnes &= figureSelector;
        unpinnedOnes &= figureSelector;
    }

    // saving results of previous el passant field, used only when the figure is not a pawn

    // processing unpinned moves
    while (unpinnedOnes)
    {
        // processing moves
        const int figPos        = ExtractMsbPos(unpinnedOnes);
        const uint64_t figBoard = MaxMsbPossible >> figPos;

        // selecting allowed moves if in check
        const uint64_t figMoves = [&]() constexpr
        {
            if constexpr (!isCheck)
                return MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap;
            if constexpr (isCheck)
                return MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap & allowedMoveSelector;
        }();

        // Performing checks for castlings
        std::bitset<Board::CastlingCount + 1> updatedCastlings = _board.Castlings;
        if constexpr (checkForCastling)
            updatedCastlings[RookMap::GetMatchingCastlingIndex(_board, figBoard)] = false;

        // preparing moves
        const uint64_t attackMoves                        = figMoves & enemyMap;
        [[maybe_unused]] const uint64_t nonAttackingMoves = figMoves ^ attackMoves;

        // processing move consequences

        if constexpr (!GenOnlyTacticalMoves)
            _processNonAttackingMoves<MapT, ApplyHeuristicEval, promotePawns, elPassantFieldDeducer>(
                results, pawnAttacks, nonAttackingMoves, MapT::GetBoardIndex(_board.MovingColor), figBoard,
                updatedCastlings, fullMap
            );

        _processAttackingMoves<MapT, GenOnlyTacticalMoves, ApplyHeuristicEval, promotePawns>(
            results, pawnAttacks, attackMoves, MapT::GetBoardIndex(_board.MovingColor), figBoard, updatedCastlings,
            fullMap
        );

        unpinnedOnes ^= figBoard;
    }

    // if a check is detected, the pinned figure stays in place
    if constexpr (isCheck)
        return;

    // processing pinned moves
    // Note: corner Rook possibly applicable to castling cannot be pinned
    while (pinnedOnes)
    {
        // processing moves
        const int figPos            = ExtractMsbPos(pinnedOnes);
        const uint64_t figBoard     = MaxMsbPossible >> figPos;
        const uint64_t allowedTiles = GenerateAllowedTilesForPrecisedPinnedFig(figBoard, fullMap);
        const uint64_t figMoves     = MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap & allowedTiles;
        // TODO: check applied here?
        // TODO: breaking if there?

        // preparing moves
        const uint64_t attackMoves                        = figMoves & enemyMap;
        [[maybe_unused]] const uint64_t nonAttackingMoves = figMoves ^ attackMoves;

        // processing move consequences

        if constexpr (!GenOnlyTacticalMoves)
            _processNonAttackingMoves<MapT, ApplyHeuristicEval, promotePawns, elPassantFieldDeducer>(
                results, pawnAttacks, nonAttackingMoves, MapT::GetBoardIndex(_board.MovingColor), figBoard,
                _board.Castlings, fullMap
            );

        // TODO: There is exactly one move possible
        _processAttackingMoves<MapT, GenOnlyTacticalMoves, ApplyHeuristicEval, promotePawns>(
            results, pawnAttacks, attackMoves, MapT::GetBoardIndex(_board.MovingColor), figBoard, _board.Castlings,
            fullMap
        );

        pinnedOnes ^= figBoard;
    }
}

template <class MapT, bool ApplyHeuristicEval, bool promotePawns, uint64_t (*elPassantFieldDeducer)(uint64_t, uint64_t)>
void MoveGenerator::_processNonAttackingMoves(
    payload &results, const uint64_t pawnAttacks, uint64_t nonAttackingMoves, const size_t figBoardIndex,
    const uint64_t startField, const std::bitset<Board::CastlingCount + 1> castlings, const uint64_t fullMap
) const
{
    TraceIfFalse(figBoardIndex < Board::BitBoardsCount, "Invalid figure board index!");

    while (nonAttackingMoves)
    {
        // extracting moves
        const int movePos        = ExtractMsbPos(nonAttackingMoves);
        const uint64_t moveBoard = MaxMsbPossible >> movePos;

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

            if (figBoardIndex == wPawnsIndex && _isPawnGivingCheck<WhitePawnMap>(moveBoard))
                mv.SetCheckType();
            else if (figBoardIndex == bPawnsIndex && _isPawnGivingCheck<BlackPawnMap>(moveBoard))
                mv.SetCheckType();
            else if (_isGivingCheck<MapT>(movePos, fullMap ^ startField, SwapColor(figBoardIndex > wKingIndex)))
                mv.SetCheckType();

            // if el passant line is passed when a figure moved to these line flags will turn on
            if constexpr (elPassantFieldDeducer != nullptr)
            {
                // TODO: CHANGED TEMP
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
                int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, startField, moveBoard);
                eval         = MoveSortEval::ApplyKillerMoveEffect(eval, _kTable, mv, _ply);
                eval         = MoveSortEval::ApplyCounterMoveEffect(eval, _counterMove, mv);
                eval         = MoveSortEval::ApplyHistoryTableBonus(eval, mv, _hTable);
                eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
                mv.SetEval(static_cast<int16_t>(eval));
            }

            results.Push(_threadStack, mv);
        }
        if constexpr (promotePawns)
        // upgrading pawn case
        {
            // iterating through upgradable pieces
            for (size_t i = knightsIndex; i < kingIndex; ++i)
            {
                const auto targetBoard = _board.MovingColor * Board::BitBoardsPerCol + i;

                Move mv{};

                // preparing basic move info
                mv.SetStartField(ExtractMsbPos(startField));
                mv.SetStartBoardIndex(figBoardIndex);
                mv.SetTargetField(movePos);
                mv.SetTargetBoardIndex(targetBoard);
                mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(castlings);
                mv.SetMoveType(PackedMove::PromoFlag | PromoFlags[i]);

                if (_isPromotingPawnGivingCheck<MapT>(movePos, fullMap ^ startField, targetBoard))
                    mv.SetCheckType();

                // preparing heuristic eval info
                if constexpr (ApplyHeuristicEval)
                {
                    int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, startField, moveBoard);
                    eval         = MoveSortEval::ApplyPromotionEffects(eval, targetBoard);
                    eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
                    mv.SetEval(static_cast<int16_t>(eval));
                }

                results.Push(_threadStack, mv);
            }
        }

        nonAttackingMoves ^= moveBoard;
    }
}

template <class MapT, bool IsQsearch, bool ApplyHeuristicEval, bool promotePawns>
void MoveGenerator::_processAttackingMoves(
    payload &results, const uint64_t pawnAttacks, uint64_t attackingMoves, const size_t figBoardIndex,
    const uint64_t startField, const std::bitset<Board::CastlingCount + 1> castlings, const uint64_t fullMap
) const
{
    TraceIfFalse(figBoardIndex < Board::BitBoardsCount, "Invalid figure board index!");

    while (attackingMoves)
    {
        // extracting moves
        const int movePos                  = ExtractMsbPos(attackingMoves);
        const uint64_t moveBoard           = MaxMsbPossible >> movePos;
        const size_t attackedFigBoardIndex = GetIndexOfContainingBitBoard(moveBoard, SwapColor(_board.MovingColor));

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

            if (figBoardIndex == wPawnsIndex && _isPawnGivingCheck<WhitePawnMap>(moveBoard))
                mv.SetCheckType();
            else if (figBoardIndex == bPawnsIndex && _isPawnGivingCheck<BlackPawnMap>(moveBoard))
                mv.SetCheckType();
            else if (_isGivingCheck<MapT>(movePos, fullMap ^ startField, SwapColor(figBoardIndex > wKingIndex)))
                mv.SetCheckType();

            // preparing heuristic eval info
            if constexpr (ApplyHeuristicEval)
            {
                int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, startField, moveBoard);
                eval         = MoveSortEval::ApplyCaptureEffect<IsQsearch>([&]() { return SEE(mv); }, eval, figBoardIndex, attackedFigBoardIndex);
                eval         = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, movePos);
                eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());

                mv.SetEval(static_cast<int16_t>(eval));
            }

            results.Push(_threadStack, mv);
        }
        if constexpr (promotePawns)
        // upgrading pawn case
        {
            // iterating through upgradable pieces
            for (size_t i = knightsIndex; i < kingIndex; ++i)
            {
                const auto targetBoard = _board.MovingColor * Board::BitBoardsPerCol + i;

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

                if (_isPromotingPawnGivingCheck<MapT>(movePos, fullMap ^ startField, targetBoard))
                    mv.SetCheckType();

                // preparing heuristic eval info

                if constexpr (ApplyHeuristicEval)
                {
                    int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, startField, moveBoard);
                    eval         = MoveSortEval::ApplyCaptureEffect<IsQsearch>([&]() { return SEE(mv); }, eval, figBoardIndex, attackedFigBoardIndex);
                    eval         = MoveSortEval::ApplyPromotionEffects(eval, targetBoard);
                    eval         = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, movePos);
                    eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
                    mv.SetEval(static_cast<int16_t>(eval));
                }

                results.Push(_threadStack, mv);
            }
        }

        attackingMoves ^= moveBoard;
    }
}

template <bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, bool GeneratePseudoMoves>
void MoveGenerator::_processPlainKingMoves(
    payload &results, const uint64_t blockedFigMap, const uint64_t allyMap, const uint64_t enemyMap
) const
{
    TraceIfFalse(allyMap != 0, "Ally map is empty!");
    TraceIfFalse(enemyMap != 0, "Enemy map is empty!");

    static constexpr size_t CastlingPerColor = 2;

    // generating moves
    const uint64_t kingMoves = KingMap::GetMoves(_board.GetKingMsbPos(_board.MovingColor)) &
            (GeneratePseudoMoves ? KING_NO_BLOCKED_MAP : ~blockedFigMap) &
            ~allyMap &
            // NOTE: when we do not use blocked fig map we should block kings to prevent attacking themselves
            (GeneratePseudoMoves ? ~KingMap::GetMoves(_board.GetKingMsbPos(SwapColor(_board.MovingColor))) : KING_NO_BLOCKED_MAP);


    uint64_t attackingMoves  = kingMoves & enemyMap;
    [[maybe_unused]] uint64_t nonAttackingMoves = kingMoves ^ attackingMoves;

    // preparing variables
    auto castlings                                                        = _board.Castlings;
    castlings[CastlingPerColor * _board.MovingColor + KingCastlingIndex]  = false;
    castlings[CastlingPerColor * _board.MovingColor + QueenCastlingIndex] = false;

    const int oldKingPos = ExtractMsbPos(_board.BitBoards[_board.MovingColor * Board::BitBoardsPerCol + kingIndex]);

    // processing simple non-attacking moves
    if constexpr (!GenOnlyTacticalMoves)
        while (nonAttackingMoves)
        {
            // extracting new king position data
            const int newPos = ExtractMsbPos(nonAttackingMoves);

            Move mv{};

            // preparing basic move info
            mv.SetStartField(oldKingPos);
            mv.SetStartBoardIndex(_board.MovingColor * Board::BitBoardsPerCol + kingIndex);
            mv.SetTargetField(newPos);
            mv.SetTargetBoardIndex(_board.MovingColor * Board::BitBoardsPerCol + kingIndex);
            mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetCasltingRights(castlings);

            // preparing heuristic eval info
            if constexpr (ApplyHeuristicEval)
            {
                int32_t eval = MoveSortEval::ApplyKillerMoveEffect(0, _kTable, mv, _ply);
                eval         = MoveSortEval::ApplyCounterMoveEffect(eval, _counterMove, mv);
                eval         = MoveSortEval::ApplyHistoryTableBonus(eval, mv, _hTable);
                eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
                mv.SetEval(static_cast<int16_t>(eval));
            }

            results.Push(_threadStack, mv);

            nonAttackingMoves ^= (MaxMsbPossible >> newPos);
        }

    // processing slightly more complicated attacking moves
    while (attackingMoves)
    {
        // extracting new king position data
        const int newPos            = ExtractMsbPos(attackingMoves);
        const uint64_t newKingBoard = MaxMsbPossible >> newPos;

        // finding an attacked figure
        const size_t attackedFigBoardIndex = GetIndexOfContainingBitBoard(newKingBoard, SwapColor(_board.MovingColor));

        Move mv{};

        // preparing basic move info
        mv.SetStartField(oldKingPos);
        mv.SetStartBoardIndex(_board.MovingColor * Board::BitBoardsPerCol + kingIndex);
        mv.SetTargetField(newPos);
        mv.SetTargetBoardIndex(_board.MovingColor * Board::BitBoardsPerCol + kingIndex);
        mv.SetKilledBoardIndex(attackedFigBoardIndex);
        mv.SetKilledFigureField(newPos);
        mv.SetElPassantField(Board::InvalidElPassantField);
        mv.SetCasltingRights(castlings);
        mv.SetMoveType(PackedMove::CaptureFlag);

        // preparing heuristic eval info

        if constexpr (ApplyHeuristicEval)
        {
            int32_t eval = MoveSortEval::ApplyCaptureMostRecentSquareEffect(0, _mostRecentSq, newPos);
            eval += MoveSortEval::FigureEval[attackedFigBoardIndex]; // adding value of the killed figure
            eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
            mv.SetEval(static_cast<int16_t>(eval));
        }

        results.Push(_threadStack, mv);

        attackingMoves ^= newKingBoard;
    }
}


template <bool ApplyHeuristicEval, bool GeneratePseudoLegalMoves>
void MoveGenerator::_processKingCastlings(payload &results, const uint64_t blockedFigMap, const uint64_t fullMap) const
{
    TraceIfFalse(fullMap != 0, "Full map is empty!");

    for (size_t i = 0; i < Board::CastlingsPerColor; ++i)
        if (const size_t castlingIndex = _board.MovingColor * Board::CastlingsPerColor + i;
            _board.Castlings[castlingIndex] &&
            (Board::CastlingsRookMaps[castlingIndex] &
             _board.BitBoards[_board.MovingColor * Board::BitBoardsPerCol + rooksIndex]) != 0 &&
            (Board::CastlingSensitiveFields[castlingIndex] & (GeneratePseudoLegalMoves ? CASTLING_PSEUDO_LEGAL_BLOCKED : blockedFigMap)) == 0 &&
            (Board::CastlingTouchedFields[castlingIndex] & fullMap) == 0)
        {
            auto castlings                                                                = _board.Castlings;
            castlings[_board.MovingColor * Board::CastlingsPerColor + KingCastlingIndex]  = false;
            castlings[_board.MovingColor * Board::CastlingsPerColor + QueenCastlingIndex] = false;

            Move mv{};

            // preparing basic move info
            mv.SetStartField(ExtractMsbPos(Board::DefaultKingBoards[_board.MovingColor]));
            mv.SetStartBoardIndex(_board.MovingColor * Board::BitBoardsPerCol + kingIndex);
            mv.SetTargetField(Board::CastlingNewKingPos[castlingIndex]);
            mv.SetTargetBoardIndex(_board.MovingColor * Board::BitBoardsPerCol + kingIndex);
            mv.SetKilledBoardIndex(_board.MovingColor * Board::BitBoardsPerCol + rooksIndex);
            mv.SetKilledFigureField(ExtractMsbPos(Board::CastlingsRookMaps[castlingIndex]));
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetCasltingRights(castlings);
            mv.SetCastlingType(1 + castlingIndex); // God only knows why I made a sentinel at index 0 at this moment
            mv.SetMoveType(PackedMove::CastlingFlag);

            // preparing heuristic eval

            if constexpr (ApplyHeuristicEval)
            {
                int32_t eval = MoveSortEval::ApplyKillerMoveEffect(0, _kTable, mv, _ply);
                eval         = MoveSortEval::ApplyCounterMoveEffect(eval, _counterMove, mv);
                eval         = MoveSortEval::ApplyHistoryTableBonus(eval, mv, _hTable);
                eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
                mv.SetEval(static_cast<int16_t>(eval));
            }

            results.Push(_threadStack, mv);
        }
}

template<bool ApplyHeuristicEval, class MapT>
void
MoveGenerator::_processElPassantPseudoMoves(MoveGenerator::payload &results,
                                            const uint64_t pawnAttacks, const uint64_t pieces)
{
    if (_board.ElPassantField == Board::InvalidElPassantBitBoard)
        return;

    const uint64_t suspectedFields = MapT::GetElPassantSuspectedFields(_board.ElPassantField);
    uint64_t elPassantPawns = pieces & suspectedFields;

    while (elPassantPawns)
    {
        const int pawnMsb = ExtractMsbPos(elPassantPawns);
        const uint64_t pawnMap = MaxMsbPossible >> pawnMsb;
        const uint64_t moveBitMap = MapT::GetElPassantMoveField(_board.ElPassantField);

        // preparing basic move information
        Move mv{};
        mv.SetStartField(pawnMsb);
        mv.SetStartBoardIndex(MapT::GetBoardIndex(0));
        mv.SetTargetField(ExtractMsbPos(moveBitMap));
        mv.SetTargetBoardIndex(MapT::GetBoardIndex(0));
        mv.SetKilledBoardIndex(MapT::GetEnemyPawnBoardIndex());
        mv.SetKilledFigureField(ExtractMsbPos(_board.ElPassantField));
        mv.SetElPassantField(Board::InvalidElPassantField);
        mv.SetCasltingRights(_board.Castlings);
        mv.SetMoveType(PackedMove::CaptureFlag);

        if (_isPawnGivingCheck<MapT>(moveBitMap))
            mv.SetCheckType();

        // preparing heuristic evaluation
        if constexpr (ApplyHeuristicEval)
        {
            int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, 0, pawnMap, moveBitMap);
            eval = MoveSortEval::ApplyAttackFieldEffects(eval, pawnAttacks, pawnMap, moveBitMap);
            eval = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, ExtractMsbPos(moveBitMap));
            eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
            mv.SetEval(static_cast<int16_t>(eval));
        }

        results.Push(_threadStack, mv);
        elPassantPawns ^= pawnMap;
    }
}

template<bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT>
void
MoveGenerator::_processPawnPlainPseudoMoves(MoveGenerator::payload &results, const uint64_t pawnAttacks,
                                            const uint64_t fullMap, uint64_t promoPieces, const uint64_t nonPromoPieces)
{
    // iterate through every promoting piece
    while (promoPieces)
    {
        const int pawnPos        = ExtractMsbPos(promoPieces);
        const uint64_t pawnBitBoard = MaxMsbPossible >> pawnPos;

        // generate plain moves
        uint64_t plainPseudoMoves = MapT::GetSinglePlainMoves(pawnBitBoard, fullMap);

        while (plainPseudoMoves)
        {
            const int pawnTarget        = ExtractMsbPos(plainPseudoMoves);
            const uint64_t pawnTargetBitBoard = MaxMsbPossible >> pawnTarget;

            // iterating through upgradable pieces
            for (size_t i = knightsIndex; i < kingIndex; ++i)
            {
                const auto targetBoard = _board.MovingColor * Board::BitBoardsPerCol + i;

                Move mv{};

                // preparing basic move info
                mv.SetStartField(pawnPos);
                mv.SetStartBoardIndex(MapT::GetBoardIndex(0));
                mv.SetTargetField(pawnTarget);
                mv.SetTargetBoardIndex(targetBoard);
                mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(_board.Castlings);
                mv.SetMoveType(PackedMove::PromoFlag | PromoFlags[i]);

                if (_isPromotingPawnGivingCheck<MapT>(pawnTarget, fullMap ^ pawnBitBoard, targetBoard))
                    mv.SetCheckType();

                // preparing heuristic eval info
                if constexpr (ApplyHeuristicEval)
                {
                    int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, pawnBitBoard, pawnTargetBitBoard);
                    eval         = MoveSortEval::ApplyPromotionEffects(eval, targetBoard);
                    eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
                    mv.SetEval(static_cast<int16_t>(eval));
                }

                results.Push(_threadStack, mv);
            }

            plainPseudoMoves ^= pawnTargetBitBoard;
        }

        promoPieces ^= pawnBitBoard;
    }

    // All tactical moves generated
    if constexpr (GenOnlyTacticalMoves)
        return;

    uint64_t firstMoves = MapT::GetSinglePlainMoves(nonPromoPieces, fullMap);
    uint64_t firstPawns = MapT::RevertSinglePlainMoves(firstMoves);
    uint64_t secondMoves = MapT::GetSinglePlainMoves(firstMoves & MapT::GetSinglePlainMoves(MapT::StartMask, fullMap), fullMap);
    uint64_t secondPawns = MapT::RevertSinglePlainMoves(MapT::RevertSinglePlainMoves(secondMoves));

    // go through single upfront moves
    while (firstMoves)
    {
        const int moveMsb = ExtractMsbPos(firstMoves);
        const int pawnMsb = ExtractMsbPos(firstPawns);
        const uint64_t pawnBitBoard = MaxMsbPossible >> pawnMsb;
        const uint64_t moveBitBoard = MaxMsbPossible >> moveMsb;

        Move mv{};

        // preparing basic move info
        mv.SetStartField(pawnMsb);
        mv.SetStartBoardIndex(MapT::GetBoardIndex(0));
        mv.SetTargetField(moveMsb);
        mv.SetTargetBoardIndex(MapT::GetBoardIndex(0));
        mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
        mv.SetElPassantField(Board::InvalidElPassantField);
        mv.SetCasltingRights(_board.Castlings);

        if (_isPawnGivingCheck<MapT>(moveBitBoard))
            mv.SetCheckType();

        // preparing heuristic evaluation
        if constexpr (ApplyHeuristicEval)
        {
            int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, pawnBitBoard, moveBitBoard);
            eval         = MoveSortEval::ApplyKillerMoveEffect(eval, _kTable, mv, _ply);
            eval         = MoveSortEval::ApplyCounterMoveEffect(eval, _counterMove, mv);
            eval         = MoveSortEval::ApplyHistoryTableBonus(eval, mv, _hTable);
            eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
            mv.SetEval(static_cast<int16_t>(eval));
        }

        results.Push(_threadStack, mv);
        firstMoves ^= moveBitBoard;
        firstPawns ^= pawnBitBoard;
    }

    // go through double upfront moves
    while (secondMoves)
    {
        const int moveMsb = ExtractMsbPos(secondMoves);
        const int pawnMsb = ExtractMsbPos(secondPawns);
        const uint64_t pawnBitBoard = MaxMsbPossible >> pawnMsb;
        const uint64_t moveBitBoard = MaxMsbPossible >> moveMsb;

        Move mv{};

        // preparing basic move info
        mv.SetStartField(pawnMsb);
        mv.SetStartBoardIndex(MapT::GetBoardIndex(0));
        mv.SetTargetField(moveMsb);
        mv.SetTargetBoardIndex(MapT::GetBoardIndex(0));
        mv.SetKilledBoardIndex(Board::SentinelBoardIndex);
        mv.SetElPassantField(moveMsb);
        mv.SetCasltingRights(_board.Castlings);

        if (_isPawnGivingCheck<MapT>(moveBitBoard))
            mv.SetCheckType();

        // preparing heuristic evaluation
        if constexpr (ApplyHeuristicEval)
        {
            int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, pawnBitBoard, moveBitBoard);
            eval         = MoveSortEval::ApplyKillerMoveEffect(eval, _kTable, mv, _ply);
            eval         = MoveSortEval::ApplyCounterMoveEffect(eval, _counterMove, mv);
            eval         = MoveSortEval::ApplyHistoryTableBonus(eval, mv, _hTable);
            eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());
            mv.SetEval(static_cast<int16_t>(eval));
        }

        results.Push(_threadStack, mv);
        secondMoves ^= moveBitBoard;
        secondPawns ^= pawnBitBoard;
    }
}

template<bool ApplyHeuristicEval, bool IsQSearch, class MapT>
void
MoveGenerator::_processPawnAttackPseudoMoves(payload &results, const uint64_t pawnAttacks,
                                             const uint64_t enemyMap, const uint64_t fullMap, uint64_t promoMoves, uint64_t nonPromoMoves)
{
    // NOTE: attack can overlap each other so attacking moves should be generated separately for each piece

    // iterate through every non promoting piece
    while (nonPromoMoves)
    {
        const int pawnPos        = ExtractMsbPos(nonPromoMoves);
        const uint64_t pawnBitBoard = MaxMsbPossible >> pawnPos;

        // generate attacking moves
        uint64_t attackPseudoMoves = MapT::GetAttackFields(pawnBitBoard) & enemyMap;

        while (attackPseudoMoves)
        {
            const int pawnTarget        = ExtractMsbPos(attackPseudoMoves);
            const uint64_t pawnTargetBitBoard = MaxMsbPossible >> pawnTarget;
            const size_t attackedFigBoardIndex = GetIndexOfContainingBitBoard(pawnTargetBitBoard, SwapColor(_board.MovingColor));

            Move mv{};

            // preparing basic move info
            mv.SetStartField(pawnPos);
            mv.SetStartBoardIndex(MapT::GetBoardIndex(0));
            mv.SetTargetField(pawnTarget);
            mv.SetTargetBoardIndex(MapT::GetBoardIndex(0));
            mv.SetKilledBoardIndex(attackedFigBoardIndex);
            mv.SetKilledFigureField(pawnTarget);
            mv.SetElPassantField(Board::InvalidElPassantField);
            mv.SetCasltingRights(_board.Castlings);
            mv.SetMoveType(PackedMove::CaptureFlag);

            if (_isPawnGivingCheck<MapT>(pawnTargetBitBoard))
                mv.SetCheckType();

            // preparing heuristic eval info
            if constexpr (ApplyHeuristicEval)
            {
                int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, pawnBitBoard, pawnTargetBitBoard);
                eval         = MoveSortEval::ApplyCaptureEffect<IsQSearch>([&]() { return SEE(mv); }, eval, MapT::GetBoardIndex(0), attackedFigBoardIndex);
                eval         = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, pawnPos);
                eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());

                mv.SetEval(static_cast<int16_t>(eval));
            }

            results.Push(_threadStack, mv);
            attackPseudoMoves ^= pawnTargetBitBoard;
        }

        nonPromoMoves ^= pawnBitBoard;
    }

    // iterate through every promoting piece
    while (promoMoves)
    {
        const int pawnPos        = ExtractMsbPos(promoMoves);
        const uint64_t pawnBitBoard = MaxMsbPossible >> pawnPos;

        // generate attacking moves
        uint64_t attackPseudoMoves = MapT::GetAttackFields(pawnBitBoard) & enemyMap;

        while (attackPseudoMoves)
        {
            const int pawnTarget        = ExtractMsbPos(attackPseudoMoves);
            const uint64_t pawnTargetBitBoard = MaxMsbPossible >> pawnTarget;
            const size_t attackedFigBoardIndex = GetIndexOfContainingBitBoard(pawnTargetBitBoard, SwapColor(_board.MovingColor));

            // iterating through upgradable pieces
            for (size_t i = knightsIndex; i < kingIndex; ++i)
            {
                const auto targetBoard = _board.MovingColor * Board::BitBoardsPerCol + i;

                Move mv{};

                // preparing basic move info
                mv.SetStartField(pawnPos);
                mv.SetStartBoardIndex(MapT::GetBoardIndex(0));
                mv.SetTargetField(pawnTarget);
                mv.SetTargetBoardIndex(targetBoard);
                mv.SetKilledBoardIndex(attackedFigBoardIndex);
                mv.SetKilledFigureField(pawnTarget);
                mv.SetElPassantField(Board::InvalidElPassantField);
                mv.SetCasltingRights(_board.Castlings);
                mv.SetMoveType(PackedMove::CaptureFlag | PackedMove::PromoFlag | PromoFlags[i]);

                if (_isPromotingPawnGivingCheck<MapT>(pawnTarget, fullMap ^ pawnBitBoard, targetBoard))
                    mv.SetCheckType();

                // preparing heuristic eval info

                if constexpr (ApplyHeuristicEval)
                {
                    int32_t eval = MoveSortEval::ApplyAttackFieldEffects(0, pawnAttacks, pawnBitBoard, pawnTargetBitBoard);
                    eval         = MoveSortEval::ApplyCaptureEffect<IsQSearch>([&]() { return SEE(mv); }, eval, MapT::GetBoardIndex(0), attackedFigBoardIndex);
                    eval         = MoveSortEval::ApplyPromotionEffects(eval, targetBoard);
                    eval         = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, pawnTarget);
                    eval         = MoveSortEval::ApplyCheckBonus(eval, mv.IsChecking());

                    mv.SetEval(static_cast<int16_t>(eval));
                }

                results.Push(_threadStack, mv);
            }

            attackPseudoMoves ^= pawnTargetBitBoard;
        }

        promoMoves ^= pawnBitBoard;
    }
}

template<bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT>
void MoveGenerator::_processPawnPseudoMoves(MoveGenerator::payload &results, const uint64_t pawnAttacks, const uint64_t enemyMap,
                                            const uint64_t allyMap)
{
    // Distinguish pawns that should be promoted from usual ones
    const uint64_t promotingPawns    = _board.BitBoards[MapT::GetBoardIndex(0)] & MapT::PromotingMask;
    const uint64_t nonPromotingPawns = _board.BitBoards[MapT::GetBoardIndex(0)] ^ promotingPawns;
    const uint64_t fullMap = enemyMap | allyMap;

    _processPawnAttackPseudoMoves<ApplyHeuristicEval, GenOnlyTacticalMoves, MapT>(
            results, pawnAttacks, enemyMap, fullMap, promotingPawns, nonPromotingPawns
    );

    _processPawnPlainPseudoMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, MapT>(
            results, pawnAttacks, fullMap, promotingPawns, nonPromotingPawns
    );

    _processElPassantPseudoMoves<ApplyHeuristicEval, MapT>(results, pawnAttacks, nonPromotingPawns);
}

template<bool GenOnlyTacticalMoves, bool ApplyHeuristicEval, class MapT, bool checkForCastling>
void MoveGenerator::_processFigPseudoMoves(MoveGenerator::payload &results, const uint64_t pawnAttacks, const uint64_t enemyMap,
                                           const uint64_t allyMap)
{
    TraceIfFalse(enemyMap != 0, "Enemy map is empty!");
    TraceIfFalse(allyMap != 0, "Ally map is empty!");

    // prepare full map and extract figures BitMap from the board
    const uint64_t fullMap = enemyMap | allyMap;
    uint64_t figures = _board.BitBoards[MapT::GetBoardIndex(_board.MovingColor)];

    // processing unpinned moves
    while (figures)
    {
        // processing moves
        const int figPos        = ExtractMsbPos(figures);
        const uint64_t figBoard = MaxMsbPossible >> figPos;

        // Generating actual pseudo legal moves
        const uint64_t figMoves = MapT::GetMoves(figPos, fullMap, enemyMap) & ~allyMap;

        // Performing checks for castlings
        std::bitset<Board::CastlingCount + 1> updatedCastlings = _board.Castlings;
        if constexpr (checkForCastling)
            updatedCastlings[RookMap::GetMatchingCastlingIndex(_board, figBoard)] = false;

        // preparing moves
        const uint64_t attackMoves                        = figMoves & enemyMap;
        [[maybe_unused]] const uint64_t nonAttackingMoves = figMoves ^ attackMoves;

        // processing move consequences
        if constexpr (!GenOnlyTacticalMoves)
            // Don't add simple moves when we should generate only attacking moves
            _processNonAttackingMoves<MapT, ApplyHeuristicEval, false>(
                    results, pawnAttacks, nonAttackingMoves, MapT::GetBoardIndex(_board.MovingColor), figBoard,
                    updatedCastlings, fullMap
            );

        _processAttackingMoves<MapT, GenOnlyTacticalMoves, ApplyHeuristicEval, false>(
                results, pawnAttacks, attackMoves, MapT::GetBoardIndex(_board.MovingColor), figBoard, updatedCastlings,
                fullMap
        );

        figures ^= figBoard;
    }
}

template<bool GenOnlyTacticalMoves, bool ApplyHeuristicEval>
MoveGenerator::payload MoveGenerator::GetPseudoLegalMoves(const PackedMove counterMove, const int ply, const int mostRecentMovedSquare) {
    // Init of heuristic components
    _counterMove = counterMove;
    _saveKillerPly(ply);
    _mostRecentSq = mostRecentMovedSquare;

    // allocate results container
    payload results = _threadStack.GetPayload();

    // Generate bitboards with corresponding colors
    const uint64_t enemyMap = GetColBitMap(SwapColor(_board.MovingColor));
    const uint64_t allyMap  = GetColBitMap(_board.MovingColor);

    // prepare fields attacked by pawns - it will be used in heuristic sort eval
    const uint64_t pawnAttacks =
            _board.MovingColor == WHITE
            ? BlackPawnMap::GetAttackFields(
                    _board.BitBoards[Board::BitBoardsPerCol * SwapColor(_board.MovingColor) + pawnsIndex]
            )
            : WhitePawnMap::GetAttackFields(
                    _board.BitBoards[Board::BitBoardsPerCol * SwapColor(_board.MovingColor) + pawnsIndex]
            );


    _processFigPseudoMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, KnightMap>(
            results, pawnAttacks, enemyMap, allyMap
    );

    _processFigPseudoMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, BishopMap>(
            results, pawnAttacks, enemyMap, allyMap
    );

    _processFigPseudoMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, RookMap, true>(
            results, pawnAttacks, enemyMap, allyMap
    );

    _processFigPseudoMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, QueenMap>(
            results, pawnAttacks, enemyMap, allyMap
    );

    if (_board.MovingColor == WHITE)
        _processPawnPseudoMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, WhitePawnMap>(
                results, pawnAttacks, enemyMap, allyMap
        );
    else
        _processPawnPseudoMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, BlackPawnMap>(
                results, pawnAttacks, enemyMap, allyMap
        );

    _processPlainKingMoves<GenOnlyTacticalMoves, ApplyHeuristicEval, true>(
            results, KING_NO_BLOCKED_MAP, allyMap, enemyMap
    );

    if constexpr (!GenOnlyTacticalMoves)
        _processKingCastlings<ApplyHeuristicEval, true>(
                results, CASTLING_PSEUDO_LEGAL_BLOCKED, enemyMap | allyMap
        );

    return results;
}


#endif // MOVEGENERATOR_H
