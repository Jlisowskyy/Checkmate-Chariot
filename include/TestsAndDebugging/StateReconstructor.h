//
// Created by Jlisowskyy on 5/19/24.
//

#ifndef CHECKMATE_CHARIOT_STATERECONSTRUCTOR_H
#define CHECKMATE_CHARIOT_STATERECONSTRUCTOR_H

#include <string>
#include <fstream>

#include "TestSetup.h"

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
        for (std::string line{}; std::getline(stream, line); )
        {
            if (line == "breakpoint")
                tool(setup.GetEngine());
//            else

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
