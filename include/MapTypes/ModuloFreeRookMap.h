//
// Created by Jlisowskyy on 2/9/24.
//

#ifndef MODULOFREEROOKMAP_H
#define MODULOFREEROOKMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/RookMapGenerator.h"
#include "../MoveGeneration/SparseRandomGenerator.h"

class ModuloFreeRookMap
{
    using _hashFuncT = Fast2PowHashFunction<SparseRandomGenerator<>>;
    using _underlyingMapT = movesHashMap<_hashFuncT>;

public:
    constexpr ModuloFreeRookMap()
    {
        for (int i = 0; i < Board::BoardFields; ++i)
        {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(RookMapGenerator::InitMasks(boardIndex), funcs[i]);
            MoveInitializer(_maps[i],
                            [](const uint64_t n, const int ind) constexpr
                            {
                                return RookMapGenerator::GenMoves(n, ind);
                            },
                            [](const int ind, const RookMapGenerator::MasksT&m) constexpr
                            {
                                return RookMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m);
                            },
                            [](const uint64_t b, const RookMapGenerator::MasksT&m) constexpr { return b; },
                            boardIndex
            );
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const
    {
        const uint64_t closestNeighbors = RookMapGenerator::StripBlockingNeighbors(fullBoard, _maps[msbInd].masks);

        return _maps[msbInd][closestNeighbors];
    }

    static void ParameterSearch()
    {
        _underlyingMapT::FindCollidingIndices(
            funcs,
            [](const int ind, const RookMapGenerator::MasksT&m)
            {
                return RookMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m);
            },
            [](const int bInd) { return RookMapGenerator::InitMasks(bInd); }
        );
        _underlyingMapT::FindHashParameters(
            funcs,
            [](const int ind, const RookMapGenerator::MasksT&m)
            {
                return RookMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m);
            },
            [](const int bInd) { return RookMapGenerator::InitMasks(bInd); }
        );
    }

    // ------------------------------
    // class fields
    // ------------------------------

private:
    static constexpr _hashFuncT funcs[Board::BoardFields]{
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 256)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 256)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 256)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 256)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 128)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
        _hashFuncT(std::make_tuple(1LLU, 1LLU, 64)),
    };

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //MODULOFREEROOKMAP_H
