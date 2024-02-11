//
// Created by Jlisowskyy on 2/10/24.
//

#ifndef FANCYMAGICROOKMAP_H
#define FANCYMAGICROOKMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/RookMapGenerator.h"
#include "../MoveGeneration/SparseRandomGenerator.h"

class FancyMagicRookMap {
    using _hashFuncT = FancyMagicHashFunction<SparseRandomGenerator<>>;
    using _underlyingMapT = movesHashMap<_hashFuncT, 512>;
public:
    constexpr FancyMagicRookMap() {
        for (int i = 0; i < Board::BoardFields; ++i) {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(RookMapGenerator::InitMasks(boardIndex), funcs[i]);
			_maps[i].InitFullMask();

            MoveInitializer(_maps[i],
                [](const uint64_t n, const int ind) constexpr { return RookMapGenerator::GenMoves(n, ind); },
                [](const int ind, const RookMapGenerator::MasksT& m) constexpr { return RookMapGenerator::GenPossibleNeighborsWithOverlap(m); },
                [](const uint64_t b, const RookMapGenerator::MasksT& m) constexpr { return RookMapGenerator::StripBlockingNeighbors(b, m); },
                boardIndex
            );
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const {
        const uint64_t neighbors = fullBoard & _maps[msbInd].fullMask;
        return _maps[msbInd][neighbors];
    }

    static void ParameterSearch() {
		auto nGen = [](const int _, const RookMapGenerator::MasksT& m) { return RookMapGenerator::GenPossibleNeighborsWithOverlap(m); };
    	auto mInit = [](const int bInd) { return RookMapGenerator::InitMasks(bInd); };

    	_underlyingMapT::FindCollidingIndices<
    		decltype(nGen),
    		decltype(mInit),
    		[](const uint64_t n, const std::array<uint64_t, 4>& m) { return RookMapGenerator::StripBlockingNeighbors(n, m); }
    	>(
			funcs,
			nGen,
			mInit
		);

    	_underlyingMapT::FindHashParameters<
			decltype(nGen),
			decltype(mInit),
			[](const uint64_t n, const std::array<uint64_t, 4>& m) { return RookMapGenerator::StripBlockingNeighbors(n, m); }
    	>(
            funcs,
            nGen,
            mInit
        );
    }

    // ------------------------------
    // class fields
    // ------------------------------

private:

    static constexpr _hashFuncT funcs[Board::BoardFields] {
        _hashFuncT(std::make_tuple(1LLU, 12)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 12)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)), 
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 10)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 12)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 11)),
        _hashFuncT(std::make_tuple(1LLU, 12)),
};

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //FANCYMAGICROOKMAP_H
