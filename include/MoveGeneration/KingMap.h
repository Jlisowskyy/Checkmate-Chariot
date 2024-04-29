//
// Created by Jlisowskyy on 12/30/23.
//

#ifndef KINGMAP_H
#define KINGMAP_H

#include <array>
#include <cstdint>

#include "KnightMap.h"
#include "MoveGenerationUtils.h"

struct KingMap
{
    // ------------------------------
    // Class creation
    // ------------------------------

    KingMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr size_t GetBoardIndex(int color);

    [[nodiscard]] static constexpr uint64_t GetMoves(int msbInd);

    // genarates tiles on which pawns currently attacks king
    [[nodiscard]] static constexpr uint64_t GetSimpleFigCheckPawnAllowedTiles(const Board& bd);

    [[nodiscard]] static constexpr uint64_t GetSimpleFigCheckKnightsAllowedTiles(const Board& bd);

    // ------------------------------
    // Private methods
    // ------------------------------

   private:
    // genarates possibles tiles on which enemy pawn could attack king
    [[nodiscard]] static constexpr uint64_t _getWhiteKingDetectionTiles(const Board& bd);

    // genarates possibles tiles on which enemy pawn could attack king
    [[nodiscard]] static constexpr uint64_t _getBlackKingDetectionTiles(const Board& bd);

    // ------------------------------
    // Class Fields
    // ------------------------------

    static constexpr size_t maxMovesCount = 8;

    // Describes king possible moves cordinates.
    static constexpr int movesCords[] = {-1, -9, -8, -7, 1, 9, 8, 7};

    // Describes accordingly y positions after the move relatively to king's y position.
    // Used to omit errors during generation.
    static constexpr int rowCords[] = {0, -1, -1, -1, 0, 1, 1, 1};

    static constexpr std::array<uint64_t, Board::BitBoardFields> movesMap =
        GenStaticMoves(maxMovesCount, movesCords, rowCords);

    // Masks used to detect allowed tiles when checked by pawn
    static constexpr uint64_t LeftPawnDetectionMask = ~GenMask(0, 57, 8);
    static constexpr uint64_t RightPawnDetetcionMask = ~GenMask(7, 64, 8);
public:

    static constexpr std::array<uint64_t, 2> ShelterLocationMask = []() {
        const int fields[] = {0,1,6,7};
        const int lines[][2] = {{0,1*8},{6*8,7*8}};
        uint64_t base{};

        for(const int field : fields) {
            base |= minMsbPossible << field;
        }

        std::array<uint64_t, 2> rv{};

        for (int col = WHITE; col <= BLACK; ++col) {
            rv[col] = (base << lines[col][0]) | (base << lines[col][1]);
        }

        return rv;
    }();
};

constexpr size_t KingMap::GetBoardIndex(const int color)
{
    return Board::BitBoardsPerCol * color + kingIndex;
}

constexpr uint64_t KingMap::GetMoves(const int msbInd)
{ return movesMap[msbInd]; }

constexpr uint64_t KingMap::GetSimpleFigCheckPawnAllowedTiles(const Board& bd)
{
    const uint64_t detectionFields =
            bd.MovingColor == WHITE ? _getWhiteKingDetectionTiles(bd) : _getBlackKingDetectionTiles(bd);

    return detectionFields & bd.BitBoards[Board::BitBoardsPerCol * SwapColor(bd.MovingColor) + pawnsIndex];
}

constexpr uint64_t KingMap::GetSimpleFigCheckKnightsAllowedTiles(const Board& bd)
{
    const uint64_t detectionFields = KnightMap::GetMoves(bd.GetKingMsbPos(bd.MovingColor));

    return detectionFields & bd.BitBoards[Board::BitBoardsPerCol * SwapColor(bd.MovingColor) + knightsIndex];
}

constexpr uint64_t KingMap::_getWhiteKingDetectionTiles(const Board& bd)
{
    const uint64_t kingMap = bd.BitBoards[Board::BitBoardsPerCol * WHITE + kingIndex];

    const uint64_t leftDetectionTile = (kingMap & LeftPawnDetectionMask) << 7;
    const uint64_t rightDetectionTile = (kingMap & RightPawnDetetcionMask) << 9;

    return leftDetectionTile | rightDetectionTile;
}

constexpr uint64_t KingMap::_getBlackKingDetectionTiles(const Board& bd)
{
    const uint64_t kingMap = bd.BitBoards[Board::BitBoardsPerCol * BLACK + kingIndex];

    const uint64_t rightDetectionTile = (kingMap & RightPawnDetetcionMask) >> 7;
    const uint64_t leftDetectionTile = (kingMap & LeftPawnDetectionMask) >> 9;

    return leftDetectionTile | rightDetectionTile;
}

#endif  // KINGMAP_H
