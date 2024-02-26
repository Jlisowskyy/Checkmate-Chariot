#include <cstdlib>

#include "include/ChessEngine.h"

#include "include/OpeningBook/OpeningBook.h"
#include "include/OpeningBook/BookTester.h"

int main(const int argc, const char** argv) {
    // ChessEngineMainEntry(argc, argv);
    OpenningBook book("resources/uci_ready", OpenningBook::bookFileType::text_uci);
    BookTester::PerformRandomAccessTest(book, "resources/uci_ready", 100);

    return EXIT_SUCCESS;
}
