#include <cstdlib>
#include <iostream>

#include "include/ChessEngine.h"
#include "include/Search/BestMoveSearch.h"

int main(const int argc, const char** argv) {
    // ChessEngineMainEntry(argc, argv);

    BestMoveSearch seracher{FenTranslator::GetDefault()};
    seracher.searchMoveTimeFullBoardEvalUnthreaded<decltype(BoardEvaluator::NaiveEvaluation2), true>(BoardEvaluator::NaiveEvaluation2, 1000);
    return EXIT_SUCCESS;
}
