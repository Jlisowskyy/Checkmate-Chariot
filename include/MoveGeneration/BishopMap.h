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

    [[nodiscard]] static constexpr size_t GetBoardIndex(const int color)
    {
        return Board::BoardsPerCol * color + bishopsIndex;
    }

    [[nodiscard]] static constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard,
                                                     [[maybe_unused]] const uint64_t _ = 0)
    {
        return _map.GetMoves(msbInd, fullBoard);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr const char* names[] = {"nwMask", "neMask", "swMask", "seMask"};

    static constexpr _underlyingMap _map{};
};

#endif //BISHOPMAP_H
