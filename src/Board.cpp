//
// Created by Jlisowskyy on 3/4/24.
//

#include "../include/Board.h"

#include <format>
#include <iostream>

bool Board::Comp(const Board &a, const Board &b)
{
    for (int i = 0; i < 12; ++i)
        if (a.boards[i] != b.boards[i])
        {
            std::cout << std::format("Invalid board: {}\n", i);
            return false;
        }

    for (int i = 0; i < 4; ++i)
        if (a.Castlings[i] != b.Castlings[i])
            return false;

    if (a.elPassantField != b.elPassantField)
        return false;

    if (a.movColor != b.movColor)
        return false;

    return true;
}
