//
// Created by Jlisowskyy on 12/26/23.
//

#include <sstream>

#include "../include/ChessEngine.h"
#include "../include/Engine.h"
#include "../include/Interface/UCITranslator.h"

void ChessEngineMainEntry(const int argc, const char **argv)
{
    // Start the time manager
    GameTimeManager::StartTimerAsync();

    // Initialize Engine
    Engine engine{};

    // Provide to the translator the underlying engine instance
    UCITranslator translator{engine};

    // Start the engine
    engine.Initialize();

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
