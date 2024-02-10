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
		_hashFuncT(std::make_tuple(3611869282901619919LLU, 6)),
		_hashFuncT(std::make_tuple(18228596997040662761LLU, 5)),
		_hashFuncT(std::make_tuple(6460947485659077082LLU, 5)),
		_hashFuncT(std::make_tuple(13752940679180250076LLU, 5)),
		_hashFuncT(std::make_tuple(4241559519926323204LLU, 5)),
		_hashFuncT(std::make_tuple(208466150794661613LLU, 5)),
		_hashFuncT(std::make_tuple(13573396270746166127LLU, 5)),
		_hashFuncT(std::make_tuple(15433831379842884746LLU, 6)),
		_hashFuncT(std::make_tuple(5458137257786957413LLU, 5)),
		_hashFuncT(std::make_tuple(15317014978965832668LLU, 5)),
		_hashFuncT(std::make_tuple(3986727869601871670LLU, 5)),
		_hashFuncT(std::make_tuple(6197114153223970289LLU, 5)),
		_hashFuncT(std::make_tuple(6397499356811370666LLU, 5)),
		_hashFuncT(std::make_tuple(4535982479599701613LLU, 5)),
		_hashFuncT(std::make_tuple(17376566781982244960LLU, 5)),
		_hashFuncT(std::make_tuple(3266463099269102052LLU, 5)),
		_hashFuncT(std::make_tuple(5896716211751964931LLU, 5)),
		_hashFuncT(std::make_tuple(243028095806079859LLU, 5)),
		_hashFuncT(std::make_tuple(15702874870926707967LLU, 7)),
		_hashFuncT(std::make_tuple(11613292506489506816LLU, 7)),
		_hashFuncT(std::make_tuple(12494788516674558978LLU, 7)),
		_hashFuncT(std::make_tuple(13163493309466603516LLU, 7)),
		_hashFuncT(std::make_tuple(5001228016187988525LLU, 5)),
		_hashFuncT(std::make_tuple(10169910673128752515LLU, 5)),
		_hashFuncT(std::make_tuple(7397172614331155718LLU, 5)),
		_hashFuncT(std::make_tuple(15882937967404034260LLU, 5)),
		_hashFuncT(std::make_tuple(10402694421956853558LLU, 7)),
		_hashFuncT(std::make_tuple(4069501445375656064LLU, 9)),
		_hashFuncT(std::make_tuple(10083843241836806529LLU, 9)),
		_hashFuncT(std::make_tuple(6266797948639643237LLU, 7)),
		_hashFuncT(std::make_tuple(16797423475124582351LLU, 5)),
		_hashFuncT(std::make_tuple(5261299203380555532LLU, 5)),
		_hashFuncT(std::make_tuple(10054851337067495605LLU, 5)),
		_hashFuncT(std::make_tuple(18231622418956876490LLU, 5)),
		_hashFuncT(std::make_tuple(1820992464513543679LLU, 7)),
		_hashFuncT(std::make_tuple(12483134911496110085LLU, 9)),
		_hashFuncT(std::make_tuple(1223299049642410168LLU, 9)),
		_hashFuncT(std::make_tuple(4888956467406968000LLU, 7)),
		_hashFuncT(std::make_tuple(5015872955111408367LLU, 5)),
		_hashFuncT(std::make_tuple(7034782627490706770LLU, 5)),
		_hashFuncT(std::make_tuple(17400956056348401736LLU, 5)),
		_hashFuncT(std::make_tuple(4619649999728459497LLU, 5)),
		_hashFuncT(std::make_tuple(5975714137870106383LLU, 7)),
		_hashFuncT(std::make_tuple(2932970949829467478LLU, 7)),
		_hashFuncT(std::make_tuple(2902574806744450484LLU, 7)),
		_hashFuncT(std::make_tuple(1688859317807109378LLU, 7)),
		_hashFuncT(std::make_tuple(6061795778347790648LLU, 5)),
		_hashFuncT(std::make_tuple(18307229971900796838LLU, 5)),
		_hashFuncT(std::make_tuple(17965068854548627547LLU, 5)),
		_hashFuncT(std::make_tuple(16628456126524129413LLU, 5)),
		_hashFuncT(std::make_tuple(10905209255181491571LLU, 5)),
		_hashFuncT(std::make_tuple(15279504055131985538LLU, 5)),
		_hashFuncT(std::make_tuple(5042597873833943191LLU, 5)),
		_hashFuncT(std::make_tuple(10685254548740368231LLU, 5)),
		_hashFuncT(std::make_tuple(5873211263693006843LLU, 5)),
		_hashFuncT(std::make_tuple(6554152907125806591LLU, 5)),
		_hashFuncT(std::make_tuple(17911104566945579206LLU, 6)),
		_hashFuncT(std::make_tuple(2395073496242704814LLU, 5)),
		_hashFuncT(std::make_tuple(12909519480940001285LLU, 5)),
		_hashFuncT(std::make_tuple(16968431856099893139LLU, 5)),
		_hashFuncT(std::make_tuple(6726376817723420599LLU, 5)),
		_hashFuncT(std::make_tuple(13044608236556692757LLU, 5)),
		_hashFuncT(std::make_tuple(4783150144548767711LLU, 5)),
		_hashFuncT(std::make_tuple(18381131039969901408LLU, 6)),
	};
    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //FANCYMAGICBISHOPMAP_H
