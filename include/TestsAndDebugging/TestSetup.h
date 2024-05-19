//
// Created by Jlisowskyy on 5/17/24.
//

#ifndef CHECKMATE_CHARIOT_TESTSETUP_H
#define CHECKMATE_CHARIOT_TESTSETUP_H

#include <sstream>
#include <thread>
#include <semaphore>

#include "../Engine.h"
#include "../Interface/UCITranslator.h"
#include "../ThreadManagement/GameTimeManager.h"

/*
 * Class used to simulate real environment, simplifies testing.
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
        _translatorThread = new std::thread(_job, &_stream, &_eng, &_sem);
    }

    void ProcessCommand(const std::string &str) {
        _stream.clear();
        _stream << str << std::endl;
        _sem.release();
    }

    void Close()
    {
        if (_translatorThread)
        {
            ProcessCommand("exit");
            _translatorThread->join();
            delete _translatorThread;
            _translatorThread = nullptr;
        }
    }

    Engine& GetEngine(){
        return _eng;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    static void _job(std::stringstream *stream, Engine *engine, std::binary_semaphore* sem)
    {
        GameTimeManager::StartTimerAsync();
        engine->Initialize();

        UCITranslator translator{*engine};

        do
        {
            sem->acquire();
        }while (translator.BeginCommandTranslation(*stream) != UCITranslator::UCICommand::quitCommand);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    std::binary_semaphore _sem{0};
    std::stringstream _stream{};
    std::thread *_translatorThread{};
    Engine _eng{};
};

#endif // CHECKMATE_CHARIOT_TESTSETUP_H
