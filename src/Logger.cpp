//
// Created by wookie on 5/8/24.
//

#include <cassert>
#include <memory>

#include "../include/Interface/Logger.h"

FileLogger GlobalLogger = FileLogger("log.txt");

[[maybe_unused]] Logger::Logger(std::ostream &stream) {
    loggingStream = &stream;
}

[[maybe_unused]] Logger::Logger(Logger *next) {
    this->nextHandler = std::shared_ptr<Logger>(next);
}

[[maybe_unused]] Logger::Logger(Logger *next, std::ostream &stream) {
    this->nextHandler = std::shared_ptr<Logger>(next);
    loggingStream = &stream;
}

Logger &Logger::operator=(const Logger & rhs) {
    if (this == &rhs)
            return *this;

    loggingStream = rhs.loggingStream;
    nextHandler = std::make_shared<Logger>(rhs.nextHandler.get());
    return *this;
}

Logger::Logger(const Logger &clone) {
    loggingStream = clone.loggingStream;
    nextHandler = std::make_shared<Logger>(clone.nextHandler.get());
}

[[maybe_unused]] Logger *Logger::SetNext(Logger *handler) {
    nextHandler = std::make_shared<Logger>(handler);
    return handler;
}

[[maybe_unused]] void Logger::SetLoggingStream(std::ostream &stream) {
    loggingStream = &stream;
}

Logger &Logger::operator<<(Logger::streamFunction func) {
    if (loggingStream){
        func(*loggingStream);
    }

    if (nextHandler != nullptr)
        (*nextHandler) << func;

    return *this;
}

void Logger::AppendNext(Logger *handler) {
    if (nextHandler == nullptr)
        nextHandler = std::shared_ptr<Logger>(handler);
    else
        nextHandler->AppendNext(handler);
}

[[maybe_unused]] FileLogger::FileLogger(const std::string &FileName) {
    loggingFileStream = std::ofstream{FileName};
    loggingStream = &loggingFileStream;
    assert(loggingFileStream && "FileLogger: Unable to open file for logging");
}
FileLogger::~FileLogger() {
    loggingFileStream.close();
}
StderrLogger::StderrLogger() {
    loggingStream = &std::cerr;
}
StdoutLogger::StdoutLogger() {
    loggingStream = &std::cout;
}