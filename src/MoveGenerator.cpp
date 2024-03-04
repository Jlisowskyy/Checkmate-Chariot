//
// Created by Jlisowskyy on 3/4/24.
//

#include "../include/MoveGeneration/MoveGenerator.h"

bool MoveGenerator::IsCheck() const
{
    [[maybe_unused]] const auto [unused1, checksCount, unused2] = GetBlockedFieldMap(GetFullMap());

    return checksCount > 0;
}

// Gets occupancy maps, which simply indicates wheter some field is occupied or not. Does not distinguish colors.
uint64_t MoveGenerator::GetFullMap() const
{
    uint64_t map = 0;
    for (const auto m: board.boards) map |= m;
    return map;
}

// Gets occupancy maps, which simply indicates wheter some field is occupied or not, by desired color figures.
uint64_t MoveGenerator::GetColMap(const int col) const
{
    uint64_t map = 0;
    for (size_t i = 0; i < Board::BoardsPerCol; ++i) map |= board.boards[Board::BoardsPerCol * col + i];
    return map;
}

size_t MoveGenerator::GetIndexOfContainingBoard(const uint64_t map, const int col) const
{
    const size_t range = Board::BoardsPerCol * col + kingIndex;
    const size_t startInd = Board::BoardsPerCol * col;

    for (size_t i = startInd; i < range; ++i)
        if ((board.boards[i] & map) != 0) return i;

#ifndef NDEBUG
    throw std::runtime_error(std::format("[ ERROR ] This code path should never be executed in {} on line {}"
                                         "\n Figure not found on enemy maps!", __FILE__, __LINE__));
#endif

    return 0;
}

std::tuple<uint64_t, uint8_t, uint8_t> MoveGenerator::GetBlockedFieldMap(const uint64_t fullMap) const
{
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

    // Rook attacks generation.
    const uint64_t rookBlockedMap = _blockIterativeGenerator(
        board.boards[enemyFigInd + rooksIndex] | board.boards[enemyFigInd + queensIndex],
        [=](const int pos) { return RookMap::GetMoves(pos, fullMapWoutKing); }
    );

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByRookFlag = (rookBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByRookFlag;

    // Bishop attacks generation.
    const uint64_t bishopBlockedMap = _blockIterativeGenerator(
        board.boards[enemyFigInd + bishopsIndex] | board.boards[enemyFigInd + queensIndex],
        [=](const int pos) { return BishopMap::GetMoves(pos, fullMapWoutKing); }
    );

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByBishopFlag = (bishopBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByBishopFlag;

    // Pawns attacks generation.
    const uint64_t pawnsMap = board.boards[enemyFigInd + pawnsIndex];
    const uint64_t pawnBlockedMap = enemyCol == WHITE
                                        ? WhitePawnMap::GetAttackFields(pawnsMap)
                                        : BlackPawnMap::GetAttackFields(pawnsMap);

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByPawnFlag = (pawnBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByPawnFlag;

    // modyfing check type
    chT += simpleFigCheck * wasCheckedByPawnFlag; // Note: king cannot be double checked by simple figure

    // Knight attacks generation.
    const uint64_t knighBlockedMap = _blockIterativeGenerator(board.boards[enemyFigInd + knightsIndex],
                                                              [=](const int pos) { return KnightMap::GetMoves(pos); }
    );

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByKnightFlag = (knighBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByKnightFlag;

    // modyfing check type
    chT += simpleFigCheck * wasCheckedByKnightFlag; // Note: king cannot be double checked by simple figure

    const uint64_t blockedMap = kingBlockedMap | pawnBlockedMap | knighBlockedMap | rookBlockedMap | bishopBlockedMap;
    return {blockedMap, checksCount, chT};
}

// TODO: here pinned figs could be processed first to get some slight speed up
uint64_t MoveGenerator::GetPinnedFigsMapWoutCheck(const int col, const uint64_t fullMap) const
{
    const size_t enemyCord = SwapColor(col) * Board::BoardsPerCol;
    const size_t allyCord = col * Board::BoardsPerCol;
    const int allyKingShift = ConvertToReversedPos(board.GetKingMsbPos(col));

    const uint64_t rookLinesOnKing = KingMap::pinMasks[board.GetKingMsbPos(col)].rookMask;
    const uint64_t suspectedRooks = (board.boards[enemyCord + rooksIndex] | board.boards[enemyCord + queensIndex]) &
                                    rookLinesOnKing;
    const uint64_t figsPinnedByRookMoves = _getPinnedFigsWoutCheckGenerator<RookMap>(
        suspectedRooks, fullMap, allyCord, allyKingShift);

    const uint64_t bishopLinesOnKing = KingMap::pinMasks[board.GetKingMsbPos(col)].bishopMask;
    const uint64_t suspectedBishops = (board.boards[enemyCord + bishopsIndex] | board.boards[enemyCord + queensIndex]) &
                                      bishopLinesOnKing;
    const uint64_t figsPinnedByBishopMoves = _getPinnedFigsWoutCheckGenerator<BishopMap>(
        suspectedBishops, fullMap, allyCord, allyKingShift);

    return figsPinnedByRookMoves | figsPinnedByBishopMoves;
}

// Todo: check wheter interrupting search when checking figure is found boosts up performance here
std::pair<uint64_t, uint64_t> MoveGenerator::GetPinnedFigsMapWithCheck(const int col, const uint64_t fullMap) const
{
    const size_t enemyCord = SwapColor(col) * Board::BoardsPerCol;
    const size_t allyCord = col * Board::BoardsPerCol;
    const int allyKingShift = ConvertToReversedPos(board.GetKingMsbPos(col));

    // Rook lines search
    const uint64_t rookLinesOnKing = KingMap::pinMasks[board.GetKingMsbPos(col)].rookMask;
    const uint64_t suspectedRooks = (board.boards[enemyCord + rooksIndex] | board.boards[enemyCord + queensIndex]) &
                                    rookLinesOnKing;
    const auto [figsPinnedByRookMoves, allowedTileRook] =
            _getPinnedFigsWithCheckGenerator<RookMap>(suspectedRooks, fullMap, allyCord, allyKingShift);

    // TODO: what I meant: if (!allowedTileRook) { ... }
    // Bishop lines search
    const uint64_t bishopLinesOnKing = KingMap::pinMasks[board.GetKingMsbPos(col)].bishopMask;
    const uint64_t suspectedBishops = (board.boards[enemyCord + bishopsIndex] | board.boards[enemyCord + queensIndex]) &
                                      bishopLinesOnKing;
    const auto [figsPinnedByBishopMoves, allowedTileBishop] =
            _getPinnedFigsWithCheckGenerator<BishopMap>(suspectedBishops, fullMap, allyCord, allyKingShift);

    return {figsPinnedByRookMoves | figsPinnedByBishopMoves, allowedTileRook | allowedTileBishop};
}

uint64_t MoveGenerator::GetAllowedTilesWhenCheckedByNonSliding() const
{
    uint64_t allowedTiles{};

    allowedTiles |= KingMap::GetSimpleFigCheckKnightsAllowedTiles(board);
    allowedTiles |= KingMap::GetSimpleFigCheckPawnAllowedTiles(board);

    return allowedTiles;
}

// TODO: pretty fking stupid but ok could be optimised after tests:
uint64_t MoveGenerator::_generateAllowedTilesForPrecisedPinnedFig(const uint64_t figBoard,
                                                                   const uint64_t fullMap) const
{
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
