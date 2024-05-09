#include <gtest/gtest.h>

#include "../include/Evaluation/BoardEvaluator.h"
#include "../include/Interface/FenTranslator.h"

TEST(BoardEvaluator, TapperedEvalHighLow)
{
    // Arrange
    Board fullBoard = FenTranslator::GetDefault();
    Board emptyBoard{};
    FenTranslator::Translate("8/8/8/8/8/8/8/8 w - - 0 1", emptyBoard);

    Board avgBoard{};
    FenTranslator::Translate("3k4/6q1/2b5/8/8/1Q6/8/3K1R2 w - - 0 1", avgBoard);

    ASSERT_EQ(BoardEvaluator::InterpGameStage(fullBoard, 0, 1), 0);
    ASSERT_EQ(BoardEvaluator::InterpGameStage(emptyBoard, 0, 1), 1);

    static constexpr int32_t maxEval = 1000;
    ASSERT_LT(BoardEvaluator::InterpGameStage(avgBoard, 0, maxEval), maxEval);
    ASSERT_GT(BoardEvaluator::InterpGameStage(avgBoard, 0, maxEval), 0);
}
