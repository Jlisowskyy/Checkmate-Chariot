//
// Created by Jlisowskyy on 5/19/24.
//

#ifndef CHECKMATE_CHARIOT_STATERECONSTRUCTOR_H
#define CHECKMATE_CHARIOT_STATERECONSTRUCTOR_H

#include <fstream>
#include <string>

#include "../ParseTools.h"
#include "TestSetup.h"

/*
 *  IMPORTANT NOTE:
 *
 *  Exact state will probably be not able to reconstruct due to OS scheduler random impact
 *  on threads running the search!
 *
 *  File passed as an input needs to be formated in given way per line: { posix time in ms } | { command }
 *  Time is used to simulate exact same command execution time
 * */

struct StateReconstructor
{
    // ------------------------------
    // Class creation
    // ------------------------------

    StateReconstructor()  = delete;
    ~StateReconstructor() = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template <class debugTool> static bool Reconstruct(debugTool tool, const std::string &inputPath, bool brakeAfterEveryCommand = false)
    {
        std::ifstream stream{inputPath};

        if (!stream)
            return false;

        TestSetup setup{};
        setup.Initialize();

        lli prevTime = -1;
        // process log line by line
        for (std::string line{}; std::getline(stream, line);)
        {
            if (line == "breakpoint" || brakeAfterEveryCommand)
            {
                tool(setup);

                // run given function and wait for input
                getchar();
                continue;
            }

            // Parse timings from the file
            std::string timeStr;
            size_t pos;
            if ((pos = ParseTools::ExtractNextWord(line, timeStr, 0)) == ParseTools::InvalidNextWorldRead)
                continue;

            // Read timings from the log
            const lli milliseconds = std::stoll(timeStr);

            // give the engine some time to process the command
            if (prevTime != -1)
            {
                const lli diff = milliseconds - prevTime;

                if (diff > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(diff));
                prevTime = milliseconds;
            }
            else
                prevTime = milliseconds;

            const auto command = ParseTools::GetTrimmed(line.substr(pos));

            GlobalLogger.LogStream << std::format("Processing command: {}", command) << std::endl;
            setup.ProcessCommand(command);
        }

        setup.Close();
        UCITranslator finalTranslator(setup.GetEngine());
        finalTranslator.BeginCommandTranslation(std::cin);

        return true;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------
};

#endif // CHECKMATE_CHARIOT_STATERECONSTRUCTOR_H
