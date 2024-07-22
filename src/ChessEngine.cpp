//
// Created by Jlisowskyy on 12/26/23.
//

#include <sstream>
#include <iostream>

#include "../include/ChessEngine.h"
#include "../include/Engine.h"
#include "../include/Interface/UCITranslator.h"
#include "../include/TestsAndDebugging/DebugTools.h"
#include "../include/TunableParameters.h"

void ChessEngineMainEntry(const int argc, const char **argv)
{
    // Start the time manager
    GameTimeManager::StartTimerAsync();

    // Initialize Engine
    Engine engine{};

    // Provide to the translator the underlying engine instance
    UCITranslator translator{engine};

    // Initialize parameters list
    GlobalParametersList::Init();

    // Disable C compatibility:
    // std::ios_base::sync_with_stdio(false);
    // std::cin.tie(nullptr);
    // std::cout.tie(nullptr);

    // If on linux platform configure stack trace on signal:
#ifdef __unix__
//
//    static constexpr int signals[] { SIGSEGV, SIGINT, SIGKILL, SIGTERM, SIGABRT };
//
//    for (int sig : signals)
//        signal(sig, TRACE_HANDLER);

#endif

    if (argc == 1)
        // Start the engine without any concerns if there is no command line arguments
        translator.BeginCommandTranslation(std::cin);
    else
    // Inject all the arguments as single commands to the engine
    {
        std::string commandBuffer{};
        for (int i = 1; i < argc; ++i) commandBuffer += std::string(argv[i]) + '\n';

        std::istringstream stream(commandBuffer);
        auto lastCommand = translator.BeginCommandTranslation(stream);

        // Continue usual execution if the last command was not a quit command
        if (lastCommand != UCITranslator::UCICommand::quitCommand)
            translator.BeginCommandTranslation(std::cin);
    }
}
