#include <cstdlib>

#include "include/ChessEngine.h"

#include "include/OpeningBook/OpeningBook.h"
#include "include/OpeningBook/BookTester.h"

int main(const int argc, const char** argv) {
    // ChessEngineMainEntry(argc, argv);
    OpeningBook book("resources/uci_ready", OpeningBook::bookFileType::text_uci);
    BookTester::PerformRandomAccessTest(book, "resources/uci_ready", 1000);
    // auto line = ParseTools::Split("c2c4 c7c5 g1f3 g8f6 g2g3 b7b6 f1g2 c8b7 e1g1 e7e6 b1c3 f8e7 d2d4 c5d4 d1d4 d7d6 f1d1 a7a6 b2b3");
    // std::cout << book.GetRandomNextMove(line) << std::endl;

    return EXIT_SUCCESS;
}
