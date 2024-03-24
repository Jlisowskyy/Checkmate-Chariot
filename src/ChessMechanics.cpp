//
// Created by Jlisowskyy on 12/31/23.
//

#include "../include/MoveGeneration/ChessMechanics.h"

#include "../include/MoveGeneration/BishopMap.h"
#include "../include/MoveGeneration/BlackPawnMap.h"
#include "../include/MoveGeneration/KingMap.h"
#include "../include/MoveGeneration/WhitePawnMap.h"
#include "../include/MoveGeneration/RookMap.h"

#include <cassert>

bool ChessMechanics::IsCheck() const
{
    [[maybe_unused]] const auto [unused1, checksCount, unused2] = GetBlockedFieldMap(GetFullMap());

    return checksCount > 0;
}

// Gets occupancy maps, which simply indicates wheter some field is occupied or not. Does not distinguish colors.
uint64_t ChessMechanics::GetFullMap() const
{
    uint64_t map = 0;
    for (const auto m : board.boards)
        map |= m;
    return map;
}

// Gets occupancy maps, which simply indicates wheter some field is occupied or not, by desired color figures.
uint64_t ChessMechanics::GetColMap(const int col) const
{
    assert(col == 1 || col == 0);

    uint64_t map = 0;
    for (size_t i = 0; i < Board::BoardsPerCol; ++i)
        map |= board.boards[Board::BoardsPerCol * col + i];
    return map;
}

size_t ChessMechanics::GetIndexOfContainingBoard(const uint64_t map, const int col) const
{
    assert(col == 1 || col == 0);
    assert(map != 0);

    const size_t range = Board::BoardsPerCol * col + kingIndex;
    const size_t startInd = Board::BoardsPerCol * col;

    for (size_t i = startInd; i < range; ++i)
        if ((board.boards[i] & map) != 0)
            return i;

#ifndef NDEBUG
    throw std::runtime_error(
        std::format("[ ERROR ] This code path should never be executed in {} on line {}"
                    "\n Figure not found on enemy maps!",
                    __FILE__, __LINE__));
#endif

    return 0;
}

// Todo: test later wheter accumulation of blocked maps is faster?
// Todo: AVX applied here?

/*      IMPORTANT NOTES:
 *  BlockedFieldsMap - indicates wheter some field could be attacked by enemy figures in their next round.
 *  During generation process there are check test performed with counting, what yields 3 code branches inside main
 *  generation code. Map is mainly used when king is moving, allowing to simply predict wheter king should
 *  move to that tile or not.
 *
 */

std::tuple<uint64_t, uint8_t, uint8_t> ChessMechanics::GetBlockedFieldMap(const uint64_t fullMap) const
{
    assert(fullMap != 0);

    uint8_t checksCount{};
    uint8_t chT{};

    const int enemyCol = SwapColor(board.movColor);
    const size_t enemyFigInd = enemyCol * Board::BoardsPerCol;
    const int allyKingShift = ConvertToReversedPos(board.GetKingMsbPos(board.movColor));
    const uint64_t allyKingMap = 1LLU << allyKingShift;

    // allows to also simply predict which tiles on the other side of the king are allowed.
    const uint64_t fullMapWoutKing = fullMap ^ allyKingMap;

    // King attacks generation.
    const uint64_t kingBlockedMap = KingMap::GetMoves(board.GetKingMsbPos(enemyCol));

    // Rook attacks generation. Needs special treatment to correctly detect double check, especially with pawn promotion
    const auto [rookBlockedMap, checkCountRook] =
        _getRookBlockedMap(board.boards[enemyFigInd + rooksIndex] | board.boards[enemyFigInd + queensIndex], fullMapWoutKing, allyKingMap);

    // = 0, 1 or eventually 2 when promotion to rook like type happens
    checksCount += checkCountRook;

    // Bishop attacks generation.
    const uint64_t bishopBlockedMap =
        _blockIterativeGenerator(board.boards[enemyFigInd + bishopsIndex] | board.boards[enemyFigInd + queensIndex],
                                 [=](const int pos) { return BishopMap::GetMoves(pos, fullMapWoutKing); });

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByBishopFlag = (bishopBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByBishopFlag;

    // Pawns attacks generation.
    const uint64_t pawnsMap = board.boards[enemyFigInd + pawnsIndex];
    const uint64_t pawnBlockedMap =
        enemyCol == WHITE ? WhitePawnMap::GetAttackFields(pawnsMap) : BlackPawnMap::GetAttackFields(pawnsMap);

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByPawnFlag = (pawnBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByPawnFlag;

    // modyfing check type
    chT += simpleFigCheck * wasCheckedByPawnFlag;  // Note: king cannot be double checked by simple figure

    // Knight attacks generation.
    const uint64_t knighBlockedMap = _blockIterativeGenerator(board.boards[enemyFigInd + knightsIndex],
                                                              [=](const int pos) { return KnightMap::GetMoves(pos); });

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByKnightFlag = (knighBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByKnightFlag;

    // modyfing check type
    chT += simpleFigCheck * wasCheckedByKnightFlag;  // Note: king cannot be double checked by simple figure

    const uint64_t blockedMap = kingBlockedMap | pawnBlockedMap | knighBlockedMap | rookBlockedMap | bishopBlockedMap;
    return {blockedMap, checksCount, chT};
}

uint64_t ChessMechanics::GetAllowedTilesWhenCheckedByNonSliding() const
{
    uint64_t allowedTiles{};

    allowedTiles |= KingMap::GetSimpleFigCheckKnightsAllowedTiles(board);
    allowedTiles |= KingMap::GetSimpleFigCheckPawnAllowedTiles(board);

    return allowedTiles;
}

std::pair<uint64_t, uint8_t> ChessMechanics::_getRookBlockedMap(uint64_t rookMap, const uint64_t fullMapWoutKing,
    const uint64_t kingMap)
{
    assert(kingMap != 0);

    uint64_t blockedTiles{};
    uint8_t checks{};

    while(rookMap)
    {
        const int msbPos = ExtractMsbPos(rookMap);

        const uint64_t moves = RookMap::GetMoves(msbPos, fullMapWoutKing);
        blockedTiles |= moves;
        checks += ((moves & kingMap) != 0);

        rookMap ^= maxMsbPossible >> msbPos;
    }

    return {blockedTiles, checks};
}

// TODO: pretty fking stupid but ok could be optimised after tests:
uint64_t ChessMechanics::GenerateAllowedTilesForPrecisedPinnedFig(const uint64_t figBoard, const uint64_t fullMap) const
{
    assert(fullMap != 0);
    assert(figBoard != 0);

    constexpr size_t bishopRange = PinningMasks::BishopLines + PinningMasks::PinningMaskPerLinesType;
    for (size_t i = PinningMasks::BishopLines; i < bishopRange; ++i)
        if (const uint64_t mask = KingMap::pinMasks[board.GetKingMsbPos(board.movColor)].masks[i];
            (figBoard & mask) != 0)
        {
            return BishopMap::GetMoves(board.GetKingMsbPos(board.movColor), fullMap ^ figBoard) & mask;
        }

    constexpr size_t rookRange = PinningMasks::RookLines + PinningMasks::PinningMaskPerLinesType;
    for (size_t i = PinningMasks::RookLines; i < rookRange; ++i)
        if (const uint64_t mask = KingMap::pinMasks[board.GetKingMsbPos(board.movColor)].masks[i];
            (figBoard & mask) != 0)
        {
            return RookMap::GetMoves(board.GetKingMsbPos(board.movColor), fullMap ^ figBoard) & mask;
        }

#ifndef NDEBUG
    throw std::runtime_error("Fatal error occured during allowed tiles generation!\n");
#endif

    return 0;
}
