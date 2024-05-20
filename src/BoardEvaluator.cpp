//
// Created by Jlisowskyy on 5/17/24.
//

#include "../include/Evaluation/BoardEvaluator.h"

BoardEvaluator::MaterialArrayT BoardEvaluator::_materialTable = []() -> MaterialArrayT
{
    // Lambda used to based on given index return array of figure counts
    auto _reverseMaterialIndex = [](const size_t index) -> FigureCountsArrayT
    {
        return {
            (index % BlackPawnCoef) / WhitePawnCoef,     (index % BlackKnightCoef) / WhiteKnightCoef,
            (index % BlackBishopCoef) / WhiteBishopCoef, (index % BlackRookCoef) / WhiteRookCoef,
            (index % BlackQueenCoef) / WhiteQueenCoef,   index / BlackPawnCoef,
            (index % WhitePawnCoef) / BlackKnightCoef,   (index % WhiteKnightCoef) / BlackBishopCoef,
            (index % WhiteBishopCoef) / BlackRookCoef,   (index % WhiteRookCoef) / BlackQueenCoef,
        };
    };

    MaterialArrayT arr{};

    // processing all position with standard procedure
    for (size_t i = 0; i < MaterialTableSize; ++i)
    {
        auto figArr = _reverseMaterialIndex(i);

        int32_t phase         = BoardEvaluator::_calcPhase(figArr);
        int32_t materialValue = BoardEvaluator::_slowMaterialCalculation<EvalMode::BaseMode>(figArr, phase);

        arr[i] = static_cast<int16_t>(materialValue);
    }

    // applying draw position scores
    for (const auto &drawPos : MaterialDrawPositionConstelations)
    {
        size_t index{};

        for (size_t i = 0; i < drawPos.size(); ++i) index += drawPos[i] * FigCoefs[i];

        arr[index] = 0;
    }

    return arr;
}();
