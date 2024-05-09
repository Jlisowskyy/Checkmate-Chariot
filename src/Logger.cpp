//
// Created by wookie on 5/8/24.
//

#include <cassert>
#include <memory>
#include <utility>

#include "../include/Interface/Logger.h"

StdoutLogger GlobalLogger{};

Logger::Logger() : LogStream(*this), TraceStream(*this) {}
Logger::Logger(Logger *next) : Logger() { this->nextHandler = std::shared_ptr<Logger>(next); }
Logger::Logger(Logger::log_sp next) : Logger() { this->nextHandler = std::move(next); }
Logger::Logger(std::ostream &stream) : Logger() { loggingStream = &stream; }
Logger::Logger(Logger *next, std::ostream &stream) : Logger()
{
    this->nextHandler = std::shared_ptr<Logger>(next);
    loggingStream     = &stream;
}
Logger::Logger(Logger::log_sp next, std::ostream &stream) : Logger()
{
    this->nextHandler = std::move(next);
    loggingStream     = &stream;
}
Logger::Logger(Logger &&other) noexcept : Logger()
{
    loggingStream = other.loggingStream;
    nextHandler   = std::move(other.nextHandler);
}
Logger &Logger::SetNext(Logger *handler)
{
    nextHandler = std::shared_ptr<Logger>(handler);
    return *(nextHandler.get());
}
[[maybe_unused]] Logger &Logger::SetNext(Logger::log_sp handler)
{
    nextHandler = std::move(handler);
    return *(nextHandler.get());
}
Logger &Logger::AppendNext(Logger *handler)
{
    if (nextHandler == nullptr)
        nextHandler = std::shared_ptr<Logger>(handler);
    else
        nextHandler->AppendNext(handler);

    return *this;
}
Logger &Logger::AppendNext(Logger::log_sp handler)
{
    if (nextHandler == nullptr)
        nextHandler = std::move(handler);
    else
        nextHandler->AppendNext(handler);

    return *this;
}
[[maybe_unused]] void Logger::SetLoggingStream(std::ostream &stream) { loggingStream = &stream; }
Logger &Logger::operator<<(Logger::streamFunction func)
{
    if (loggingStream)
    {
        func(*loggingStream);
    }

    if (nextHandler != nullptr)
        (*nextHandler) << func;

    return *this;
}

FileLogger::FileLogger(const std::string &FileName) { ChangeFile(FileName); }

void FileLogger::ChangeFile(const std::string &FileName)
{
    if (loggingFileStream && loggingFileStream.is_open())
        loggingFileStream.close();

    loggingFileStream = std::ofstream{FileName};
    loggingStream     = &loggingFileStream;
    assert(loggingFileStream && "FileLogger: Unable to open file for logging");
}
StderrLogger::StderrLogger() { loggingStream = &std::cerr; }
StdoutLogger::StdoutLogger() { loggingStream = &std::cout; }
Logger::TraceC &Logger::TraceC::operator<<(Logger::streamFunction func)
{
#ifdef NDEBUG
    logger << func;
#endif
    return *this;
}
Logger::LogC &Logger::LogC::operator<<(Logger::streamFunction func)
{
    logger << func;
    return *this;
}
