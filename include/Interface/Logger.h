//
// Created by wookie on 5/8/24.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>

/// <summary>
/// Concept for types that allow streaming (i.e. can be used with std::ostream)
/// </summary>
template <typename T>
concept Streamable = requires(T a, std::ostream &os) {
    {
        os << a
    } -> std::same_as<std::ostream &>;
};

/// <summary>
/// Logger class that allows for chaining of loggers and logging to multiple streams
/// It is thread safe. Logging is done with the << operator or the Log function
/// @example Logger logger; logger \<\< "Hello, World!";
/// @example Logger logger = std::move(StdoutLogger().AppendNext(new FileLogger("log.txt")));
/// </summary>
class [[maybe_unused]] Logger
{
    using log_sp = std::shared_ptr<Logger>;
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    /// <summary> Default constructor, creates a logger with no output stream </summary>
    Logger() = default;
    /// <summary>
    /// Constructor which sets next handler in the chain
    /// @remark this creates a shared pointer to the next handler. If you already have a shared pointer, use the other
    /// constructor
    /// </summary>
    explicit Logger(Logger *next);
    /// <summary>
    /// Constructor which sets next handler in the chain. It assigns the provided shared pointer to the next handler
    /// </summary>
    explicit Logger(log_sp next);
    /// <summary> Constructor with output stream </summary>
    explicit Logger(std::ostream &stream);
    /// <summary>
    /// Constructor with next handler in the chain and output stream
    /// @remark this creates a shared pointer to the next handler. If you already have a shared pointer, use the other
    /// constructor
    /// </summary>
    explicit Logger(Logger *next, std::ostream &stream);
    /// <summary>
    /// Constructor with next handler in the chain and output stream.
    /// It assigns the provided shared pointer to the next handler
    /// </summary>
    explicit Logger(log_sp next, std::ostream &stream);
    virtual ~Logger() = default;

    Logger(const Logger &loggerToCopy) = delete; // Copy constructor does not make sense
    Logger &operator=(Logger &&other)  = delete; // Move assignment does not make sense
    Logger(Logger &&other) noexcept;             // Move constructor exists and is implemented

    // ------------------------------
    // Class interaction
    // ------------------------------

    /// <summary>
    /// Set the next handler in the chain
    /// @remark this creates a shared pointer to the provided handler.
    /// </summary>
    virtual Logger &SetNext(Logger *handler);
    /// <summary>
    /// Set the next handler in the chain
    /// @remark this assigns the provided shared pointer to the next handler
    /// </summary>
    [[maybe_unused]] virtual Logger &SetNext(log_sp handler);
    /// <summary>
    /// Append a handler to the end of the chain
    /// @remark this creates a shared pointer to the provided handler.
    /// </summary>
    virtual Logger& AppendNext(Logger *handler);
    /// <summary>
    /// Append a handler to the end of the chain
    /// @remark this assigns the provided shared pointer to the next handler
    /// </summary>
    virtual Logger& AppendNext(log_sp handler);

    /// <summary> Log a message </summary>
    template <Streamable T> void Log(const T &logMessage)
    {
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

    /// <summary> Log a message using streams </summary>
    template <Streamable T> Logger &operator<<(const T &logMessage)
    {
        Log(logMessage);
        return *this;
    }
    /// <summary> Allow for stream functions to be used with the logger </summary>
    typedef std::ostream &(*streamFunction)(std::ostream &);
    Logger &operator<<(streamFunction func);

    // ------------------------------
    // class fields
    // ------------------------------

    private:
    std::shared_ptr<Logger> nextHandler = nullptr;

    protected:
    std::mutex logGuard{};
    std::ostream *loggingStream = nullptr;
};

/// <summary> Logger that logs to stdout </summary>
class [[maybe_unused]] StdoutLogger : public Logger
{
    public:
    StdoutLogger();
};

/// <summary> Logger that logs to stderr </summary>
class [[maybe_unused]] StderrLogger : public Logger
{
    public:
    StderrLogger();
};

/// <summary> Logger that logs to a file </summary>
class FileLogger : public Logger
{
    public:
    explicit FileLogger(const std::string &FileName);
    void ChangeFile(const std::string &FileName);

    protected:
    std::ofstream loggingFileStream;
};

extern StdoutLogger GlobalLogger;

#endif // LOGGER_H
