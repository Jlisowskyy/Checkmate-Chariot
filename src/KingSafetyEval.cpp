//
// Created by Jlisowskyy on 3/26/24.
//

#include "../include/Evaluation/KingSafetyEval.h"

#include "../include/MoveGeneration/FileMap.h"
#include "../include/MoveGeneration/KingMap.h"

uint64_t KingSafetyEval::GetSafetyFields(const Board& bd, const int col)
{
    const uint64_t kingMap = bd.boards[KingMap::GetBoardIndex(col)];
    const uint64_t kingRing = KingMap::GetMoves(ExtractMsbPos(kingMap)) | kingMap;
    return kingRing;
}

int32_t KingSafetyEval::EvalKingShelter(const Board& bd) {
    int32_t eval{};
    if ((bd.boards[wKingIndex] & KingMap::ShelterLocationMask[WHITE]) != 0
        && CountOnesInBoard(bd.boards[wPawnsIndex] & GetFrontlineMask(WHITE, ExtractMsbPos(bd.boards[wKingIndex]))) < 3)
        eval += KingNoShelterPenalty;

    if ((bd.boards[bKingIndex] & KingMap::ShelterLocationMask[BLACK]) != 0
        && CountOnesInBoard(bd.boards[bPawnsIndex] & GetFrontlineMask(BLACK, ExtractMsbPos(bd.boards[bKingIndex]))) < 3)
        eval += -KingNoShelterPenalty;

    return eval;
}

int32_t KingSafetyEval::EvalKingOpenFiles(const Board& bd) {
    int32_t eval{};

    const auto wSep = FileMap::GetSepFiles(ExtractMsbPos(bd.boards[wKingIndex]));
    for(size_t i = 0 ; i < FileMap::FileSepSize; ++i)
        eval += ((bd.boards[wPawnsIndex] & wSep[i])==0)*KingOpenFilePenalty;

    const auto bSep = FileMap::GetSepFiles(ExtractMsbPos(bd.boards[bKingIndex]));
    for(size_t i = 0 ; i < FileMap::FileSepSize; ++i)
        eval -= ((bd.boards[bPawnsIndex] & bSep[i])==0)*KingOpenFilePenalty;

    return eval;
}



