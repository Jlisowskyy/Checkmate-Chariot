//
// Created by Jlisowskyy on 3/23/24.
//

#include "../include/MapTypes/FancyMagicBishopMap.h"

void FancyMagicBishopMap::ParameterSearch()
{
    auto nGen = []([[maybe_unused]] const int, const BishopMapGenerator::MasksT &m)
    {
        return BishopMapGenerator::GenPossibleNeighborsWithOverlap(m);
    };
    auto mInit = [](const int bInd)
    {
        return BishopMapGenerator::InitMasks(bInd);
    };

    _underlyingMapT::FindCollidingIndices<
        decltype(nGen), decltype(mInit), [](const uint64_t n, const std::array<uint64_t, 4> &m)
        {
            return BishopMapGenerator::StripBlockingNeighbors(n, m);
        }>(funcs, nGen, mInit);

    _underlyingMapT::FindHashParameters<
        decltype(nGen), decltype(mInit), [](const uint64_t n, const std::array<uint64_t, 4> &m)
        {
            return BishopMapGenerator::StripBlockingNeighbors(n, m);
        }>(funcs, nGen, mInit);
}
