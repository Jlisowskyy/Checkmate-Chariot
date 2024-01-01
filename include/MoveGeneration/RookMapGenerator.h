//
// Created by Jlisowskyy on 1/1/24.
//

#ifndef ROOKMAPGENERATOR_H
#define ROOKMAPGENERATOR_H

#include <array>

#include "../BitOperations.h"
#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "MoveGeneration.h"

class RookMapGenerator {
    static constexpr size_t MaxRookPossibleNeighbors = 144;
public:
    // ---------------------------------------
    // Class creation and initialization
    // ---------------------------------------

    RookMapGenerator() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    [[nodiscard]] static constexpr std::array<movesHashMap, Board::BoardFields> GetMap() {
        std::array<movesHashMap, Board::BoardFields> maps{};

        _initMaps(maps);
        _initMoves(maps);

        return maps;
    }

    static void FindHashParameters(){
        movesHashMap::FindHashParameters(aHashValues, bHashValues, GetMap().data(),
            [](const int bInd, const movesHashMap& record) { return _genPossibleNeighbors(bInd, record); }
        );
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    static constexpr void _initMoves(std::array<movesHashMap, Board::BoardFields>& maps){
        MoveInitializer(maps,
            [](const uint64_t neighbors, const int bInd) constexpr
                            { return _genMoves(neighbors, bInd); },
            [](const int bInd, const movesHashMap& record) constexpr
                            { return _genPossibleNeighbors(bInd, record); }
        );
    }

    static constexpr void _initMaps(std::array<movesHashMap, Board::BoardFields>& maps){
        MapInitializer(aHashValues, bHashValues, maps,
            [](const int x, const int y) constexpr
                { return _neighborLayoutPossibleCountOnField(x, y); },
            [](const int bInd) constexpr
                { return _initMasks(bInd); }
        );
    }

    [[nodiscard]] constexpr static std::array<uint64_t, movesHashMap::MasksCount> _initMasks(const int bInd){
        std::array<uint64_t, movesHashMap::MasksCount> ret{};

        // Mask generation.
        const int lBarrier = (bInd / 8) * 8;
        const int rBarrier = lBarrier + 7;

        // Lower vertical line
        ret[dMask] = GenMask(7, bInd, -8, std::greater{});

        // Left Horizontal line
        ret[lMask] = GenMask(lBarrier, bInd, -1, std::greater{});

        // Right horizontal line
        ret[rMask] = GenMask(rBarrier, bInd, 1, std::less{});

        // Upper vertical line
        ret[uMask] = GenMask(56, bInd, 8, std::less{});

        return ret;
    }

    [[nodiscard]] constexpr static uint64_t _genMoves(const uint64_t neighbors, const int bInd){
        constexpr int uBarrier = 64;
        constexpr int dBarrier = -1;
        const int lBarrier = (bInd / 8) * 8 - 1;
        const int rBarrier = lBarrier + 9;

        uint64_t moves = 0;

        // upper lines moves
        moves |= GenSlidingMoves(neighbors, bInd, 8,
         [&](const int x){ return x < uBarrier; }
        );

        // lower lines moves
        moves |= GenSlidingMoves(neighbors, bInd, -8,
                                 [&](const int x){ return x > dBarrier; }
        );

        // right line moves
        moves |= GenSlidingMoves(neighbors, bInd, 1,
                                 [&](const int x){ return x < rBarrier; }
        );

        // left line moves
        moves |= GenSlidingMoves(neighbors, bInd, -1,
                                 [&](const int x){ return x > lBarrier; }
        );

        return moves;
    }

    [[nodiscard]] constexpr static std::tuple<std::array<uint64_t, MaxRookPossibleNeighbors>, size_t>
        _genPossibleNeighbors(const int bInd, const movesHashMap& record)
    {
        std::array<uint64_t, MaxRookPossibleNeighbors> ret{};
        size_t usedFields = 0;

        const int lBarrier = ((bInd >> 3) << 3) - 1;
        const int rBarrier = lBarrier + 9;
        constexpr int uBarrier = 64;
        constexpr int dBarrier = -1;

        const uint64_t bPos = 1LLU << bInd;
        for (int l = bInd; l > lBarrier; --l) {
            const uint64_t lPos = minMsbPossible << l;
            if (lPos != bPos && (record.masks[lMask] & lPos) == 0)
                continue;

            for (int r = bInd; r < rBarrier; ++r) {
                const uint64_t rPos = minMsbPossible << r;
                if (rPos != bPos && (record.masks[rMask] & rPos) == 0)
                    continue;

                for (int u = bInd; u < uBarrier; u+=8) {
                    const uint64_t uPos = minMsbPossible << u;
                    if (uPos != bPos && (record.masks[uMask] & uPos) == 0)
                        continue;


                    for (int d = bInd; d > dBarrier; d-=8) {
                        const uint64_t dPos = minMsbPossible << d;
                        if (dPos != bPos && (record.masks[dMask] & dPos) == 0)
                            continue;

                        const uint64_t neighbor = (dPos | uPos | rPos | lPos) ^ bPos;
                        ret[usedFields++] = neighbor;
                    }
                }
            }
        }

        return {ret, usedFields};
    }


    [[nodiscard]] static constexpr size_t _neighborLayoutPossibleCountOnField(const int x, const int y)
    {
        const int lCount = std::max(1, x);
        const int dCount = std::max(1, y);
        const int uCount = std::max(1, 7-y);
        const int rCount = std::max(1, 7-x);

        return lCount * rCount * dCount * uCount;
    }

    // ------------------------------
    // Class inner types
    // ------------------------------
public:

    enum maskInd
    {
        lMask,
        rMask,
        uMask,
        dMask,
    };

    // ------------------------------
    // Class fields
    // ------------------------------
private:

    static constexpr uint64_t aHashValues[Board::BoardFields] = {
        3393641422875280979LLU,
        5471342235767773913LLU,
        15250091735978237630LLU,
        12442485387656305543LLU,
        0,
        0,
        9676328163961630310LLU,
        0,
        0,
        0,
        0,
        0,
        216587437274599482LLU,
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
        9048537097603897538LLU,
        16622022989650590567LLU,
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
        9112182513768572659LLU,
        9321354766185832861LLU,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        14235290041803269772LLU,
        0,
        0,
        0,
        0,
        13178403195126074399LLU,
        2182512708681601225LLU,
        17174721626244749098LLU,
        6616896462523411835LLU,
        17204342939815526595LLU,
        9846749713679825003LLU,
        18181425027871411485LLU,
        17506460483279960373LLU,
        15969999706032231071LLU,
        4954199076915290639LLU,
    };

    static constexpr uint64_t bHashValues[Board::BoardFields] = {
        17312422767356678212LLU,
        17307815045900276771LLU,
        16356769246725350830LLU,
        16198993108558860794LLU,
        0,
        0,
        13570260835205647813LLU,
        0,
        0,
        0,
        0,
        0,
        10929190736371504000LLU,
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
        8504663955913029650LLU,
        697641190516267145LLU,
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
        17493466365116381458LLU,
        10023829598634898570LLU,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        17014087199960585519LLU,
        0,
        0,
        0,
        0,
        3429497423754197273LLU,
        17466145088073078408LLU,
        2025289277575576374LLU,
        7937903244653902784LLU,
        14168889339532498898LLU,
        4953919125260249464LLU,
        8170902763400531562LLU,
        7554106665307752666LLU,
        8765763996995066529LLU,
        7109284141322176083LLU,
    };
};

#endif //ROOKMAPGENERATOR_H
