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
        CountOnesInBoard(bd.BitBoards[wPawnsIndex] & GetFrontLineMask(WHITE, ExtractMsbPos(bd.BitBoards[wKingIndex]))) <
            3)
        eval += KingNoShelterPenalty;

    if ((bd.BitBoards[bKingIndex] & KingMap::ShelterLocationMask[BLACK]) != 0 &&
        CountOnesInBoard(bd.BitBoards[bPawnsIndex] & GetFrontLineMask(BLACK, ExtractMsbPos(bd.BitBoards[bKingIndex]))) <
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
void __attribute__((always_inline)) KingSafetyEval::UpdateKingAttacks(
    KingSafetyEval::_kingSafetyInfo_t &info, const uint64_t attacks, const uint64_t kingRing, const int32_t pointsPerAttack
)
{
    const int32_t kingAttackingCount = CountOnesInBoard(attacks & kingRing);

    info.attackCounts += kingAttackingCount > 0;
    info.attackPoints += kingAttackingCount * pointsPerAttack;
}
uint64_t KingSafetyEval::GetFrontLineMask(const int col, const int msbPos) { return _kingPawnDefenseFields[col][msbPos]; }
int32_t KingSafetyEval::ScoreKingRingControl(
    const KingSafetyEval::_kingSafetyInfo_t &whiteInfo, const KingSafetyEval::_kingSafetyInfo_t &blackInfo
)
{
    int32_t bonus {};

    bonus += (whiteInfo.attackCounts > 2) * (-_kingSafetyValues[whiteInfo.attackPoints]);
    bonus += (blackInfo.attackCounts > 2) * (_kingSafetyValues[blackInfo.attackPoints]);

    return bonus;
}
