//
// Created by Jlisowskyy on 2/9/24.
//

#ifndef MODULOFREEBISHOPMAP_H
#define MODULOFREEBISHOPMAP_H

#include "../EngineTypeDefs.h"
#include "../movesHashMap.h"
#include "HashFunctions.h"
#include "../MoveGeneration/BishopMapGenerator.h"

class ModuloFreeBishopMap
{
    using _hashFuncT = Fast2PowHashFunction<>;
    using _underlyingMapT = movesHashMap<_hashFuncT>;

public:
    constexpr ModuloFreeBishopMap()
    {
        for (int i = 0; i < Board::BoardFields; ++i)
        {
            const int boardIndex = ConvertToReversedPos(i);

            _maps[i] = _underlyingMapT(BishopMapGenerator::InitMasks(boardIndex), funcs[i]);
            MoveInitializer(_maps[i],
                            [](const uint64_t n, const int ind) constexpr
                            {
                                return BishopMapGenerator::GenMoves(n, ind);
                            },
                            [](const int ind, const BishopMapGenerator::MasksT&m) constexpr
                            {
                                return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m);
                            },
                            [](const uint64_t b, const BishopMapGenerator::MasksT&m) constexpr { return b; },
                            boardIndex
            );
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
            [](const int ind, const BishopMapGenerator::MasksT&m)
            {
                return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m);
            },
            [](const int bInd) { return BishopMapGenerator::InitMasks(bInd); }
        );
        _underlyingMapT::FindHashParameters(
            funcs,
            [](const int ind, const BishopMapGenerator::MasksT&m)
            {
                return BishopMapGenerator::GenPossibleNeighborsWoutOverlap(ind, m);
            },
            [](const int bInd) { return BishopMapGenerator::InitMasks(bInd); }
        );
    }

    // ------------------------------
    // class fields
    // ------------------------------

private:
    static constexpr _hashFuncT funcs[Board::BoardFields]{
        _hashFuncT(std::make_tuple(266043027777574782973635861182587003313_uint128_t,
                                   238517668359387894087658987839636362141_uint128_t, 8)),
        _hashFuncT(std::make_tuple(129360080686236685331059738277301180383_uint128_t,
                                   30348595553190967410278849185103277442_uint128_t, 8)),
        _hashFuncT(std::make_tuple(329194294087454444130849186386769383206_uint128_t,
                                   295583450937212073704460118080565566818_uint128_t, 16)),
        _hashFuncT(std::make_tuple(16905957965211088386542136079668632599_uint128_t,
                                   280371033323315290635294445974707597036_uint128_t, 16)),
        _hashFuncT(std::make_tuple(141845663761185746903261144068446687252_uint128_t,
                                   76193295783862088793933021933624104032_uint128_t, 16)),
        _hashFuncT(std::make_tuple(233300922229284803661308744777717758690_uint128_t,
                                   221653448984185442331515749432823273781_uint128_t, 16)),
        _hashFuncT(std::make_tuple(313901898875267687737341933132390544232_uint128_t,
                                   38220229170487388897452040499319838099_uint128_t, 8)),
        _hashFuncT(std::make_tuple(250562873279360895950662641019871376327_uint128_t,
                                   262274612978290069986941747170896842416_uint128_t, 8)),
        _hashFuncT(std::make_tuple(163450363544370497413552932861311312144_uint128_t,
                                   142691627883217884856912888817649608847_uint128_t, 8)),
        _hashFuncT(std::make_tuple(254193774803989644529457641668324742029_uint128_t,
                                   204747950379119111511065451897338458133_uint128_t, 8)),
        _hashFuncT(std::make_tuple(284316562308004662192880868317750003783_uint128_t,
                                   286413684911227451048926919000941581068_uint128_t, 16)),
        _hashFuncT(std::make_tuple(153539108917360719987401581675944915495_uint128_t,
                                   278499382947226034879644107623724523462_uint128_t, 16)),
        _hashFuncT(std::make_tuple(72474617864321760101653334591045072105_uint128_t,
                                   268144309452916123204078657444229801783_uint128_t, 16)),
        _hashFuncT(std::make_tuple(63838122395036385150710689691038286412_uint128_t,
                                   102982776128906022008190078786340158220_uint128_t, 16)),
        _hashFuncT(std::make_tuple(214356066869456195920846399834509215937_uint128_t,
                                   324029232447533030111941807900235614133_uint128_t, 8)),
        _hashFuncT(std::make_tuple(176218202965029702260166015168742290743_uint128_t,
                                   32233051672661226346244216238850019347_uint128_t, 8)),
        _hashFuncT(std::make_tuple(84922468480391084427705156622218227832_uint128_t,
                                   288262105731202742126800559496951219320_uint128_t, 16)),
        _hashFuncT(std::make_tuple(96930823710176575153739567381419705016_uint128_t,
                                   298909168153788723933625896341715883273_uint128_t, 16)),
        _hashFuncT(std::make_tuple(89840340216256499235513934623432260636_uint128_t,
                                   264051791381489718896562221618593554326_uint128_t, 64)),
        _hashFuncT(std::make_tuple(319603094695135314204959398680154144714_uint128_t,
                                   228542830449050867848568334675981163744_uint128_t, 64)),
        _hashFuncT(std::make_tuple(78029081897178050706674455115393620937_uint128_t,
                                   197852974345406930452405526874006890456_uint128_t, 64)),
        _hashFuncT(std::make_tuple(96304740937368463977876194497296335334_uint128_t,
                                   296528041864445416375836240361250604208_uint128_t, 64)),
        _hashFuncT(std::make_tuple(235319607686439934001084893614324358984_uint128_t,
                                   20593725507985939211991619143754454666_uint128_t, 16)),
        _hashFuncT(std::make_tuple(89774569706558475130870707046326163990_uint128_t,
                                   92919114169754024379247073804584835374_uint128_t, 16)),
        _hashFuncT(std::make_tuple(83967308851054260395839737869585902583_uint128_t,
                                   160110954961496653035135401983414088418_uint128_t, 16)),
        _hashFuncT(std::make_tuple(202972415515580537121870756448477103291_uint128_t,
                                   196776856598081419544783254211201682209_uint128_t, 16)),
        _hashFuncT(std::make_tuple(269004304710674320360408278445791921170_uint128_t,
                                   98571089113415699182631109799535556093_uint128_t, 64)),
        _hashFuncT(std::make_tuple(76295252216125546838076762931414601330_uint128_t,
                                   203046717249977694687399628204433823728_uint128_t, 256)),
        _hashFuncT(std::make_tuple(4486165511643092376164928706565867550_uint128_t,
                                   138716266635335987578496586809889013671_uint128_t, 256)),
        _hashFuncT(std::make_tuple(84631477923837718672118615867043784493_uint128_t,
                                   201566294796021462441344464919811052031_uint128_t, 64)),
        _hashFuncT(std::make_tuple(116257013918210427356022263220545343965_uint128_t,
                                   122062993086569972129460126586298440447_uint128_t, 16)),
        _hashFuncT(std::make_tuple(317803411101034407175369436694602691097_uint128_t,
                                   179057253359645268813359414469897994286_uint128_t, 16)),
        _hashFuncT(std::make_tuple(16972573539653208644672183138172707394_uint128_t,
                                   151677301753761909343613841072932829695_uint128_t, 16)),
        _hashFuncT(std::make_tuple(29331246450412237355229105448581244258_uint128_t,
                                   339274817644723451647915039729433554925_uint128_t, 16)),
        _hashFuncT(std::make_tuple(258510593215053857394247042569482591200_uint128_t,
                                   287355651940751070242818519269489402265_uint128_t, 64)),
        _hashFuncT(std::make_tuple(63651610578179645689534089708336663647_uint128_t,
                                   301129160548128902971010283883442679125_uint128_t, 256)),
        _hashFuncT(std::make_tuple(180487177329107068962121166671369879219_uint128_t,
                                   111182633171932447531542263673998120271_uint128_t, 256)),
        _hashFuncT(std::make_tuple(153057380163329834444967657571905849774_uint128_t,
                                   329517617151777672761383542997120604398_uint128_t, 64)),
        _hashFuncT(std::make_tuple(74405893469770853933255429292295677187_uint128_t,
                                   73326404132848323424973986570847350569_uint128_t, 16)),
        _hashFuncT(std::make_tuple(2573503664755798327593892250010671815_uint128_t,
                                   204109989019135868206127237786203480892_uint128_t, 16)),
        _hashFuncT(std::make_tuple(209630127598986439408600671023865629637_uint128_t,
                                   145778198203691756373460839902110523224_uint128_t, 16)),
        _hashFuncT(std::make_tuple(136626718393823211129397821766991977581_uint128_t,
                                   133019906435152226451633462879720514881_uint128_t, 16)),
        _hashFuncT(std::make_tuple(271246423587906948846583601133756814136_uint128_t,
                                   39171163143182867898227584793660680649_uint128_t, 64)),
        _hashFuncT(std::make_tuple(223173516203499729745509346276329472803_uint128_t,
                                   291670981000011027765789948047107814106_uint128_t, 64)),
        _hashFuncT(std::make_tuple(166035766183143411149973748415246830228_uint128_t,
                                   191687186409960977408827824791459030921_uint128_t, 64)),
        _hashFuncT(std::make_tuple(147729139413509599593825189282412776539_uint128_t,
                                   122974407964092541459298835682542051916_uint128_t, 64)),
        _hashFuncT(std::make_tuple(262535741954897241480323110578743988846_uint128_t,
                                   252239841231439434058630248410682370127_uint128_t, 16)),
        _hashFuncT(std::make_tuple(50500618211224408737143930664110958469_uint128_t,
                                   182627061718736294457218405814323404519_uint128_t, 16)),
        _hashFuncT(std::make_tuple(206799588121109518731351435969704108586_uint128_t,
                                   278616272068504429155566990522551352740_uint128_t, 8)),
        _hashFuncT(std::make_tuple(200074678333644839609905494813002155399_uint128_t,
                                   144829499320361320886582494027847252876_uint128_t, 8)),
        _hashFuncT(std::make_tuple(45764937404098564788802392192708896460_uint128_t,
                                   151012407329384828698915848062942128325_uint128_t, 16)),
        _hashFuncT(std::make_tuple(74546411939065647087496912304194505335_uint128_t,
                                   283466852894856982299931410124190641615_uint128_t, 16)),
        _hashFuncT(std::make_tuple(162258643662644351146529917514314647320_uint128_t,
                                   291771464654208229876805266555213068288_uint128_t, 16)),
        _hashFuncT(std::make_tuple(186152548736076107403311221887580316708_uint128_t,
                                   279477943385810347179004838252223822983_uint128_t, 16)),
        _hashFuncT(std::make_tuple(147586704518090679374680991759504470880_uint128_t,
                                   127220689507804105379500997853448188635_uint128_t, 8)),
        _hashFuncT(std::make_tuple(27530742155889343149396730173624546734_uint128_t,
                                   206109750392966945351911995088436149463_uint128_t, 8)),
        _hashFuncT(std::make_tuple(258660774603147265932486294141397435382_uint128_t,
                                   316257049748829704392353398800007018539_uint128_t, 8)),
        _hashFuncT(std::make_tuple(136754008419454373691928266809403547630_uint128_t,
                                   137715668543862997043384702569897607301_uint128_t, 8)),
        _hashFuncT(std::make_tuple(99890173003660029041552184246412100149_uint128_t,
                                   261578942946360767058948728316204936450_uint128_t, 16)),
        _hashFuncT(std::make_tuple(137513674955086756899820350145372469495_uint128_t,
                                   165187319093383978370555909962330116009_uint128_t, 16)),
        _hashFuncT(std::make_tuple(97744450933298133797131979176193384028_uint128_t,
                                   48540639095788085049646814884618596921_uint128_t, 16)),
        _hashFuncT(std::make_tuple(326390385083086076268507701131263042124_uint128_t,
                                   335295815772714735141362809485607246394_uint128_t, 16)),
        _hashFuncT(std::make_tuple(139404376275829663864170957981269568289_uint128_t,
                                   18360219849977102428756391261879881714_uint128_t, 8)),
        _hashFuncT(std::make_tuple(11570861985274328016003644510779477088_uint128_t,
                                   112283389177672780736737098051476027196_uint128_t, 8)),
    };

    std::array<_underlyingMapT, Board::BoardFields> _maps;
};

#endif //MODULOFREEBISHOPMAP_H
