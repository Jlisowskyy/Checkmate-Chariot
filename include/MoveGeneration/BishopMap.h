//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef BISHOPMAP_H
#define BISHOPMAP_H

#include "../MapTypes/FancyMagicBishopMap.h"

class BishopMap
{
    // -------------------------------
    // Underlying map definition
    // -------------------------------

    using _underlyingMap = FancyMagicBishopMap;

    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    public:
    BishopMap() = delete;

    ~BishopMap() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr size_t GetBoardIndex(int color);

    [[nodiscard]] static constexpr uint64_t GetMoves(int msbInd, uint64_t fullBoard, [[maybe_unused]] uint64_t = 0);

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr const char *names[] = {"nwMask", "neMask", "swMask", "seMask"};

    static constexpr _underlyingMap _map{};
};

constexpr size_t BishopMap::GetBoardIndex(const int color) { return Board::BitBoardsPerCol * color + bishopsIndex; }

constexpr uint64_t BishopMap::GetMoves(const int msbInd, const uint64_t fullBoard, const uint64_t)
{
    return _map.GetMoves(msbInd, fullBoard);
}

#endif // BISHOPMAP_H
