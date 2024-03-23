//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef ROOKMAP_H
#define ROOKMAP_H

#include "../MapTypes/FancyMagicRookMap.h"

class RookMap
{
    // -------------------------------
    // Underlying map definition
    // -------------------------------

    using _underlyingMap = FancyMagicRookMap;

   public:
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    RookMap() = delete;

    ~RookMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr size_t GetBoardIndex(int color);

    [[nodiscard]] static constexpr uint64_t GetMoves(int msbInd, uint64_t fullBoard,
                                                     [[maybe_unused]] uint64_t = 0);

    [[nodiscard]] static constexpr size_t GetMatchingCastlingIndex(const Board& bd, uint64_t figBoard);

    // ------------------------------
    // Class fields
    // ------------------------------
   private:
    static constexpr const char* names[] = {"lMask", "rMask", "uMask", "dMask"};

    static constexpr _underlyingMap _map{};
};

constexpr size_t RookMap::GetBoardIndex(const int color)
{
    return Board::BoardsPerCol * color + rooksIndex;
}

constexpr uint64_t RookMap::GetMoves(const int msbInd, const uint64_t fullBoard, const uint64_t)
{
    return _map.GetMoves(msbInd, fullBoard);
}

constexpr size_t RookMap::GetMatchingCastlingIndex(const Board& bd, const uint64_t figBoard)
{
    for (size_t i = 0; i < Board::CastlingsPerColor; ++i)
        if (const size_t index = bd.movColor * Board::CastlingsPerColor + i;
            bd.Castlings[index] && (Board::CastlingsRookMaps[index] & figBoard) != 0)
            return index;

    return Board::SentinelCastlingIndex;
}

#endif  // ROOKMAP_H
