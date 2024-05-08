//
// Created by wookie on 5/8/24.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <mutex>
#include <memory>

template<typename T>
concept Streamable = requires(T a, std::ostream& os) {
    { os << a } -> std::same_as<std::ostream&>;
};

class Logger
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    /// <summary> Default constructor, creates a logger with no output stream </summary>
    Logger() = default;
    /// <summary> Constructor with next handler in the chain </summary>
    [[maybe_unused]] explicit Logger(Logger* next);
    /// <summary> Constructor with output stream </summary>
    [[maybe_unused]] explicit Logger(std::ostream &stream);
    /// <summary> Constructor with next handler in the chain and output stream </summary>
    [[maybe_unused]] explicit Logger(Logger* next, std::ostream &stream);

    Logger(const Logger & clone);
    Logger &operator=(const Logger & rhs);

    // ------------------------------
    // Class interaction
    // ------------------------------

    /// <summary> Set the next handler in the chain </summary>
    [[maybe_unused]] virtual Logger *SetNext(Logger *handler);
    [[maybe_unused]] virtual void AppendNext(Logger *handler);
    /// <summary> Log a message </summary>
    template <Streamable T> [[maybe_unused]] void Log(const T &logMessage){
        if (loggingStream)
        {
            std::lock_guard<std::mutex> lock(logGuard);
            *loggingStream << logMessage;
        }
        if (nextHandler != nullptr)
            nextHandler->Log(logMessage);
    }
    /// <summary> Set the output stream </summary>
    [[maybe_unused]] void SetLoggingStream(std::ostream &stream);

    /// <summary> Start logging a message using streams</summary>
    template <Streamable T> Logger & operator<<(const T &logMessage){
        Log(logMessage);
        return *this;
    }
    typedef std::ostream &(*streamFunction)(std::ostream &);
    Logger & operator<<(streamFunction func);

    // ------------------------------
    // class fields
    // ------------------------------

    private:
    std::shared_ptr<Logger> nextHandler = nullptr;

    protected:
    std::mutex logGuard {};
    std::ostream *loggingStream = nullptr;
};

class [[maybe_unused]] StdoutLogger : public Logger
{
    public:
    StdoutLogger();
};

class [[maybe_unused]] StderrLogger : public Logger
{
    public:
    StderrLogger();
};

class FileLogger : public Logger
{
    public:
    [[maybe_unused]] explicit FileLogger(const std::string &FileName);
    ~FileLogger();

    void ChangeFile(const std::string &FileName);

    protected:
    std::ofstream loggingFileStream;
};

extern StdoutLogger GlobalLogger;

#endif // LOGGER_H
