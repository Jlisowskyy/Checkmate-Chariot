#include <cstdlib>
#include <iostream>

#include "include/ChessEngine.h"
#include "include/Search/BestMoveSearch.h"

int main(const int argc, const char** argv) {
    // ChessEngineMainEntry(argc, argv);

    BestMoveSearch seracher{};
    seracher.SearchMoveTimeFullBoardEval(BoardEvaluator::NaiveEvaluation2, 10);
    return EXIT_SUCCESS;
}
