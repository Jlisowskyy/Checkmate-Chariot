//
// Created by Jlisowskyy on 3/23/24.
//

#include "../include/MoveGeneration/Move.h"
#include "../include/EngineUtils.h"

std::string PackedMove::GetLongAlgebraicNotation() const
{
    static constexpr char PromoFigs[] = { 'q', 'r', 'b', 'n' };
    std::string rv;

    auto [c1, c2] = ConvertToCharPos((int)GetStartField());
    rv += c1; rv += c2;
    auto [c3, c4] = ConvertToCharPos((int)GetTargetField());
    rv += c3; rv += c4;

    if (IsPromo())
        rv += PromoFigs[GetMoveType() & PromoSpecBits];

    return rv;
}
