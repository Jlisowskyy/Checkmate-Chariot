//
// Created by Jlisowskyy on 4/29/24.
//

#include "../include/Evaluation/MoveSortEval.h"
int32_t MoveSortEval::ApplyAttackFieldEffects(
    int32_t eval, const uint64_t pawnAttacks, const uint64_t startField, const uint64_t targetField
)
{
    if ((pawnAttacks & startField) != 0)
        eval += RunAwayPrize;
    if ((pawnAttacks & targetField) != 0)
        eval += AttackedFigurePenalty;

    return eval;
}
int32_t MoveSortEval::ApplyPromotionEffects(const int32_t eval, const size_t nFig)
{
    return FigureEval[nFig] + eval + PromotionBonus;
}
int32_t MoveSortEval::ApplyKilledFigEffect(const int32_t eval, const size_t attackFig, const size_t killedFig)
{
    return eval + FigureEval[killedFig] - FigureEval[attackFig] + CaptureBonus;
}
int32_t
MoveSortEval::ApplyKillerMoveEffect(const int32_t eval, const KillerTable &kTable, const Move mv, const int depthLeft)
{
    return eval + KillerMovePrize * kTable.IsKillerMove(mv, depthLeft);
}
int32_t MoveSortEval::ApplyCounterMoveEffect(const int32_t eval, const PackedMove counterMove, const Move move)
{
    return eval + CounterMovePrize * (move.GetPackedMove() == counterMove);
}
int32_t MoveSortEval::ApplyCaptureMostRecentSquareEffect(
    const int32_t eval, const int mostRecentSquareMsb, const int moveSquare
)
{
    return eval + MostRecentSquarePrize * (mostRecentSquareMsb == moveSquare);
}
int32_t MoveSortEval::ApplyHistoryTableBonus(const int32_t eval, const Move mv, const HistoricTable &hTable)
{
    return eval + hTable.GetBonusMove(mv);
}
