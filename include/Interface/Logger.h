//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <mutex>

class Logger
{
public:
    Logger() = default;

    Logger(const Logger&) = delete;

    Logger& operator=(const Logger&) = delete;

    void Log(const std::string&logMessage);

    std::ostream& StartLogging();

    std::ostream& StartErrLogging();

    void LogError(const std::string&logMessage);

    bool ChangeLogStream(const std::string&FileName);

    void ChangeLogStream(std::ostream&stream);

    void CloseFStream();

    // ------------------------------
    // class fields
    // ------------------------------
private:
    std::mutex logGuard{};
    std::ofstream loggingFstream{};
    std::ostream* errLoggingStream = &std::cerr;
    std::ostream* loggingStream = &std::cout;
};

extern Logger GlobalLogger;

#endif //LOGGER_H
