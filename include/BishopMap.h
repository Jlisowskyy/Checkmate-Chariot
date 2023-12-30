//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef BISHOPMAP_H
#define BISHOPMAP_H

#include "movesHashMap.h"
#include "EngineTypeDefs.h"
#include "MoveGeneration.h"

class BishopMap {
    static constexpr size_t MaxBishopPossibleNeighbors = 108;

public:
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    BishopMap() {
        _initMaps();
    }

    void FindHashParameters() {

    }

    // ------------------------------
    // Class interaction
    // ------------------------------

    void Tester() {
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                std::cout << _neighborLayoutPossibleCountOnField(x, y) << std::endl;
            }
        }

    }

    [[nodiscard]] uint64_t GetMoves(int msbInd, uint64_t fullBoard, uint64_t allyBoard) const {
        return 0;
    }

    // ------------------------------
    // Class private methods
    // ------------------------------

    [[nodiscard]] static constexpr size_t _neighborLayoutPossibleCountOnField(const int x, const int y) {
        const size_t nwCount = std::max(1, std::min(x, 7-y));
        const size_t neCount = std::max(1, std::min(7-x, 7-y));
        const size_t swCount = std::max(1, std::min(x, y));
        const size_t seCount = std::max(1, std::min(7-x, y));

        return nwCount * neCount * swCount * seCount;
    }

    [[nodiscard]] static constexpr uint64_t _genNWMask(const int x, const int y, const int bInd) {
        const int border = bInd + 7*std::max(0, std::min(x, 7-y) - 1);

        return GenMask(border, bInd, 7, std::less_equal{});
    }

    [[nodiscard]] static constexpr uint64_t _genNEMask(const int x, const int y, const int bInd) {
        const int border = bInd + 9*std::max(0, std::min(7-x, 7-y) - 1);

        return GenMask(border, bInd, 9, std::less_equal{});
    }

    [[nodiscard]] static constexpr uint64_t _genSWMask(const int x, const int y, const int bInd) {
        const int border = bInd - 9*std::max(0, std::min(x, y) - 1);

        return GenMask(border, bInd, -9, std::greater_equal{});
    }

    [[nodiscard]] static constexpr uint64_t _genSEMask(const int x, const int y, const int bInd) {
        const int border = bInd - 7*std::max(0, std::min(7-x, y) - 1);

        return GenMask(border, bInd, -7, std::greater_equal{});
    }

    [[nodiscard]] static constexpr std::array<uint64_t, movesHashMap::MasksCount> _initMasks(const int bInd) {
        std::array<uint64_t, movesHashMap::MasksCount> ret{};
        const int x = bInd % 8;
        const int y = bInd / 8;

        ret[neMask] = _genNEMask(x, y, bInd);
        ret[nwMask] = _genNWMask(x, y, bInd);
        ret[seMask] = _genSEMask(x, y, bInd);
        ret[swMask] = _genSWMask(x, y, bInd);

        return ret;
    }

    void _initMaps() {
        MapInitializer(aHashValues, bHashValues, layer1,
            [](const int x, const int y) { return _neighborLayoutPossibleCountOnField(x, y); },
            [](const int bInd) { return _initMasks(bInd); }
        );
    }

    // ------------------------------
    // Class inner types
    // ------------------------------
private:

    enum maskInd {
        nwMask,
        neMask,
        swMask,
        seMask,
    };

    // ------------------------------
    // Class fields
    // ------------------------------

    static constexpr uint64_t aHashValues[Board::BoardFields] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static constexpr uint64_t bHashValues[Board::BoardFields] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    movesHashMap layer1[Board::BoardFields];
};



#endif //BISHOPMAP_H
