//
// Created by wookie on 5/8/24.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>

#define WrapTraceMsgError(msg) GlobalLogger.TraceStream << std::format("[ TRACE ][ ERROR ] {}", msg)
#define WrapTraceMsgInfo(msg)  GlobalLogger.TraceStream << std::format("[ TRACE ][ INFO ] {}", msg)

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
/// It is thread safe. Logging is done with the \<\< operator or the Log function
/// @example Logger logger; logger.LogStream \<\< "Hello, World!";
/// @example Logger logger; logger.TraceStream \<\< "Hello, World!";
/// @example Logger logger = std::move(StdoutLogger().AppendNext(new FileLogger("log.txt")));
/// </summary>
class [[maybe_unused]] Logger
{
    protected:
    class LogC;
    class TraceC;

    protected:
    using log_sp = std::shared_ptr<Logger>;
    // ------------------------------
    // nested classes
    // ------------------------------

    protected:
    /// <summary>
    /// Class that allows for logging to the logger via a stream
    /// @example Logger logger; logger.LogStream \<\< "Hello, World!";
    /// </summary>
    class LogC
    {
        public:
        LogC(Logger &logger) : logger(logger) {}
        template <Streamable T> LogC &operator<<(const T &logMessage)
        {
            logger.Log(logMessage);
            return *this;
        }
        /// <summary> Allow for stream functions to be used with the logger </summary>
        typedef std::ostream &(*streamFunction)(std::ostream &);
        LogC &operator<<(streamFunction func);

        private:
        Logger &logger;
    };

    /// <summary>
    /// Class that allows for logging to the logger via a stream if in DEBUG
    /// @example Logger logger; logger.TraceStream \<\< "Hello, World!";
    /// </summary>
    class TraceC
    {
        public:
        TraceC(Logger &logger) : logger(logger) {}
        template <Streamable T> TraceC &operator<<(const T &logMessage)
        {
            logger.Trace(logMessage);
            return *this;
        }
        /// <summary> Allow for stream functions to be used with the logger </summary>
        typedef std::ostream &(*streamFunction)(std::ostream &);
        TraceC &operator<<(streamFunction func);

        private:
        Logger &logger;
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    /// <summary> Default constructor, creates a logger with no output stream </summary>
    Logger();
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

    public:
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
    virtual Logger &AppendNext(Logger *handler);
    /// <summary>
    /// Append a handler to the end of the chain
    /// @remark this assigns the provided shared pointer to the next handler
    /// </summary>
    virtual Logger &AppendNext(log_sp handler);

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
    /// <summary> Log a message if in DEBUG</summary>
    template <Streamable T> void Trace(const T &logMessage)
    {
#ifndef NDEBUG
        if (loggingStream)
        {
            std::lock_guard<std::mutex> lock(logGuard);
            *loggingStream << logMessage;
        }
        if (nextHandler != nullptr)
            nextHandler->Log(logMessage);
#endif
    }
    /// <summary> Set the output stream </summary>
    [[maybe_unused]] void SetLoggingStream(std::ostream &stream);

    private:
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

    public:
    LogC LogStream;
    TraceC TraceStream;

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
