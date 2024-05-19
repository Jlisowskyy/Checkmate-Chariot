//
// Created by Jlisowskyy on 5/19/24.
//

#ifndef CHECKMATE_CHARIOT_STATERECONSTRUCTOR_H
#define CHECKMATE_CHARIOT_STATERECONSTRUCTOR_H

#include <string>
#include <fstream>

#include "TestSetup.h"
#include "../ParseTools.h"

/*
 *  IMPORTANT NOTE:
 *
 *  Exact state will probably be not able to reconstruct due to OS scheduler random impact
 *  on threads running the search!
 *
 * */

struct StateReconstructor
{
// ------------------------------
// Class creation
// ------------------------------

    StateReconstructor() = delete;
    ~StateReconstructor() = delete;

// ------------------------------
// Class interaction
// ------------------------------

    template<class debugTool>
    static bool Reconstruct(debugTool tool, const std::string& inputPath)
    {
        std::ifstream stream{inputPath};

        if (!stream)
            return false;

        TestSetup setup{};
        lli prevTime = -1;

        // process log line by line
        for (std::string line{}; std::getline(stream, line); )
        {
            if (line == "breakpoint") {
                tool(setup.GetEngine());

                // run given function and wait for input
                getchar();
                continue;
            }

            // Parse timings from the file
            const auto split = ParseTools::Split<[](int c) { return c == '|'; }>(line);
            const auto timeStr = ParseTools::GetTrimmed(split[0]);

            // Read timings from the log
            const lli milliseconds = std::stoll(timeStr);

            // give the engine some time to process the command
            if (prevTime != -1){
                const lli diff = milliseconds - prevTime;

                std::this_thread::sleep_for(std::chrono::milliseconds(diff));
            }
            else
                prevTime = milliseconds;

            const auto command = ParseTools::GetTrimmed(split[1]);
            setup.ProcessCommand(command);
        }

        return true;
    }

// ------------------------------
// Private class methods
// ------------------------------

// ------------------------------
// Class fields
// ------------------------------


};

#endif //CHECKMATE_CHARIOT_STATERECONSTRUCTOR_H
