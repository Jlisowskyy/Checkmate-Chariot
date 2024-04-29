//
// Created by Jlisowskyy on 3/26/24.
//

#include "../include/Evaluation/KingSafetyEval.h"

#include "../include/MoveGeneration/FileMap.h"
#include "../include/MoveGeneration/KingMap.h"

uint64_t KingSafetyEval::GetSafetyFields(const Board &bd, const int col)
{
    const uint64_t kingMap  = bd.BitBoards[KingMap::GetBoardIndex(col)];
    const uint64_t kingRing = KingMap::GetMoves(ExtractMsbPos(kingMap)) | kingMap;
    return kingRing;
}

int32_t KingSafetyEval::EvalKingShelter(const Board &bd)
{
    int32_t eval{};
    if ((bd.BitBoards[wKingIndex] & KingMap::ShelterLocationMask[WHITE]) != 0 &&
        CountOnesInBoard(bd.BitBoards[wPawnsIndex] & GetFrontlineMask(WHITE, ExtractMsbPos(bd.BitBoards[wKingIndex]))) <
            3)
        eval += KingNoShelterPenalty;

    if ((bd.BitBoards[bKingIndex] & KingMap::ShelterLocationMask[BLACK]) != 0 &&
        CountOnesInBoard(bd.BitBoards[bPawnsIndex] & GetFrontlineMask(BLACK, ExtractMsbPos(bd.BitBoards[bKingIndex]))) <
            3)
        eval += -KingNoShelterPenalty;

    return eval;
}

int32_t KingSafetyEval::EvalKingOpenFiles(const Board &bd)
{
    int32_t eval{};

    const auto wSep = FileMap::GetSepFiles(ExtractMsbPos(bd.BitBoards[wKingIndex]));
    for (size_t i = 0; i < FileMap::FileSepSize; ++i)
        eval += ((bd.BitBoards[wPawnsIndex] & wSep[i]) == 0) * KingOpenFilePenalty;

    const auto bSep = FileMap::GetSepFiles(ExtractMsbPos(bd.BitBoards[bKingIndex]));
    for (size_t i = 0; i < FileMap::FileSepSize; ++i)
        eval -= ((bd.BitBoards[bPawnsIndex] & bSep[i]) == 0) * KingOpenFilePenalty;

    return eval;
}
