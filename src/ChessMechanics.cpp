//
// Created by Jlisowskyy on 12/31/23.
//

#include "../include/MoveGeneration/ChessMechanics.h"

#include "../include/MoveGeneration/BishopMap.h"
#include "../include/MoveGeneration/BlackPawnMap.h"
#include "../include/MoveGeneration/KingMap.h"
#include "../include/MoveGeneration/RookMap.h"
#include "../include/MoveGeneration/WhitePawnMap.h"

bool ChessMechanics::IsCheck() const
{
    [[maybe_unused]] const auto [unused1, checksCount, unused2] = GetBlockedFieldBitMap(GetFullBitMap());

    return checksCount > 0;
}

// Gets occupancy maps, which simply indicates wheter some field is occupied or not. Does not distinguish colors.
uint64_t ChessMechanics::GetFullBitMap() const
{
    uint64_t map = 0;
    for (const auto m : _board.BitBoards) map |= m;
    return map;
}

// Gets occupancy maps, which simply indicates wheter some field is occupied or not, by desired color figures.
uint64_t ChessMechanics::GetColBitMap(const int col) const
{
    TraceIfFalse(col == 1 || col == 0, "Invalid color!");

    uint64_t map = 0;
    for (size_t i = 0; i < Board::BitBoardsPerCol; ++i) map |= _board.BitBoards[Board::BitBoardsPerCol * col + i];
    return map;
}

size_t ChessMechanics::GetIndexOfContainingBitBoard(uint64_t map, int col) const
{
    TraceIfFalse(col == 1 || col == 0, "Invalid color!");
    TraceIfFalse(map != 0, "BitMap is empty!");

    const size_t range    = Board::BitBoardsPerCol * col + kingIndex;
    const size_t startInd = Board::BitBoardsPerCol * col;

    for (size_t i = startInd; i < range; ++i)
        if ((_board.BitBoards[i] & map) != 0)
            return i;

#ifndef NDEBUG
    throw std::runtime_error(std::format(
        "[ ERROR ] This code path should never be executed in {} on line {}"
        "\n Figure not found on enemy maps!",
        __FILE__, __LINE__
    ));
#endif

    return 0;
}

/*      IMPORTANT NOTES:
 *  BlockedFieldsMap - indicates wheter some field could be attacked by enemy figures in their next round.
 *  During generation process there are check test performed with counting, what yields 3 code branches inside main
 *  generation code. Map is mainly used when king is moving, allowing to simply predict wheter king should
 *  move to that tile or not.
 *
 */

std::tuple<uint64_t, uint8_t, uint8_t> ChessMechanics::GetBlockedFieldBitMap(uint64_t fullMap) const
{
    TraceIfFalse(fullMap != 0, "Full map is empty!");

    uint8_t checksCount{};
    uint8_t chT{};

    const int enemyCol         = SwapColor(_board.MovingColor);
    const size_t enemyFigInd   = enemyCol * Board::BitBoardsPerCol;
    const int allyKingShift    = ConvertToReversedPos(_board.GetKingMsbPos(_board.MovingColor));
    const uint64_t allyKingMap = 1LLU << allyKingShift;

    // allows to also simply predict which tiles on the other side of the king are allowed.
    const uint64_t fullMapWoutKing = fullMap ^ allyKingMap;

    // King attacks generation.
    const uint64_t kingBlockedMap = KingMap::GetMoves(_board.GetKingMsbPos(enemyCol));

    // Rook attacks generation. Needs special treatment to correctly detect double check, especially with pawn promotion
    const auto [rookBlockedMap, checkCountRook] = _getRookBlockedMap(
        _board.BitBoards[enemyFigInd + rooksIndex] | _board.BitBoards[enemyFigInd + queensIndex], fullMapWoutKing,
        allyKingMap
    );

    // = 0, 1 or eventually 2 when promotion to rook like type happens
    checksCount += checkCountRook;

    // Bishop attacks generation.
    const uint64_t bishopBlockedMap = _blockIterativeGenerator(
        _board.BitBoards[enemyFigInd + bishopsIndex] | _board.BitBoards[enemyFigInd + queensIndex],
        [=](const int pos)
        {
            return BishopMap::GetMoves(pos, fullMapWoutKing);
        }
    );

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByBishopFlag = (bishopBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByBishopFlag;

    // Pawns attacks generation.
    const uint64_t pawnsMap = _board.BitBoards[enemyFigInd + pawnsIndex];
    const uint64_t pawnBlockedMap =
        enemyCol == WHITE ? WhitePawnMap::GetAttackFields(pawnsMap) : BlackPawnMap::GetAttackFields(pawnsMap);

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByPawnFlag = (pawnBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByPawnFlag;

    // modyfing check type
    chT += simpleFigCheck * wasCheckedByPawnFlag; // Note: king cannot be double checked by simple figure

    // Knight attacks generation.
    const uint64_t knighBlockedMap = _blockIterativeGenerator(
        _board.BitBoards[enemyFigInd + knightsIndex],
        [=](const int pos)
        {
            return KnightMap::GetMoves(pos);
        }
    );

    // = 1 or 0 depending whether hits or not
    const uint8_t wasCheckedByKnightFlag = (knighBlockedMap & allyKingMap) >> allyKingShift;
    checksCount += wasCheckedByKnightFlag;

    // modyfing check type
    chT += simpleFigCheck * wasCheckedByKnightFlag; // Note: king cannot be double checked by simple figure

    const uint64_t blockedMap = kingBlockedMap | pawnBlockedMap | knighBlockedMap | rookBlockedMap | bishopBlockedMap;
    return {blockedMap, checksCount, chT};
}

uint64_t ChessMechanics::GetAllowedTilesWhenCheckedByNonSliding() const
{
    uint64_t allowedTiles{};

    allowedTiles |= KingMap::GetSimpleFigCheckKnightsAllowedTiles(_board);
    allowedTiles |= KingMap::GetSimpleFigCheckPawnAllowedTiles(_board);

    return allowedTiles;
}

std::pair<uint64_t, uint8_t>
ChessMechanics::_getRookBlockedMap(uint64_t rookMap, const uint64_t fullMapWoutKing, const uint64_t kingMap)
{
    TraceIfFalse(kingMap != 0, "King map is empty!");

    uint64_t blockedTiles{};
    uint8_t checks{};

    while (rookMap)
    {
        const int msbPos = ExtractMsbPos(rookMap);

        const uint64_t moves = RookMap::GetMoves(msbPos, fullMapWoutKing);
        blockedTiles |= moves;
        checks += ((moves & kingMap) != 0);

        rookMap ^= MaxMsbPossible >> msbPos;
    }

    return {blockedTiles, checks};
}

uint64_t ChessMechanics::GenerateAllowedTilesForPrecisedPinnedFig(const uint64_t figBoard, const uint64_t fullMap) const
{
    TraceIfFalse(fullMap != 0, "Full map is empty!");
    TraceIfFalse(CountOnesInBoard(figBoard) == 1, "Only one figure should be pinned!");

    const int msbPos         = ExtractMsbPos(figBoard);
    const uint64_t KingBoard = _board.BitBoards[_board.MovingColor * Board::BitBoardsPerCol + kingIndex];

    const uint64_t RookPerspectiveMoves = RookMap::GetMoves(msbPos, fullMap);
    if ((RookPerspectiveMoves & KingBoard) != 0)
        return RookPerspectiveMoves & RookMap::GetMoves(ExtractMsbPos(KingBoard), fullMap ^ figBoard);

    const uint64_t BishopPerspectiveMoves = BishopMap::GetMoves(msbPos, fullMap);
    return BishopPerspectiveMoves & BishopMap::GetMoves(ExtractMsbPos(KingBoard), fullMap ^ figBoard);
}
