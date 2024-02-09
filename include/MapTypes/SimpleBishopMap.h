//
// Created by Jlisowskyy on 2/5/24.
//

#ifndef SIMPLEBISHOPMAP_H
#define SIMPLEBISHOPMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/BishopMapGenerator.h"

class SimpleBishopMap {
    using _hashFuncT = BaseHashFunction<true>;
    using _underlyingMapT = movesHashMap<_hashFuncT>;
public:
    constexpr SimpleBishopMap() {
        for (int i = 0; i < Board::BoardFields; ++i) {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(BishopMapGenerator::InitMasks(boardIndex), funcs[i]);
            MoveInitializer(_maps[i],
                [](const uint64_t n, const int ind) constexpr { return BishopMapGenerator::GenMoves(n, ind); },
                [](const int ind, const BishopMapGenerator::MasksT& m) constexpr { return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
                [](const uint64_t b, const BishopMapGenerator::MasksT& m) constexpr { return b; },
                boardIndex
            );
        }
    }

    [[nodiscard]] constexpr uint64_t GetMoves(const int msbInd, const uint64_t fullBoard) const {
        const uint64_t closestNeighbors = BishopMapGenerator::StripBlockingNeighbors(fullBoard, _maps[msbInd].masks);

        return _maps[msbInd][closestNeighbors];
    }

    static void ParameterSearch() {
    	_underlyingMapT::FindCollidingIndices(
			funcs,
			[](const int ind, const BishopMapGenerator::MasksT& m) { return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
			[](const int bInd) { return BishopMapGenerator::InitMasks(bInd); }
		);
        _underlyingMapT::FindHashParameters(
            funcs,
            [](const int ind, const BishopMapGenerator::MasksT& m) { return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m); },
            [](const int bInd) { return BishopMapGenerator::InitMasks(bInd); }
        );
    }

    // ------------------------------
    // class fields
    // ------------------------------

private:

    static constexpr _hashFuncT funcs[Board::BoardFields] {
		_hashFuncT(std::make_tuple(2540615534426442431LLU, 18161687812890534152LLU, 11, 8)),
		_hashFuncT(std::make_tuple(11237402613493751323LLU, 8003405968917746827LLU, 11, 8)),
		_hashFuncT(std::make_tuple(9510716373887163911LLU, 3524171947865153647LLU, 17, 16)),
		_hashFuncT(std::make_tuple(6449400174864287381LLU, 7356523049413390067LLU, 17, 16)),
		_hashFuncT(std::make_tuple(12665527288850002160LLU, 3174498411009562157LLU, 17, 16)),
		_hashFuncT(std::make_tuple(4853634482874923740LLU, 10774850018386523877LLU, 17, 16)),
		_hashFuncT(std::make_tuple(18188589664130705605LLU, 15631639931568946807LLU, 11, 8)),
		_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
		_hashFuncT(std::make_tuple(8693478209756545513LLU, 4506678441337439676LLU, 11, 8)),
		_hashFuncT(std::make_tuple(4585269150775790892LLU, 7807210325974365171LLU, 11, 8)),
		_hashFuncT(std::make_tuple(7869510348008695377LLU, 7812956633162111118LLU, 17, 16)),
		_hashFuncT(std::make_tuple(1443005652786245088LLU, 17157700586518416221LLU, 17, 16)),
		_hashFuncT(std::make_tuple(13031468250171439383LLU, 16910704318291963571LLU, 17, 16)),
		_hashFuncT(std::make_tuple(669930446377799544LLU, 4923447669805025047LLU, 17, 16)),
		_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
		_hashFuncT(std::make_tuple(10139111774217651801LLU, 14841578872426480958LLU, 11, 8)),
		_hashFuncT(std::make_tuple(1476600364530001388LLU, 2707978354268175262LLU, 17, 16)),
		_hashFuncT(std::make_tuple(12022167654595185988LLU, 7833534493593215760LLU, 17, 16)),
		_hashFuncT(std::make_tuple(15648169580968091049LLU, 7969108857820326880LLU, 67, 64)),
		_hashFuncT(std::make_tuple(9609937080274780492LLU, 11889220498155803180LLU, 67, 64)),
		_hashFuncT(std::make_tuple(6350550981077548190LLU, 16608938017224185812LLU, 67, 64)),
		_hashFuncT(std::make_tuple(3538085614934284469LLU, 16552676330972419759LLU, 67, 64)),
		_hashFuncT(std::make_tuple(14267016668354064020LLU, 16524658945140172068LLU, 17, 16)),
		_hashFuncT(std::make_tuple(12815490474586626762LLU, 680652790632923090LLU, 17, 16)),
		_hashFuncT(std::make_tuple(3922129198458549334LLU, 7272362383016809233LLU, 17, 16)),
		_hashFuncT(std::make_tuple(16385448147481807970LLU, 4622851296883138035LLU, 17, 16)),
		_hashFuncT(std::make_tuple(3958742332006820590LLU, 8446035271446045565LLU, 67, 64)),
		_hashFuncT(std::make_tuple(9967921032413118474LLU, 12841905791811288805LLU, 409, 256)),
		_hashFuncT(std::make_tuple(11959344940195839101LLU, 8569102404371456750LLU, 409, 256)),
		_hashFuncT(std::make_tuple(10040813601232188958LLU, 11356681740285351514LLU, 67, 64)),
		_hashFuncT(std::make_tuple(1851700347384103537LLU, 10227471552307425414LLU, 17, 16)),
		_hashFuncT(std::make_tuple(8797462982713812267LLU, 13539371665054356133LLU, 17, 16)),
		_hashFuncT(std::make_tuple(14518322410693734715LLU, 5421747311149177455LLU, 17, 16)),
		_hashFuncT(std::make_tuple(14217904137666045493LLU, 15719719776421088361LLU, 17, 16)),
		_hashFuncT(std::make_tuple(13322123914603482531LLU, 2602283371023497962LLU, 67, 64)),
		_hashFuncT(std::make_tuple(10239359870228509483LLU, 5296833928661566072LLU, 409, 256)),
		_hashFuncT(std::make_tuple(968467059996833866LLU, 14513346619610276223LLU, 409, 256)),
		_hashFuncT(std::make_tuple(4523455739935528873LLU, 8908615599679280237LLU, 67, 64)),
		_hashFuncT(std::make_tuple(5573301318063977032LLU, 2701862138983212730LLU, 17, 16)),
		_hashFuncT(std::make_tuple(17214074100799420005LLU, 7959085165170967323LLU, 17, 16)),
		_hashFuncT(std::make_tuple(12852543134120427076LLU, 11784632790372871805LLU, 17, 16)),
		_hashFuncT(std::make_tuple(18375345875711339538LLU, 18076858531866717577LLU, 17, 16)),
		_hashFuncT(std::make_tuple(2177580236670390454LLU, 4680939446757822130LLU, 67, 64)),
		_hashFuncT(std::make_tuple(2650344903462236261LLU, 6678320330652363795LLU, 67, 64)),
		_hashFuncT(std::make_tuple(11150643198235197004LLU, 14215969628433602951LLU, 67, 64)),
		_hashFuncT(std::make_tuple(2075560062482038899LLU, 559660358426074468LLU, 67, 64)),
		_hashFuncT(std::make_tuple(1612464035373764859LLU, 2127777402723835363LLU, 17, 16)),
		_hashFuncT(std::make_tuple(13050628286523854510LLU, 17796907330246668654LLU, 17, 16)),
		_hashFuncT(std::make_tuple(8040536374503802636LLU, 17426158046694747315LLU, 11, 8)),
		_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
		_hashFuncT(std::make_tuple(13648204158739951649LLU, 9605820949851826902LLU, 17, 16)),
		_hashFuncT(std::make_tuple(6429857502726729480LLU, 12551714209634267238LLU, 17, 16)),
		_hashFuncT(std::make_tuple(12164813425250202419LLU, 7122480647158944918LLU, 17, 16)),
		_hashFuncT(std::make_tuple(16700916157739100286LLU, 5530401310111808109LLU, 17, 16)),
		_hashFuncT(std::make_tuple(16293778903924772813LLU, 4535889864834435859LLU, 11, 8)),
		_hashFuncT(std::make_tuple(17917281977730077063LLU, 14776515324044429469LLU, 11, 8)),
		_hashFuncT(std::make_tuple(1LLU, 1LLU, 11, 8)),
		_hashFuncT(std::make_tuple(15315379478204117404LLU, 3892842113265763606LLU, 11, 8)),
		_hashFuncT(std::make_tuple(14924773218719524931LLU, 6840170782401226766LLU, 17, 16)),
		_hashFuncT(std::make_tuple(5234793116569222334LLU, 9845633416897833779LLU, 17, 16)),
		_hashFuncT(std::make_tuple(14825703494884394527LLU, 9675555754333295987LLU, 17, 16)),
		_hashFuncT(std::make_tuple(12641721242244554569LLU, 18420025087681921289LLU, 17, 16)),
		_hashFuncT(std::make_tuple(1784114987559604607LLU, 3901291902090622650LLU, 11, 8)),
		_hashFuncT(std::make_tuple(9974993581423446208LLU, 11588535016452207957LLU, 11, 8)),
	};

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //SIMPLEBISHOPMAP_H
