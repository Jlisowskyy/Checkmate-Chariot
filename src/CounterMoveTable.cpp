//
// Created by Jlisowskyy on 4/29/24.
//

#include "../include/Evaluation/CounterMoveTable.h"

void CounterMoveTable::ClearTable()
{
    for (auto& color : _counterMovesTable)
        for (auto &from : color)
            for (auto &fromTo : from) fromTo = PackedMove{};
}
