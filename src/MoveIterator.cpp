//
// Created by Jlisowskyy on 7/13/24.
//

#include "../include/Search/MoveIterator.h"

MoveIterator::MoveIterator(const Board &board, MoveGenerator &generator, MoveGenerator::payload moves):
    _board(board), _generator(generator), _moves(moves)
{

}