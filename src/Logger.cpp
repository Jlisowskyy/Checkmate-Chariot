//
// Created by Jlisowskyy on 12/28/23.
//

#include "../include/Logger.h"

void Logger::Log(const std::string& logMessage) {
    logGuard.lock();
    (*loggingStream) << logMessage << std::endl;
    logGuard.unlock();
}

void Logger::LogError(const std::string& logMessage) {
    logGuard.lock();
    (*errLoggingStream) << logMessage << std::endl;
    logGuard.unlock();
}

bool Logger::ChangeLogStream(const std::string& FileName) {
    loggingFstream = std::ofstream{FileName};

    if (!loggingFstream) return false;

    logGuard.lock();
    errLoggingStream = &loggingFstream;
    loggingStream = &loggingFstream;
    logGuard.unlock();
    return true;
}
