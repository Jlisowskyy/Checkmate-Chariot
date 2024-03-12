//
// Created by Jlisowskyy on 2/9/24.
//

#ifndef MODULOFREENOOFFSETBISHOPMAP_H
#define MODULOFREENOOFFSETBISHOPMAP_H

#include "../EngineTypeDefs.h"
#include "../MoveGeneration/BishopMapGenerator.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"

class ModuloFreeBishopMapNoOffset
{
    using _hashFuncT = Fast2PowHashFunctionNoOffset<>;
    using _underlyingMapT = movesHashMap<_hashFuncT>;

   public:
    constexpr ModuloFreeBishopMapNoOffset()
    {
        for (int i = 0; i < Board::BoardFields; ++i)
        {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(BishopMapGenerator::InitMasks(boardIndex), funcs[i]);
            MoveInitializer(
                _maps[i],
                [](const uint64_t n, const int ind) constexpr { return BishopMapGenerator::GenMoves(n, ind); },
                [](const int ind, const BishopMapGenerator::MasksT& m) constexpr
                { return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
                [](const uint64_t b, const BishopMapGenerator::MasksT& m) constexpr { return b; }, boardIndex);
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const
    {
        const uint64_t closestNeighbors = BishopMapGenerator::StripBlockingNeighbors(fullBoard, _maps[msbInd].masks);

        return _maps[msbInd][closestNeighbors];
    }

    static void ParameterSearch()
    {
        _underlyingMapT::FindCollidingIndices(
            funcs,
            [](const int ind, const BishopMapGenerator::MasksT& m)
            { return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
            [](const int bInd) { return BishopMapGenerator::InitMasks(bInd); });
        _underlyingMapT::FindHashParameters(
            funcs,
            [](const int ind, const BishopMapGenerator::MasksT& m)
            { return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
            [](const int bInd) { return BishopMapGenerator::InitMasks(bInd); });
    }

    // ------------------------------
    // class fields
    // ------------------------------

   private:
    static constexpr _hashFuncT funcs[Board::BoardFields]{
        _hashFuncT(std::make_tuple(215894281180193496734374473585231955024_uint128_t, 8)),
        _hashFuncT(std::make_tuple(195467650778217393535906139774567944459_uint128_t, 8)),
        _hashFuncT(std::make_tuple(216293696427964814876354368037875992109_uint128_t, 16)),
        _hashFuncT(std::make_tuple(221252076198217889008052025565173179723_uint128_t, 16)),
        _hashFuncT(std::make_tuple(78063702419130794175034148960880185404_uint128_t, 16)),
        _hashFuncT(std::make_tuple(261184431236441410214469446450560983213_uint128_t, 16)),
        _hashFuncT(std::make_tuple(130259512163869280381829266111049856240_uint128_t, 8)),
        _hashFuncT(std::make_tuple(229705672858226217555706002147217834791_uint128_t, 8)),
        _hashFuncT(std::make_tuple(36387597240404961182071002060391349791_uint128_t, 8)),
        _hashFuncT(std::make_tuple(304489203880393343170719583493203022979_uint128_t, 8)),
        _hashFuncT(std::make_tuple(23243878553610810220070957605132082658_uint128_t, 16)),
        _hashFuncT(std::make_tuple(170535116180251447490357529663287681223_uint128_t, 16)),
        _hashFuncT(std::make_tuple(13541508716058961453260335429800471304_uint128_t, 16)),
        _hashFuncT(std::make_tuple(67927662667462473138177295836391544133_uint128_t, 16)),
        _hashFuncT(std::make_tuple(185054539111794877726391167865063808948_uint128_t, 8)),
        _hashFuncT(std::make_tuple(267379402383934096276172444492721981183_uint128_t, 8)),
        _hashFuncT(std::make_tuple(16760536788819548464515564604232472011_uint128_t, 16)),
        _hashFuncT(std::make_tuple(281722506308733130491843997272252041606_uint128_t, 16)),
        _hashFuncT(std::make_tuple(105633044496709543938380876902728702526_uint128_t, 64)),
        _hashFuncT(std::make_tuple(179870716037005282228407977320793270426_uint128_t, 64)),
        _hashFuncT(std::make_tuple(217686986684656824771101710948509122774_uint128_t, 64)),
        _hashFuncT(std::make_tuple(274638767547895136945409514350861605225_uint128_t, 64)),
        _hashFuncT(std::make_tuple(336717791016782867575349303151936952905_uint128_t, 16)),
        _hashFuncT(std::make_tuple(136189481133986109468620807779230503316_uint128_t, 16)),
        _hashFuncT(std::make_tuple(130265802213143556730735727739271363775_uint128_t, 16)),
        _hashFuncT(std::make_tuple(304086186888322614704822493631911725578_uint128_t, 16)),
        _hashFuncT(std::make_tuple(260152563983463311536153820545900113433_uint128_t, 64)),
        _hashFuncT(std::make_tuple(154367012366300566677565711922078439698_uint128_t, 256)),
        _hashFuncT(std::make_tuple(94625415908108555462321900155842422324_uint128_t, 256)),
        _hashFuncT(std::make_tuple(23833186854653972850854309829739680966_uint128_t, 64)),
        _hashFuncT(std::make_tuple(244005689092090111261192456499530838943_uint128_t, 16)),
        _hashFuncT(std::make_tuple(236043892044409265243669226440931537382_uint128_t, 16)),
        _hashFuncT(std::make_tuple(250053736590874717497676863300808178451_uint128_t, 16)),
        _hashFuncT(std::make_tuple(98085532484890192648736821081730166878_uint128_t, 16)),
        _hashFuncT(std::make_tuple(63108688292415250606950226343111510115_uint128_t, 64)),
        _hashFuncT(std::make_tuple(166175087484336518180525617287946208277_uint128_t, 256)),
        _hashFuncT(std::make_tuple(204817231446748232468838893463006200707_uint128_t, 256)),
        _hashFuncT(std::make_tuple(228043352945701243002338663926761228227_uint128_t, 64)),
        _hashFuncT(std::make_tuple(319444979798627227026975886395823237035_uint128_t, 16)),
        _hashFuncT(std::make_tuple(71666402692521581046945106821474791346_uint128_t, 16)),
        _hashFuncT(std::make_tuple(79729205086600145096213204761748208708_uint128_t, 16)),
        _hashFuncT(std::make_tuple(131720748879027839481286073970129593682_uint128_t, 16)),
        _hashFuncT(std::make_tuple(56672308482482982251204140626957828129_uint128_t, 64)),
        _hashFuncT(std::make_tuple(168736470557539068558384059700951157938_uint128_t, 64)),
        _hashFuncT(std::make_tuple(134303057220757703460376163295631748395_uint128_t, 64)),
        _hashFuncT(std::make_tuple(72230259460768157799035555547476744809_uint128_t, 64)),
        _hashFuncT(std::make_tuple(281330727488060756837029405734346312271_uint128_t, 16)),
        _hashFuncT(std::make_tuple(7597087403753462894732707886724935756_uint128_t, 16)),
        _hashFuncT(std::make_tuple(78140377372107369766385524655580372634_uint128_t, 8)),
        _hashFuncT(std::make_tuple(301163576519616040003429176845963768203_uint128_t, 8)),
        _hashFuncT(std::make_tuple(331886540760497397189296333576010955428_uint128_t, 16)),
        _hashFuncT(std::make_tuple(99582173600700082287402348070317695822_uint128_t, 16)),
        _hashFuncT(std::make_tuple(243646773286125712640879884081148722646_uint128_t, 16)),
        _hashFuncT(std::make_tuple(283278512158881732911756590161442413778_uint128_t, 16)),
        _hashFuncT(std::make_tuple(85326626678594822206662547888830331848_uint128_t, 8)),
        _hashFuncT(std::make_tuple(330134143441032332147946672666582681863_uint128_t, 8)),
        _hashFuncT(std::make_tuple(76591022172696115192869373777087006244_uint128_t, 8)),
        _hashFuncT(std::make_tuple(163964474733126403093354009913792804855_uint128_t, 8)),
        _hashFuncT(std::make_tuple(127898352071587193417232817033645752245_uint128_t, 16)),
        _hashFuncT(std::make_tuple(68266339347261692840222218268469952278_uint128_t, 16)),
        _hashFuncT(std::make_tuple(74975431029705028731850328523218496443_uint128_t, 16)),
        _hashFuncT(std::make_tuple(48558266784182296429846042779253176790_uint128_t, 16)),
        _hashFuncT(std::make_tuple(285065950783304450633856597292051184421_uint128_t, 8)),
        _hashFuncT(std::make_tuple(31203352483911989890009866398761643486_uint128_t, 8)),
    };

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif  // MODULOFREENOOFFSETBISHOPMAP_H
