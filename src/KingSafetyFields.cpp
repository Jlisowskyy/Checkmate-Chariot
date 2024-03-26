//
// Created by Jlisowskyy on 3/26/24.
//

#include "../include/Evaluation/KingSafetyFields.h"

#include "../include/MoveGeneration/KingMap.h"

uint64_t KingSafetyFields::GetSafetyFields(const Board& bd, const int col)
{
    const uint64_t kingMap = bd.boards[KingMap::GetBoardIndex(col)];
    const uint64_t kingRing = KingMap::GetMoves(ExtractMsbPos(kingMap)) | kingMap;
    return kingRing;
}

