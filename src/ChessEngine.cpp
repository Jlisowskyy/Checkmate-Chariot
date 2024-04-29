//
// Created by Jlisowskyy on 12/26/23.
//

#include <sstream>

#include "../include/ChessEngine.h"
#include "../include/Engine.h"
#include "../include/Interface/UCITranslator.h"

void ChessEngineMainEntry(const int argc, const char **argv)
{
    Engine engine{};
    UCITranslator translator{engine};
    engine.Initialize();

    if (argc == 1)
        translator.BeginCommandTranslation(std::cin);
    else
    {
        std::string commandBuffer{};
        for (int i = 1; i < argc; ++i) commandBuffer += std::string(argv[i]) + '\n';

        std::istringstream stream(commandBuffer);
        auto lastCommand = translator.BeginCommandTranslation(stream);

        if (lastCommand != UCITranslator::UCICommand::quitCommand)
            translator.BeginCommandTranslation(std::cin);
    }
}
