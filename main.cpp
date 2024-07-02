#include <cstdlib>

#include "include/ChessEngine.h"
#include "include/TestsAndDebugging/DebugTools.h"

int main(const int argc, const char **argv)
{
#ifdef __unix__
    StackStartAddress = GetCurrStackPtr();
#endif

    ChessEngineMainEntry(argc, argv);

    return EXIT_SUCCESS;
}
