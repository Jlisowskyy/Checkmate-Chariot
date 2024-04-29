//
// Created by Jlisowskyy on 4/29/24.
//

#include "../include/Evaluation/HistoricTable.h"

void HistoricTable::SetBonusMove(const Move mv, const int depth)
{
    _table[mv.GetStartBoardIndex()][mv.GetTargetField()] =
        static_cast<int16_t>(std::min(_pointScale(_table[mv.GetStartBoardIndex()][mv.GetTargetField()], depth), Barrier));
}
void HistoricTable::ClearTable()
{
    for (auto & table : _table)
        std::fill_n(table, Board::BitBoardFields, 0);
}
int32_t HistoricTable::GetBonusMove(const Move mv) const { return _table[mv.GetStartBoardIndex()][mv.GetTargetField()]; }
void HistoricTable::ScaleTableDown()
{
    for (auto &figureMap : _table)
        for (auto &field : figureMap) field /= ScaleFactor;
}
