//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef ROOKMAP_H
#define ROOKMAP_H

#include <array>

#include "EngineTypeDefs.h"

struct RookMap {
    constexpr RookMap() = default;

    struct GenRecord {
        Field field;
        uint64_t position;
        uint64_t moves;
    };

    struct mapRecord {
        uint64_t lMask;
        uint64_t rMask;
        uint64_t uMask;
        uint64_t dMask;

        // TODO: second map
    };

    template<class incFunc, class boundryCheckFunc>
    constexpr static uint64_t GenMask(int barrier, int boardIndex, incFunc inc, boundryCheckFunc boundryCheck) {
        uint64_t mask = 0;

        while(boundryCheck(boardIndex = inc(boardIndex), barrier))
            mask |= (1LLU<<boardIndex);

        return mask;
    }

    constexpr static uint64_t GenRMask(const int barrier, const int boardIndex) {
        return GenMask(barrier, boardIndex, [](const int x) -> int { return x + 1; }, [](const int ind, const int bar) -> bool { return ind < bar; });
    }

    constexpr static uint64_t GenLMask(const int barrier, const int boardIndex) {
        return GenMask(barrier, boardIndex, [](const int x) -> int { return x - 1; }, [](const int ind, const int bar) -> bool { return ind > bar; });
    }

    constexpr static uint64_t GenUMask(const int barrier, const int boardIndex) {
        return GenMask(barrier, boardIndex, [](const int x) -> int { return x + 8; }, [](const int ind, const int bar) -> bool { return ind < bar; });
    }

    constexpr static uint64_t GenDMask(const int barrier, const int boardIndex) {
        return GenMask(barrier, boardIndex, [](const int x) -> int { return x - 8; }, [](const int ind, const int bar) -> bool { return ind > bar; });
    }

    static constexpr size_t MaxRookPossibleNeighbors = 7396;
    static constexpr std::tuple<std::array<uint64_t, MaxRookPossibleNeighbors>, size_t>
    GenPossibleNeighbors(const int bInd, const mapRecord& record) {
        auto ret = std::make_tuple(std::array<uint64_t, MaxRookPossibleNeighbors>{}, size_t{});
        size_t usedFields = 0;

        const int lBarrier = ((bInd >> 3) << 3) - 1;
        const int rBarrier = lBarrier + 9;
        const int uBarrier = 64;
        const int dBarrier = -1;

        const uint64_t bPos = 1LLU << bInd;
        for (int l = bInd; l > lBarrier; --l) {
            const uint64_t lPos = 1LLU << l;
            if (lPos != bPos && (record.lMask & lPos) == 0)
                continue;

            for (int r = bInd; r < rBarrier; ++r) {
                const uint64_t rPos = 1LLU << r;
                if (rPos != bPos && (record.rMask & rPos) == 0)
                    continue;

                for (int u = bInd; u < uBarrier; u+=8) {
                    const uint64_t uPos = 1LLU << u;
                    if (uPos != bPos && (record.uMask & uPos) == 0)
                        continue;


                    for (int d = bInd; d > dBarrier; d-=8) {
                        const uint64_t dPos = 1LLU << d;
                        if (dPos != bPos && (record.dMask & dPos) == 0)
                            continue;

                        const uint64_t neighbor = (dPos | uPos | rPos | lPos) ^ bInd;
                        std::get<0>(ret)[usedFields++] = neighbor;
                    }
                }
            }
        }

        std::get<1>(ret) = usedFields;
        return ret;
    }

    void Init() {

        // Mask generation and possibilities generation
        size_t allNeighbors = 0;
        for(int i = 0; i < Board::BoardFields; ++i) {
            const int bInd = 63 - i;

            const int lBarrier = ((bInd >> 3) << 3);
            const int rBarrier = lBarrier + 7;
            const int uBarrier = 56;
            const int dBarrier = 7;

            layer1[i].dMask = GenDMask(dBarrier, bInd);
            layer1[i].lMask = GenLMask(lBarrier, bInd);
            layer1[i].rMask = GenRMask(rBarrier, bInd);
            layer1[i].uMask = GenUMask(uBarrier, bInd);

            auto possibilities = GenPossibleNeighbors(bInd, layer1[i]);
            allNeighbors += std::get<1>(possibilities);
            std::cout << "Position index: " << bInd << ", " << std::get<1>(possibilities) << std::endl;
        }

        std::cout << "All possible neighbors: " << allNeighbors << std::endl;
    }

    // uint64_t GetMoves(uint64_t field, uint64_t neighbors) {
    //
    // }
    //
    // static uint64_t GetLeftBlock(uint64_t board, uint64_t field) {
    //
    // }
    //
    // static uint64_t GetRightBlock(uint64_t board, uint64_t field) {
    //
    // }
    //
    // static uint64_t GetUpperBlock(uint64_t board, uint64_t field) {
    //
    // }
    //
    // static uint64_t GetLowerBlock(uint64_t board, uint64_t field) {
    //
    // }
private:
    mapRecord layer1[Board::BoardFields] {};
};



#endif //ROOKMAP_H
