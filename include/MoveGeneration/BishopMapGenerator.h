//
// Created by Jlisowskyy on 1/1/24.
//

#ifndef BISHOPMAPGENERATOR_H
#define BISHOPMAPGENERATOR_H

#include "MoveGeneration.h"
#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"

class BishopMapGenerator {
    static constexpr size_t MaxBishopPossibleNeighbors = 108;

    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------
public:

    BishopMapGenerator() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template<class MapT>
    [[nodiscard]] static constexpr MapT GetMap() {
        MapT maps{};

        _initMaps(maps);
        _initMoves(maps);

        return maps;
    }


    // ------------------------------
    // Class private methods
    // ------------------------------
private:

    [[nodiscard]] static constexpr std::tuple<std::array<uint64_t, MaxBishopPossibleNeighbors>, size_t>
            _genPossibleNeighbors(const int bInd, const movesHashMap& record)
    {
        std::array<uint64_t, MaxBishopPossibleNeighbors> ret{};
        size_t usedFields = 0;

        const int x = bInd % 8;
        const int y = bInd / 8;

        const int NWBorder = bInd + 7*std::min(x, 7-y);
        const int NEBorder = bInd + 9*std::min(7-x, 7-y) ;
        const int SWBorder = bInd - 9*std::min(x, y);
        const int SEBorder = bInd - 7*std::min(7-x, y);

        const uint64_t bPos = 1LLU << bInd;
        for (int nw = bInd; nw <= NWBorder; nw += 7) {
            const uint64_t nwPos = minMsbPossible << nw;
            if (nwPos != bPos && (record.masks[nwMask] & nwPos) == 0)
                continue;

            for (int ne = bInd; ne <= NEBorder; ne +=9) {
                const uint64_t nePos = minMsbPossible << ne;
                if (nePos != bPos && (record.masks[neMask] & nePos) == 0)
                    continue;

                for (int sw = bInd; sw >= SWBorder; sw-=9) {
                    const uint64_t swPos = minMsbPossible << sw;
                    if (swPos != bPos && (record.masks[swMask] & swPos) == 0)
                        continue;

                    for (int se = bInd; se >= SEBorder; se-=7) {
                        const uint64_t sePos = minMsbPossible << se;
                        if (sePos != bPos && (record.masks[seMask] & sePos) == 0)
                            continue;

                        const uint64_t neighbor = (nwPos | nePos | swPos | sePos) ^ bPos;
                        ret[usedFields++] = neighbor;
                    }
                }
            }
        }

        return {ret, usedFields};
    }

    template<class MapT>
    static constexpr void _initMoves(MapT& maps){
        MoveInitializer(maps,
                        [](const uint64_t neighbors, const int bInd) constexpr
                        { return _genMoves(neighbors, bInd); },
                        [](const int bInd, const movesHashMap& record) constexpr
                        { return _genPossibleNeighbors(bInd, record); }
        );
    }

    [[nodiscard]] static constexpr uint64_t _genMoves(const uint64_t neighbors, const int bInd){
        const int y = bInd / 8;
        const int x = bInd % 8;

        const int NWBorder = bInd + 7*std::min(x, 7-y);
        const int NEBorder = bInd + 9*std::min(7-x, 7-y) ;
        const int SWBorder = bInd - 9*std::min(x, y);
        const int SEBorder = bInd - 7*std::min(7-x, y);

        uint64_t moves = 0;

        // NW direction moves
        moves |= GenSlidingMoves(neighbors, bInd, 7,
                                 [&](const int b) { return b <= NWBorder; }
        );

        // NE direction moves
        moves |= GenSlidingMoves(neighbors, bInd, 9,
                                 [&](const int b) { return b <= NEBorder; }
        );

        // SW direction moves
        moves |= GenSlidingMoves(neighbors, bInd, -9,
                                 [&](const int b) { return b >= SWBorder; }
        );

        // SW direction moves
        moves |= GenSlidingMoves(neighbors, bInd, -7,
                                 [&](const int b) { return b >= SEBorder; }
        );

        return moves;
    }

    [[nodiscard]] static constexpr size_t _neighborLayoutPossibleCountOnField(int x, int y){
        const size_t nwCount = std::max(1, std::min(x, 7-y));
        const size_t neCount = std::max(1, std::min(7-x, 7-y));
        const size_t swCount = std::max(1, std::min(x, y));
        const size_t seCount = std::max(1, std::min(7-x, y));

        return nwCount * neCount * swCount * seCount;
    }


    [[nodiscard]] static constexpr std::array<uint64_t, movesHashMap::MasksCount> _initMasks(const int bInd){
        std::array<uint64_t, movesHashMap::MasksCount> ret{};
        const int x = bInd % 8;
        const int y = bInd / 8;

        const int NEBorder = bInd + 9*std::max(0, std::min(7-x, 7-y) - 1);
        const int NWBorder = bInd + 7*std::max(0, std::min(x, 7-y) - 1);
        const int SEBorder = bInd - 7*std::max(0, std::min(7-x, y) - 1);
        const int SWBorder = bInd - 9*std::max(0, std::min(x, y) - 1);

        // neMask generation
        ret[neMask] = GenMask(NEBorder, bInd, 9, std::less_equal{});

        // nwMask generation
        ret[nwMask] = GenMask(NWBorder, bInd, 7, std::less_equal{});

        // seMask generation
        ret[seMask] = GenMask(SEBorder, bInd, -7, std::greater_equal{});

        // swMask generation
        ret[swMask] = GenMask(SWBorder, bInd, -9, std::greater_equal{});

        return ret;
    }

    template<class MapT>
    static constexpr void _initMaps(MapT& maps){
        MapInitializer(maps,
                       [](const int x, const int y) { return _neighborLayoutPossibleCountOnField(x, y); },
                       [](const int bInd) { return _initMasks(bInd); }
        );
    }


    // ------------------------------
    // Class inner types
    // ------------------------------
public:

    enum maskInd {
        nwMask,
        neMask,
        swMask,
        seMask,
    };

};

#endif //BISHOPMAPGENERATOR_H
