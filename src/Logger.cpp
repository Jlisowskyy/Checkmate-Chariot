//
// Created by Jlisowskyy on 12/28/23.
//

#include "../include/Interface/Logger.h"

Logger GlobalLogger{};

void Logger::Log(const std::string& logMessage) {
    logGuard.lock();
    (*loggingStream) << logMessage << std::endl;
    logGuard.unlock();
}

std::ostream& Logger::StartLogging() { return *loggingStream; }

std::ostream& Logger::StartErrLogging() { return *errLoggingStream; }

void Logger::LogError(const std::string& logMessage) {
    logGuard.lock();
    (*errLoggingStream) << logMessage << std::endl;
    logGuard.unlock();
}

bool Logger::ChangeLogStream(const std::string& FileName) {
    if (const std::ofstream nStream{FileName}; !nStream) return false;

    logGuard.lock();
    CloseFStream();
    loggingFstream = std::ofstream{FileName};
    errLoggingStream = &loggingFstream;
    loggingStream = &loggingFstream;
    logGuard.unlock();

    return true;
}

void Logger::ChangeLogStream(std::ostream& stream) {
    logGuard.lock();
    CloseFStream();
    errLoggingStream = &stream;
    loggingStream = &stream;
    logGuard.unlock();
}

void Logger::CloseFStream() {
    if (loggingFstream) loggingFstream.close();
}
