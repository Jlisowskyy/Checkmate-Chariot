//
// Created by Jlisowskyy on 5/17/24.
//

#ifndef CHECKMATE_CHARIOT_TESTSETUP_H
#define CHECKMATE_CHARIOT_TESTSETUP_H

#include <semaphore>
#include <sstream>
#include <thread>

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

    void Initialize() {
        _translatorThread = new std::thread(_job, &_stream, &_eng, &_inSem, &_outSem);

        // IMPORTANT: We assumed engine used the book, if not it should be changed
        _eng.SetUseOwnBook();
    }

    void ProcessCommand(const std::string &str)
    {
        _stream.clear();
        _stream << str << std::endl;
        _inSem.release();
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

    void WaitForResult() { _outSem.acquire(); }

    void ProcessCommandSync(const std::string &str)
    {
        ProcessCommand(str);
        WaitForResult();
    }

    Engine &GetEngine() { return _eng; }


    // ------------------------------
    // Private class methods
    // ------------------------------

    private:
    static void
    _job(std::stringstream *stream, Engine *engine, std::binary_semaphore *sem, std::binary_semaphore *outSem)
    {
        GameTimeManager::StartTimerAsync();

        UCITranslator translator{*engine};

        bool result;
        do
        {
            sem->acquire();
            result = translator.BeginCommandTranslation(*stream) != UCITranslator::UCICommand::quitCommand;
            outSem->release();

        } while (result);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    std::binary_semaphore _inSem{0};
    std::binary_semaphore _outSem{0};
    std::stringstream _stream{};
    std::thread *_translatorThread{};
    Engine _eng{};
};

#endif // CHECKMATE_CHARIOT_TESTSETUP_H
