//
// Created by Jlisowskyy on 2/10/24.
//

#ifndef FANCYMAGICROOKMAP_H
#define FANCYMAGICROOKMAP_H

#include "../EngineTypeDefs.h"
#include "../MoveGeneration/RookMapGenerator.h"
#include "../MoveGeneration/SparseRandomGenerator.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"

class FancyMagicRookMap
{
    using _hashFuncT = FancyMagicHashFunction<SparseRandomGenerator<>>;
    using _underlyingMapT = movesHashMap<_hashFuncT, RookMapGenerator::MaxRookPossibleNeighborsWithOverlap>;

   public:
    constexpr FancyMagicRookMap()
    {
        for (int i = 0; i < static_cast<int>(Board::BoardFields); ++i)
        {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(RookMapGenerator::InitMasks(boardIndex), funcs[i]);
            _maps[i].InitFullMask();

            MoveInitializer(
                _maps[i], [](const uint64_t n, const int ind) constexpr { return RookMapGenerator::GenMoves(n, ind); },
                []([[maybe_unused]] const int, const RookMapGenerator::MasksT& m) constexpr
                { return RookMapGenerator::GenPossibleNeighborsWithOverlap(m); },
                [](const uint64_t b, const RookMapGenerator::MasksT& m) constexpr
                { return RookMapGenerator::StripBlockingNeighbors(b, m); },
                boardIndex);
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const
    {
        const uint64_t neighbors = fullBoard & _maps[msbInd].fullMask;
        return _maps[msbInd][neighbors];
    }

    static void ParameterSearch()
    {
        auto nGen = []([[maybe_unused]] const int, const RookMapGenerator::MasksT& m)
        { return RookMapGenerator::GenPossibleNeighborsWithOverlap(m); };
        auto mInit = [](const int bInd) { return RookMapGenerator::InitMasks(bInd); };

        _underlyingMapT::FindCollidingIndices<decltype(nGen), decltype(mInit),
                                              [](const uint64_t n, const std::array<uint64_t, 4>& m) {
                                                  return RookMapGenerator::StripBlockingNeighbors(n, m);
                                              }>(funcs, nGen, mInit);

        _underlyingMapT::FindHashParameters<decltype(nGen), decltype(mInit),
                                            [](const uint64_t n, const std::array<uint64_t, 4>& m) {
                                                return RookMapGenerator::StripBlockingNeighbors(n, m);
                                            }>(funcs, nGen, mInit);
    }

    // ------------------------------
    // class fields
    // ------------------------------

   private:
    static constexpr _hashFuncT funcs[Board::BoardFields]{
        _hashFuncT(std::make_tuple(1170940307609551394LLU, 12)),
        _hashFuncT(std::make_tuple(864693331908632740LLU, 11)),
        _hashFuncT(std::make_tuple(18577430269659234LLU, 11)),
        _hashFuncT(std::make_tuple(4621256752021644546LLU, 11)),
        _hashFuncT(std::make_tuple(4611721383624327186LLU, 11)),
        _hashFuncT(std::make_tuple(4631389405377988738LLU, 11)),
        _hashFuncT(std::make_tuple(2402707786700033LLU, 11)),
        _hashFuncT(std::make_tuple(9367534510373093457LLU, 12)),
        _hashFuncT(std::make_tuple(1191778255215034880LLU, 11)),
        _hashFuncT(std::make_tuple(283708634530816LLU, 10)),
        _hashFuncT(std::make_tuple(1190076220893593728LLU, 10)),
        _hashFuncT(std::make_tuple(288234784937214080LLU, 10)),
        _hashFuncT(std::make_tuple(27179931734188288LLU, 10)),
        _hashFuncT(std::make_tuple(158608849371392LLU, 10)),
        _hashFuncT(std::make_tuple(9148211623101056LLU, 10)),
        _hashFuncT(std::make_tuple(36073604282188288LLU, 11)),
        _hashFuncT(std::make_tuple(720576232445575169LLU, 11)),
        _hashFuncT(std::make_tuple(72620647137804339LLU, 10)),
        _hashFuncT(std::make_tuple(288276555707220096LLU, 10)),
        _hashFuncT(std::make_tuple(72066394427031564LLU, 10)),
        _hashFuncT(std::make_tuple(10150704251469832LLU, 10)),
        _hashFuncT(std::make_tuple(9007341006553152LLU, 10)),
        _hashFuncT(std::make_tuple(9011917007896576LLU, 10)),
        _hashFuncT(std::make_tuple(36046526648172545LLU, 11)),
        _hashFuncT(std::make_tuple(2314859571530694741LLU, 11)),
        _hashFuncT(std::make_tuple(13835075682431926929LLU, 10)),
        _hashFuncT(std::make_tuple(9800959238896353792LLU, 10)),
        _hashFuncT(std::make_tuple(2542073114792192LLU, 10)),
        _hashFuncT(std::make_tuple(704855681271808LLU, 10)),
        _hashFuncT(std::make_tuple(9011057804709890LLU, 10)),
        _hashFuncT(std::make_tuple(4527926326329920LLU, 10)),
        _hashFuncT(std::make_tuple(70369306214794LLU, 11)),
        _hashFuncT(std::make_tuple(4440998332545LLU, 11)),
        _hashFuncT(std::make_tuple(288511992862474244LLU, 10)),
        _hashFuncT(std::make_tuple(577024277784079440LLU, 10)),
        _hashFuncT(std::make_tuple(148623187905611776LLU, 10)),
        _hashFuncT(std::make_tuple(1229482732766167168LLU, 10)),
        _hashFuncT(std::make_tuple(306279963332448516LLU, 10)),
        _hashFuncT(std::make_tuple(9259611966951919616LLU, 10)),
        _hashFuncT(std::make_tuple(145275121385472LLU, 11)),
        _hashFuncT(std::make_tuple(1198467691593876LLU, 11)),
        _hashFuncT(std::make_tuple(9241390833545056354LLU, 10)),
        _hashFuncT(std::make_tuple(2342154380759531528LLU, 10)),
        _hashFuncT(std::make_tuple(1155314591798952960LLU, 10)),
        _hashFuncT(std::make_tuple(72198881550796800LLU, 10)),
        _hashFuncT(std::make_tuple(39549433821532224LLU, 10)),
        _hashFuncT(std::make_tuple(9227875911897071616LLU, 10)),
        _hashFuncT(std::make_tuple(4647715090393464896LLU, 11)),
        _hashFuncT(std::make_tuple(72198332616806528LLU, 11)),
        _hashFuncT(std::make_tuple(5774178050400911872LLU, 10)),
        _hashFuncT(std::make_tuple(140754668355968LLU, 10)),
        _hashFuncT(std::make_tuple(9223512808711281664LLU, 10)),
        _hashFuncT(std::make_tuple(4612249011331809312LLU, 10)),
        _hashFuncT(std::make_tuple(11854037274989707392LLU, 10)),
        _hashFuncT(std::make_tuple(108719710863888384LLU, 10)),
        _hashFuncT(std::make_tuple(130886003756958016LLU, 11)),
        _hashFuncT(std::make_tuple(4359484989186785536LLU, 12)),
        _hashFuncT(std::make_tuple(288232592490112169LLU, 11)),
        _hashFuncT(std::make_tuple(1225005641546989824LLU, 11)),
        _hashFuncT(std::make_tuple(9871908036182822144LLU, 11)),
        _hashFuncT(std::make_tuple(5044036051065126920LLU, 11)),
        _hashFuncT(std::make_tuple(612507141651046400LLU, 11)),
        _hashFuncT(std::make_tuple(18016666254314304LLU, 11)),
        _hashFuncT(std::make_tuple(612489824202555536LLU, 12)),
    };

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif  // FANCYMAGICROOKMAP_H
