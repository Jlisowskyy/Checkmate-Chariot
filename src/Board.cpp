//
// Created by Jlisowskyy on 3/4/24.
//

#include "../include/Board.h"
#include "../include/Interface/Logger.h"

#include <format>

bool Board::Comp(const Board &a, const Board &b)
{
    for (int i = 0; i < 12; ++i)
        if (a.BitBoards[i] != b.BitBoards[i])
        {
            GlobalLogger.LogStream << std::format("Invalid board: {}\n", i);
            return false;
        }

    for (int i = 0; i < 4; ++i)
        if (a.Castlings[i] != b.Castlings[i])
            return false;

    if (a.ElPassantField != b.ElPassantField)
        return false;

    if (a.MovingColor != b.MovingColor)
        return false;

    return true;
}
