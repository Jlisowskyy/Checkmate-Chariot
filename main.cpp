#include <cstdlib>

#include "include/ChessEngine.h"
#include "include/TestsAndDebugging/TestSetup.h"

int main(const int argc, const char **argv)
{
//    ChessEngineMainEntry(argc, argv);

    TestSetup test{};
    test.Initialize();

    test.ProcessCommand("go infinite");
    std::this_thread::sleep_for(std::chrono::seconds{5});
    test.ProcessCommand("stop");
    test.Close();

    return EXIT_SUCCESS;
}
