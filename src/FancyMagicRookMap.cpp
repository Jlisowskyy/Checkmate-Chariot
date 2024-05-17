//
// Created by Jlisowskyy on 3/23/24.
//

#include "../include/MapTypes/FancyMagicRookMap.h"

void FancyMagicRookMap::ParameterSearch()
{
    auto nGen = []([[maybe_unused]] const int, const RookMapGenerator::MasksT &m)
    {
        return RookMapGenerator::GenPossibleNeighborsWithOverlap(m);
    };
    auto mInit = [](const int bInd)
    {
        return RookMapGenerator::InitMasks(bInd);
    };

    _underlyingMapT::FindCollidingIndices<
        decltype(nGen), decltype(mInit), [](const uint64_t n, const std::array<uint64_t, 4> &m)
        {
            return RookMapGenerator::StripBlockingNeighbors(n, m);
        }>(funcs, nGen, mInit);

    _underlyingMapT::FindHashParameters<
        decltype(nGen), decltype(mInit), [](const uint64_t n, const std::array<uint64_t, 4> &m)
        {
            return RookMapGenerator::StripBlockingNeighbors(n, m);
        }>(funcs, nGen, mInit);
}
