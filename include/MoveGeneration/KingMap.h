//
// Created by Jlisowskyy on 12/30/23.
//

#ifndef KINGMAP_H
#define KINGMAP_H

#include <array>
#include <cstdint>

#include "../EngineTypeDefs.h"
#include "MoveGeneration.h"
#include "PinningMask.h"

#include "KnightMap.h"

struct KingMap
{
    // ------------------------------
    // Class creation
    // ------------------------------

    KingMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr size_t GetBoardIndex(const int color)
    {
        return Board::BoardsPerCol * color + kingIndex;
    }

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd)
    {
        return movesMap[msbInd];
    }

    // genarates tiles on which pawns currently attacks king
    [[nodiscard]] static constexpr uint64_t GetSimpleFigCheckPawnAllowedTiles(const Board& bd)
    {
        const uint64_t detectionFields = bd.movColor == WHITE ?
            _getWhiteKingDetectionTiles(bd) : _getBlackKingDetectionTiles(bd);

        return detectionFields & bd.boards[Board::BoardsPerCol*SwapColor(bd.movColor) + pawnsIndex];
    }

    [[nodiscard]] static constexpr uint64_t GetSimpleFigCheckKnightsAllowedTiles(const Board& bd)
    {
        const uint64_t detectionFields = KnightMap::GetMoves(bd.GetKingMsbPos(bd.movColor));

        return detectionFields & bd.boards[Board::BoardsPerCol*SwapColor(bd.movColor) + knightsIndex];
    }

    // ------------------------------
    // Private methods
    // ------------------------------
private:

    // genarates possibles tiles on which enemy pawn could attack king
    [[nodiscard]] static constexpr uint64_t _getWhiteKingDetectionTiles(const Board& bd)
    {
        const uint64_t kingMap = bd.boards[Board::BoardsPerCol*WHITE+ kingIndex];

        const uint64_t leftDetectionTile = (kingMap & LeftPawnDetectionMask) << 7;
        const uint64_t rightDetectionTile = (kingMap & RightPawnDetetcionMask) << 9;

        return leftDetectionTile | rightDetectionTile;
    }

    // genarates possibles tiles on which enemy pawn could attack king
    [[nodiscard]] static constexpr uint64_t _getBlackKingDetectionTiles(const Board& bd)
    {
        const uint64_t kingMap = bd.boards[Board::BoardsPerCol*BLACK + kingIndex];

        const uint64_t rightDetectionTile = (kingMap & RightPawnDetetcionMask) >> 7;
        const uint64_t leftDetectionTile = (kingMap & LeftPawnDetectionMask) >> 9;

        return leftDetectionTile | rightDetectionTile;
    }

    // ------------------------------
    // Class Fields
    // ------------------------------
public:

    static constexpr std::array<PinningMasks, Board::BoardFields> pinMasks = PinningMasks::PinningArrayFactory();

private:
    static constexpr size_t maxMovesCount = 8;

    // Describes king possible moves cordinates.
    static constexpr int movesCords[] = {-1, -9, -8, -7, 1, 9, 8, 7};

    // Describes accordingly y positions after the move relatively to king's y position.
    // Used to omit errors during generation.
    static constexpr int rowCords[] = {0, -1, -1, -1, 0, 1, 1, 1};

    static constexpr std::array<uint64_t, Board::BoardFields> movesMap = GenStaticMoves(
        maxMovesCount, movesCords, rowCords);

    // Masks used to detect allowed tiles when checked by pawn
    static constexpr uint64_t LeftPawnDetectionMask = ~GenMask(0, 57, 8);
    static constexpr uint64_t RightPawnDetetcionMask = ~GenMask(7, 64, 8);
};

#endif //KINGMAP_H
