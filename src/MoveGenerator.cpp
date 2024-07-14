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

    auto moves = GetMovesSlow<false>();
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

    MoveGenerator mgen{bd, _threadStack};
    const auto moves = mgen.GetMovesSlow<false>();

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

bool MoveGenerator::IsLegal(Board& bd, const Move mv) {
    // TODO: Possibility to improve performance by introducing global state holding pinned figures
    // TODO: and distinguish given states:
    // TODO: Castling -> (check for attacks on fields),
    // TODO: King Move / El Passant -> (check if king is attacked from king's perspective),
    // TODO: Rest of moves allowed only on line between pinned and king if pinned

    return mv.GetPackedMove().IsCastling() ?
        MoveGenerator::_isCastlingLegal(bd, mv) :
        MoveGenerator::_isNormalMoveLegal(bd, mv);
}

bool MoveGenerator::_isCastlingLegal(Board& bd, Move mv)
{
    ChessMechanics mech(bd);
    const auto [blocked, unused, unused1] =
            mech.GetBlockedFieldBitMap(mech.GetFullBitMap());

    // Refer to castling generating function in MoveGenerator, there is sentinel on index 0 somehow

    // Check if the king is not attacked and castling sensitive fields too
    return (Board::CastlingSensitiveFields[mv.GetCastlingType() - 1] & blocked) == 0 &&
            (blocked & bd.GetFigBoard(bd.MovingColor, kingIndex)) == 0;
}

bool MoveGenerator::_isNormalMoveLegal(Board& bd, Move mv)
{
    bd.BitBoards[mv.GetStartBoardIndex()] ^= MaxMsbPossible >> mv.GetStartField();
    bd.BitBoards[mv.GetTargetBoardIndex()] |= MaxMsbPossible >> mv.GetTargetField();
    bd.BitBoards[mv.GetKilledBoardIndex()] ^= MaxMsbPossible >> mv.GetKilledFigureField();

    auto reverse = [&](){
        bd.BitBoards[mv.GetStartBoardIndex()] |= MaxMsbPossible >> mv.GetStartField();
        bd.BitBoards[mv.GetTargetBoardIndex()] ^= MaxMsbPossible >> mv.GetTargetField();
        bd.BitBoards[mv.GetKilledBoardIndex()] |= MaxMsbPossible >> mv.GetKilledFigureField();
    };

    ChessMechanics mechanics(bd);

    const int enemyCol       = SwapColor(bd.MovingColor);
    const int kingsMsb       = bd.GetKingMsbPos(bd.MovingColor);
    const uint64_t fullBoard = mechanics.GetFullBitMap();

    // Checking rook's perspective
    const uint64_t enemyRooks  = bd.GetFigBoard(enemyCol, rooksIndex);
    const uint64_t enemyQueens = bd.GetFigBoard(enemyCol, queensIndex);

    const uint64_t kingsRookPerspective = RookMap::GetMoves(kingsMsb, fullBoard);

    if ((kingsRookPerspective & (enemyRooks | enemyQueens)) != 0)
        return (reverse(), false);

    // Checking bishop's perspective
    const uint64_t enemyBishops = bd.GetFigBoard(enemyCol, bishopsIndex);
    const uint64_t kingsBishopPerspective = BishopMap::GetMoves(kingsMsb, fullBoard);

    if ((kingsBishopPerspective & (enemyBishops | enemyQueens)) != 0)
        return (reverse(), false);

    // checking knights attacks
    const uint64_t enemyKnights = bd.GetFigBoard(enemyCol, knightsIndex);
    const uint64_t knightsPerspective = KnightMap::GetMoves(kingsMsb);

    if ((knightsPerspective & (enemyKnights)) != 0)
        return (reverse(), false);

    // pawns checks
    const uint64_t enemyPawns = bd.GetFigBoard(enemyCol, pawnsIndex);
    const uint64_t pawnAttacks =
            enemyCol == WHITE ? WhitePawnMap::GetAttackFields(enemyPawns) : BlackPawnMap::GetAttackFields(enemyPawns);

    if ((pawnAttacks & (MaxMsbPossible >> kingsMsb)) != 0)
        return (reverse(), false);

    return (reverse(), true);
}

