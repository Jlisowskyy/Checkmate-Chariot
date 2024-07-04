//
// Created by Jlisowskyy on 3/22/24.
//

#include "../include/MoveGeneration/MoveGenerator.h"

std::map<std::string, uint64_t> MoveGenerator::GetCountedMoves(const int depth)
{
    TraceIfFalse(depth >= 1, "Depth must be at least 1!");

    std::map<std::string, uint64_t> rv{};
    Board workBoard = _board;

    VolatileBoardData data{_board};

    auto moves = GetMovesFast<false, false>();
    for (size_t i = 0; i < moves.size; ++i)
    {
        Move::MakeMove(moves[i], workBoard);
        const uint64_t moveCount = CountMoves(workBoard, depth - 1);
        Move::UnmakeMove(moves[i], workBoard, data);

        rv.emplace(moves[i].GetLongAlgebraicNotation(), moveCount);
    }

    _threadStack.PopAggregate(moves);
    return rv;
}

uint64_t MoveGenerator::CountMoves(Board &bd, const int depth)
{
    if (depth == 0)
        return 1;

    MoveGenerator mgen{bd, _threadStack, DummyHistoryTable, DummyKillerTable};
    const auto moves = mgen.GetMovesFast<false, false>();

    if (depth == 1)
    {
        _threadStack.PopAggregate(moves);
        return moves.size;
    }

    uint64_t sum{};

    VolatileBoardData data{_board};
    for (size_t i = 0; i < moves.size; ++i)
    {
        Move::MakeMove(moves[i], bd);
        sum += CountMoves(bd, depth - 1);
        Move::UnmakeMove(moves[i], bd, data);
    }

    _threadStack.PopAggregate(moves);
    return sum;
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
            mv.SetEval(static_cast<int16_t>(eval));
        }

        results.Push(_threadStack, mv);
        elPassantPawns ^= pawnMap;
    }
}

template<bool GenOnlyAttackMoves, bool ApplyHeuristicEval, class MapT>
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
                    mv.SetEval(static_cast<int16_t>(eval));
                }

                results.Push(_threadStack, mv);
            }

            plainPseudoMoves ^= pawnTargetBitBoard;
        }

        promoPieces ^= pawnBitBoard;
    }

    // All tactical moves generated
    if constexpr (GenOnlyAttackMoves)
        return;

    uint64_t firstMoves = MapT::GetSinglePlainMoves(nonPromoPieces, fullMap);
    uint64_t firstPawns = MapT::RevertSinglePlainMoves(firstMoves);
    uint64_t secondMoves = MapT::GetSinglePlainMoves(firstMoves, fullMap);
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
            mv.SetEval(static_cast<int16_t>(eval));
        }

        results.Push(_threadStack, mv);
        secondMoves ^= moveBitBoard;
        secondPawns ^= pawnBitBoard;
    }
}

template<bool ApplyHeuristicEval, class MapT>
void
MoveGenerator::_processPawnAttackPseudoMoves(MoveGenerator::payload &results, const uint64_t pawnAttacks,
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
                eval         = MoveSortEval::ApplyKilledFigEffect(eval, MapT::GetBoardIndex(0), attackedFigBoardIndex);
                eval         = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, pawnPos);
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
                    eval         = MoveSortEval::ApplyKilledFigEffect(eval, MapT::GetBoardIndex(0), attackedFigBoardIndex);
                    eval         = MoveSortEval::ApplyPromotionEffects(eval, targetBoard);
                    eval         = MoveSortEval::ApplyCaptureMostRecentSquareEffect(eval, _mostRecentSq, pawnTarget);
                    mv.SetEval(static_cast<int16_t>(eval));
                }

                results.Push(_threadStack, mv);
            }

            attackPseudoMoves ^= pawnTargetBitBoard;
        }

        promoMoves ^= pawnBitBoard;
    }
}

template<bool GenOnlyAttackMoves, bool ApplyHeuristicEval, class MapT>
void MoveGenerator::_processPawnPseudoMoves(MoveGenerator::payload &results, const uint64_t pawnAttacks, const uint64_t enemyMap,
                                            const uint64_t allyMap)
{
    // Distinguish pawns that should be promoted from usual ones
    const uint64_t promotingPawns    = _board.BitBoards[MapT::GetBoardIndex(0)] & MapT::PromotingMask;
    const uint64_t nonPromotingPawns = _board.BitBoards[MapT::GetBoardIndex(0)] ^ promotingPawns;
    const uint64_t fullMap = enemyMap | allyMap;

    _processPawnAttackPseudoMoves<MapT>(
            results, pawnAttacks, enemyMap, fullMap, promotingPawns, nonPromotingPawns
    );

    _processPawnPlainPseudoMoves<GenOnlyAttackMoves, ApplyHeuristicEval, MapT>(
        results, pawnAttacks, fullMap, promotingPawns, nonPromotingPawns
    );

    _processElPassantPseudoMoves<ApplyHeuristicEval, MapT>(results, pawnAttacks, nonPromotingPawns);
}

template<bool GenOnlyAttackMoves, bool ApplyHeuristicEval, class MapT, bool checkForCastling>
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
        if constexpr (!GenOnlyAttackMoves)
            // Don't add simple moves when we should generate only attacking moves
            _processNonAttackingMoves<MapT, ApplyHeuristicEval, false>(
                    results, pawnAttacks, nonAttackingMoves, MapT::GetBoardIndex(_board.MovingColor), figBoard,
                    updatedCastlings, fullMap
            );

        _processAttackingMoves<MapT, ApplyHeuristicEval, false>(
                results, pawnAttacks, attackMoves, MapT::GetBoardIndex(_board.MovingColor), figBoard, updatedCastlings,
                fullMap
        );

        figures ^= figBoard;
    }
}

template<bool GenOnlyAttackMoves, bool ApplyHeuristicEval>
MoveGenerator::payload MoveGenerator::GetPseudoLegalMoves(const PackedMove counterMove, const int ply, const int mostRecentMovedSquare) {
    // Init of heuristic components
    _counterMove = counterMove;
    _ply = ply;
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


    _processFigPseudoMoves<GenOnlyAttackMoves, ApplyHeuristicEval, KnightMap>(
            results, pawnAttacks, enemyMap, allyMap
    );

    _processFigPseudoMoves<GenOnlyAttackMoves, ApplyHeuristicEval, BishopMap>(
            results, pawnAttacks, enemyMap, allyMap
    );

    _processFigPseudoMoves<GenOnlyAttackMoves, ApplyHeuristicEval, RookMap, true>(
            results, pawnAttacks, enemyMap, allyMap
    );

    _processFigPseudoMoves<GenOnlyAttackMoves, ApplyHeuristicEval, QueenMap>(
            results, pawnAttacks, enemyMap, allyMap
    );

    if (_board.MovingColor == WHITE)
        _processPawnPseudoMoves<GenOnlyAttackMoves, ApplyHeuristicEval, WhitePawnMap>(
                results, pawnAttacks, enemyMap, allyMap
        );
    else
        _processPawnPseudoMoves<GenOnlyAttackMoves, ApplyHeuristicEval, BlackPawnMap>(
                results, pawnAttacks, enemyMap, allyMap
        );

    _processPlainKingMoves<GenOnlyAttackMoves, ApplyHeuristicEval, true>(
            results, KING_NO_BLOCKED_MAP, allyMap, enemyMap
    );

    if constexpr (!GenOnlyAttackMoves)
        _processKingCastlings<ApplyHeuristicEval, true>(
                results, CASTLING_PSEUDO_LEGAL_BLOCKED, enemyMap | allyMap
        );

    return results;
}
