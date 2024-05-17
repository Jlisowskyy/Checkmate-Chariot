//
// Created by Jlisowskyy on 5/17/24.
//

#ifndef CHECKMATE_CHARIOT_TESTSETUP_H
#define CHECKMATE_CHARIOT_TESTSETUP_H

#include <sstream>
#include <thread>

#include "../Engine.h"
#include "../Interface/UCITranslator.h"
#include "../ThreadManagement/GameTimeManager.h"

/*
 * Class used to simulate real environment, simplifies testing.
 *
 *      WARNING
 * Currently not working:
 * TODO: sstringstream should be replaced with some pipe like mechanisms, because it is non blocking
 * */

struct TestSetup
{
    // ------------------------------
    // Class creation
    // ------------------------------

    TestSetup() = default;

    ~TestSetup() { Close(); }

    // ------------------------------
    // Class interaction
    // ------------------------------

    void Initialize()
    {
        //        _stream << std::no
        _translatorThread = new std::thread(_job, &_stream, &_eng);
    }

    void ProcessCommand(const std::string &str) { _stream << str << std::endl; }

    void Close()
    {
        if (_translatorThread)
        {
            ProcessCommand("exit");
            _translatorThread->join();
            delete _translatorThread;
        }
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    static void _job(std::stringstream *stream, Engine *engine)
    {
        GameTimeManager::StartTimerAsync();
        UCITranslator translator{*engine};
        engine->Initialize();

        translator.BeginCommandTranslation(*stream);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    std::stringstream _stream{};
    std::thread *_translatorThread{};
    Engine _eng{};
};

#endif // CHECKMATE_CHARIOT_TESTSETUP_H
