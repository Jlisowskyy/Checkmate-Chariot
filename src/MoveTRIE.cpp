//
// Created by Jlisowskyy on 2/26/24.
//

#include "../include/OpeningBook/MoveTRIE.h"

// "d2d4 g8f6 c2c4 c7c5 d4d5 e7e6 b1c3 e6d5 c4d5 d7d6 g1f3 g7g6 g2g3 f8g7 f1g2 e8g8 e1g1 b8d7 f3d2 a7a6"
void MoveTRIE::AddMoveSequence(const std::vector<std::string>& moves)
{
    node* root = _root;

    for (const auto& move : moves)
    {
        // searching for move on array
        const ssize_t nextInd = _findOnArr(move, root->_moves);

        // if not found add move - not the most optimal soultion but correct one
        if (nextInd == -1)
        {
            root->_moves.push_back(move);
            root->_next.push_back(new node());
            root = root->_next[root->_next.size() - 1];
        }
        else
            root = root->_next[nextInd];
    }
}

void MoveTRIE::OptimiseMemoryUsage() { _optMem(_root); }

const std::vector<std::string>& MoveTRIE::FindNextMoves(const std::vector<std::string>& moves) const
{
    static constexpr std::vector<std::string> _emptVect{};
    const node* root = _root;

    for (const auto& move : moves)
    {
        const ssize_t nextInd = _findOnArr(move, root->_moves);
        if (nextInd == -1)
            return _emptVect;
        root = root->_next[nextInd];
    }

    return root->_moves;
}

ssize_t MoveTRIE::_findOnArr(const std::string& move, const std::vector<std::string>& moves)
{
    for (ssize_t i = 0; i < static_cast<ssize_t>(moves.size()); ++i)
        if (moves[i] == move)
            return i;
    return -1;
}

void MoveTRIE::_destroy(const node* root)
{
    if (!root)
        return;

    for (const auto& n : root->_next)
        _destroy(n);

    delete root;
}

void MoveTRIE::_optMem(node* root)
{
    if (!root)
        return;

    root->_moves.shrink_to_fit();
    root->_next.shrink_to_fit();

    for (const auto n : root->_next)
        _optMem(n);
}
