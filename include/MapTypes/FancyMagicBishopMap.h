//
// Created by Jlisowskyy on 2/10/24.
//

#ifndef FANCYMAGICBISHOPMAP_H
#define FANCYMAGICBISHOPMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/BishopMapGenerator.h"

class FancyMagicBishopMap {
    using _hashFuncT = FancyMagicHashFunction<>;
    using _underlyingMapT = movesHashMap<_hashFuncT, 512>;
public:
    constexpr FancyMagicBishopMap() {
        for (int i = 0; i < Board::BoardFields; ++i) {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(BishopMapGenerator::InitMasks(boardIndex), funcs[i]);
			_maps[i].InitFullMask();

            MoveInitializer(_maps[i],
                [](const uint64_t n, const int ind) constexpr { return BishopMapGenerator::GenMoves(n, ind); },
                [](const int ind, const BishopMapGenerator::MasksT& m) constexpr { return BishopMapGenerator::GenPossibleNeighborsWithOverlap(m); },
                [](const uint64_t b, const BishopMapGenerator::MasksT& m) constexpr { return BishopMapGenerator::StripBlockingNeighbors(b, m); },
                boardIndex
            );
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const {
        const uint64_t neighbors = fullBoard & _maps[msbInd].fullMask;
        return _maps[msbInd][neighbors];
    }

    static void ParameterSearch() {
		auto nGen = [](const int _, const BishopMapGenerator::MasksT& m) { return BishopMapGenerator::GenPossibleNeighborsWithOverlap(m); };
    	auto mInit = [](const int bInd) { return BishopMapGenerator::InitMasks(bInd); };

    	_underlyingMapT::FindCollidingIndices<
    		decltype(nGen),
    		decltype(mInit),
    		[](const uint64_t n, const std::array<uint64_t, 4>& m) { return BishopMapGenerator::StripBlockingNeighbors(n, m); }
    	>(
			funcs,
			nGen,
			mInit
		);

    	_underlyingMapT::FindHashParameters<
			decltype(nGen),
			decltype(mInit),
			[](const uint64_t n, const std::array<uint64_t, 4>& m) { return BishopMapGenerator::StripBlockingNeighbors(n, m); }
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
		_hashFuncT(std::make_tuple(1LLU, 6)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 6)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 9)),
		_hashFuncT(std::make_tuple(1LLU, 9)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 9)),
		_hashFuncT(std::make_tuple(1LLU, 9)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 7)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 6)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 5)),
		_hashFuncT(std::make_tuple(1LLU, 6)),
	};

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //FANCYMAGICBISHOPMAP_H
