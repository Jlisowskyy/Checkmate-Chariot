//
// Created by Jlisowskyy on 12/31/23.
//

#include "../include/MoveGeneration/ChessMechanics.h"

uint64_t ChessMechanics::GetFullMap() const
{
    uint64_t map = 0;
    for (const auto m: board.boards) map |= m;
    return map;
}

uint64_t ChessMechanics::GetColMap(const int col) const
{
    uint64_t map = 0;
    for (size_t i = 0; i < Board::BoardsPerCol; ++i) map |= board.boards[Board::BoardsPerCol * col + i];
    return map;
}

// Todo: test later wheter accumulation of blocked maps is faster?
// Todo: AVX applied here?
std::tuple<uint64_t, uint8_t, uint8_t> ChessMechanics::GetBlockedFieldMap(const uint64_t fullMap) const
{
    uint8_t checksCount{};
    uint8_t chT{};

    const int enemyCol = SwapColor(board.movColor);
    const size_t enemyFigInd = enemyCol * Board::BoardsPerCol;
    const int allyKingShift = ConvertToReversedPos(board.kingMSBPositions[board.movColor]);
    const uint64_t allyKingMap = 1LLU << allyKingShift;

    // King attacks generation.
    const uint64_t kingBlockedMap = KingMap::GetMoves(board.kingMSBPositions[enemyCol]);

    // Pawns attacks generation.
    const uint64_t pawnsMap = board.boards[enemyFigInd + pawnsIndex];
    const uint64_t pawnBlockedMap = enemyCol == WHITE
                                        ? WhitePawnMap::GetAttackFields(pawnsMap)
                                        : BlackPawnMap::GetAttackFields(pawnsMap);
    const uint8_t wasCheckedByPawnFlag = (pawnBlockedMap & allyKingMap) >> allyKingShift;
    // = 1 or 0 depending whether hits or not
    checksCount += wasCheckedByPawnFlag;
    chT = simpleFigCheck * wasCheckedByPawnFlag;

    // Knight attacks generation.
    const uint64_t knighBlockedMap = _blockIterativeGenerator(board.boards[enemyFigInd + knightsIndex],
                                                              [=](const int pos) { return KnightMap::GetMoves(pos); }
    );
    const uint8_t wasCheckedByKnightFlag = (knighBlockedMap & allyKingMap) >> allyKingShift;
    // = 1 or 0 depending whether hits or not
    checksCount += wasCheckedByKnightFlag;
    chT = simpleFigCheck * wasCheckedByKnightFlag;

    // Rook attacks generation.
    const uint64_t rookBlockedMap = _blockIterativeGenerator(
        board.boards[enemyFigInd + rooksIndex] | board.boards[enemyFigInd + queensIndex],
        [=](const int pos) { return RookMap::GetMoves(pos, fullMap); }
    );
    const uint8_t wasCheckedByRookFlag = (rookBlockedMap & allyKingMap) >> allyKingShift;
    // = 1 or 0 depending whether hits or not
    checksCount += wasCheckedByRookFlag;
    chT = slidingFigCheck * wasCheckedByRookFlag;

    // Bishop attacks generation.
    const uint64_t bishopBlockedMap = _blockIterativeGenerator(
        board.boards[enemyFigInd + bishopsIndex] | board.boards[enemyFigInd + queensIndex],
        [=](const int pos) { return BishopMap::GetMoves(pos, fullMap); }
    );
    const uint8_t wasCheckedByBishopFlag = (bishopBlockedMap & allyKingMap) >> allyKingShift;
    // = 1 or 0 depending whether hits or not
    checksCount += wasCheckedByBishopFlag;
    chT = slidingFigCheck * wasCheckedByBishopFlag;

    const uint64_t blockedMap = kingBlockedMap | pawnBlockedMap | knighBlockedMap | rookBlockedMap | bishopBlockedMap;
    return {blockedMap, checksCount, chT};
}

// TODO: here pinned figs could be processed first to get some slight speed up
uint64_t ChessMechanics::GetPinnedFigsMapWoutCheck(const int col, const uint64_t fullMap) const
{
    const size_t enemyCord = SwapColor(col) * Board::BoardsPerCol;
    const size_t allyCord = col * Board::BoardsPerCol;
    const int allyKingShift = ConvertToReversedPos(board.kingMSBPositions[col]);

    const uint64_t rookLinesOnKing = KingMap::pinMasks[board.kingMSBPositions[col]].rookMask;
    const uint64_t suspectedRooks = (board.boards[enemyCord + rooksIndex] | board.boards[enemyCord + queensIndex]) &
                                    rookLinesOnKing;
    const uint64_t figsPinnedByRookMoves = _getPinnedFigsWoutCheckGenerator<RookMap>(
        suspectedRooks, fullMap, rookLinesOnKing, allyCord, allyKingShift);

    const uint64_t bishopLinesOnKing = KingMap::pinMasks[board.kingMSBPositions[col]].bishopMask;
    const uint64_t suspectedBishops = (board.boards[enemyCord + bishopsIndex] | board.boards[enemyCord + queensIndex]) &
                                      bishopLinesOnKing;
    const uint64_t figsPinnedByBishopMoves = _getPinnedFigsWoutCheckGenerator<BishopMap>(
        suspectedBishops, fullMap, bishopLinesOnKing, allyCord, allyKingShift);

    return figsPinnedByRookMoves | figsPinnedByBishopMoves;
}

// Todo: check wheter interrupting search when checking figure is found boosts up performance here
std::pair<uint64_t, uint64_t> ChessMechanics::GetPinnedFigsMapWithCheck(const int col, const uint64_t fullMap) const
{
    const size_t enemyCord = SwapColor(col) * Board::BoardsPerCol;
    const size_t allyCord = col * Board::BoardsPerCol;
    const int allyKingShift = ConvertToReversedPos(board.kingMSBPositions[col]);

    // Rook lines search
    const uint64_t rookLinesOnKing = KingMap::pinMasks[board.kingMSBPositions[col]].rookMask;
    const uint64_t suspectedRooks = (board.boards[enemyCord + rooksIndex] | board.boards[enemyCord + queensIndex]) &
                                    rookLinesOnKing;
    const auto [figsPinnedByRookMoves, allowedTileRook] =
            _getPinnedFigsWithCheckGenerator<
                RookMap>(suspectedRooks, fullMap, rookLinesOnKing, allyCord, allyKingShift);

    // TODO: what I meant: if (!allowedTileRook) { ... }
    // Bishop lines search
    const uint64_t bishopLinesOnKing = KingMap::pinMasks[board.kingMSBPositions[col]].bishopMask;
    const uint64_t suspectedBishops = (board.boards[enemyCord + bishopsIndex] | board.boards[enemyCord + queensIndex]) &
                                      bishopLinesOnKing;
    const auto [figsPinnedByBishopMoves, allowedTileBishop] =
            _getPinnedFigsWithCheckGenerator<BishopMap>(suspectedBishops, fullMap, bishopLinesOnKing, allyCord,
                                                        allyKingShift);

    return {figsPinnedByRookMoves | figsPinnedByBishopMoves, allowedTileRook | allowedTileBishop};
}

uint64_t ChessMechanics::GetAllowedTilesWhenCheckedByNonSliding(const int col) const
{
    static constexpr int pawnDetectionOffsets[][2]{
        {7, 9},
        {-7, -9}
    };

    // preparing helping variables
    const size_t enemyInd = SwapColor(board.movColor) * Board::BoardsPerCol;
    const uint64_t allyKingPos = board.kingMSBPositions[col];
    const uint64_t allyKing = maxMsbPossible >> allyKingPos;

    uint64_t allowedTiles{};

    // Knights iteration
    uint64_t knightFigures = board.boards[enemyInd + knightsIndex];
    while (knightFigures)
    {
        // another helping variables
        const int msbPos = ExtractMsbPos(knightFigures);
        const uint64_t knightFig = maxMsbPossible >> msbPos;
        const uint64_t attacks = KnightMap::GetMoves(msbPos);

        // main processing
        allowedTiles |= (attacks & allyKing) * knightFig;

        // iteration step
        knightFigures ^= knightFig;
    }

    const uint64_t pawnDetectionFields = (maxMsbPossible >> (allyKingPos + pawnDetectionOffsets[col][0])) |
                                         (maxMsbPossible >> (allyKingPos + pawnDetectionOffsets[col][1]));
    allowedTiles |= pawnDetectionFields & board.boards[enemyInd + pawnsIndex];

    return allowedTiles;
}

std::vector<Board> ChessMechanics::GetPossibleMoveSlow()
{
    static constexpr size_t averageMoveCount = 40;

    std::vector<Board> moveVect{};
    moveVect.reserve(averageMoveCount);

    IterativeBoardTraversal(
        [&](Board&board)
        {
            moveVect.emplace_back(board);
        },
        1
    );

    return moveVect;
}

// TODO: pretty fking stupid but ok could be optimised after tests:
uint64_t ChessMechanics::_generateAllowedTilesForPrecisedPinnedFig(const uint64_t figBoard,
                                                                   const uint64_t fullMap) const
{
    constexpr size_t bishopRange = PinningMasks::BishopLines + PinningMasks::PinningMaskPerLinesType;
    for (size_t i = PinningMasks::BishopLines; i < bishopRange; ++i)
        if (const uint64_t mask = KingMap::pinMasks[board.kingMSBPositions[board.movColor]].masks[i];
            (figBoard & mask) != 0)
        {
            return BishopMap::GetMoves(board.kingMSBPositions[board.movColor], fullMap ^ figBoard) & mask;
        }

    constexpr size_t rookRange = PinningMasks::RookLines + PinningMasks::PinningMaskPerLinesType;
    for (size_t i = PinningMasks::RookLines; i < rookRange; ++i)
        if (const uint64_t mask = KingMap::pinMasks[board.kingMSBPositions[board.movColor]].masks[i];
            (figBoard & mask) != 0)
        {
            return RookMap::GetMoves(board.kingMSBPositions[board.movColor], fullMap ^ figBoard) & mask;
        }

#ifndef NDEBUG
    throw std::runtime_error("Fatal error occured during allowed tiles generation!\n");
#endif

    return 0;
}
