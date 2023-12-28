//
// Created by Jlisowskyy on 12/28/23.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <mutex>

class Logger {
public:
    static void Log(const std::string& logMessage);
    static void LogError(const std::string& logMessage);
    static bool ChangeLogStream(const std::string& FileName);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // ------------------------------
    // class fields
    // ------------------------------
private:
    Logger() = default;

    inline static std::mutex logGuard{};
    inline static std::ofstream loggingFstream{};
    inline static std::ostream* errLoggingStream = &std::cerr;
    inline static std::ostream* loggingStream = &std::cout;
};

#endif //LOGGER_H
