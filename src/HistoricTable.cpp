//
// Created by Jlisowskyy on 4/29/24.
//

#include "../include/Evaluation/HistoricTable.h"

void HistoricTable::ClearTable()
{
    for (auto & table : _table)
        std::fill_n(table, Board::BitBoardFields, 0);
}

void HistoricTable::ScaleTableDown()
{
    for (auto &figureMap : _table)
        for (auto &field : figureMap) field /= ScaleFactor;
}
