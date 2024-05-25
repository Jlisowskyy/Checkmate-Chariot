//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <chrono>
#include <map>
#include <memory>

#include "../include/ThreadManagement/SearchThreadManager.h"
#include "EngineUtils.h"
#include "Interface/FenTranslator.h"
#include "Interface/Logger.h"
#include "Interface/UCIOptions.h"
#include "OpeningBook/OpeningBook.h"

/*
 *  Class simply representing the Engine, with all the necessary methods to interact with it.
 *
 * */

class Engine
{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------

    public:
    Engine() = default;

    /* Load with initial state */
    Engine(const Board &bd) : _board(bd), _startingBoard(bd) {}

    ~Engine() = default;

    // ------------------------------
    // Type interaction
    // ------------------------------

    /* Simply writes pretty formated board to the stdout */
    void WriteBoard() const;

    /*
     * Returns map of moves and their leaf counts its mean number of leafs at the full search tree base - Used for tests
     * only. Keys are simply uci encoded moves, values are the number of leafs at the end of the search tree.
     * */
    std::map<std::string, uint64_t> GetPerft(int depth);

    /* Simple wrapper of 'GetPerft' to pretty print the results */
    template <bool LogToOut = true> double GoPerft(int depth);

    /* Tries to parse given position and applies it to internal boards */
    bool SetFenPosition(const std::string &fenStr);

    /* Set default board as internal state */
    void SetStartPos();

    /* Returns engine information */
    static const EngineInfo &GetEngineInfo();

    /* Methods tries to parse and apply moves one by one, if the process is successful sets up age to UCIMoves.size+1 */
    bool ApplyMoves(const std::vector<std::string> &UCIMoves);

    /* Restarts the engine up to initial state, sets up the default board and cleans up the Transposition Table */
    void RestartEngine();

    /* Simply copies the board */
    [[nodiscard]] Board GetUnderlyingBoardCopy() const;

    /* Simply returns the moving color */
    [[nodiscard]] int GetMovingColor() const { return _board.MovingColor; }

    [[nodiscard]] uint16_t GetAge() const { return _age; }

    /* Returns FEN encoded actual board state */
    [[nodiscard]] std::string GetFenTranslation() const;

    /* Simply issues stop command to the ThreadManager */
    void StopSearch();

    /* Simply issues go infinite command to the ThreadManager */
    void GoInfinite();

    /* Simply issues go command to the ThreadManager */
    void Go(GoInfo &info, const std::vector<std::string> &moves);

    void PonderHit();

    static const std::string &GetDebugEnginePath() { return _debugEnginePath; }

    // ------------------------------
    // private methods
    // ------------------------------

    private:
    /* Method simply generates moves and checks whether given moves is on the list if that's true applies the move to
     * the board */
    bool _applyMove(Board &board, const std::string &move, uint64_t &hash);

    // ------------------------------------
    // UCI option accessing functions
    // ------------------------------------

    static void _changeOrSetLogFile(Engine &eng, std::string &nPath);

    static void _changeHashSize([[maybe_unused]] Engine &, lli size);

    static void _changeBookUsage(Engine &eng, bool newValue);

    static void _clearHash([[maybe_unused]] Engine &);

    static void _changeDebugEnginePath(Engine &, std::string &path);

    static void _changeBookPath(Engine &engine, std::string &path);

    static void _changeThreadCount([[maybe_unused]] Engine &eng, const lli tCount)
    {
        GlobalLogger.LogStream << "New thread count: " << tCount << '\n';
    }

    // ------------------------------
    // private fields
    // ------------------------------

    Board _board         = FenTranslator::GetDefault();
    Board _startingBoard = FenTranslator::GetDefault();
    OpeningBook _book{};
    std::unordered_map<uint64_t, int> _repetitionMap{};
    std::string _bookPath = _defaultBookPath;

    bool _isStartPosPlayed                            = true;
    uint16_t _age                                     = 1;
    static constexpr std::string_view _startposPrefix = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq";
    bool UseOwnBook                                   = false;

    /// <summary>
    /// File logger for debugging, settable with UCI command. When set, it exists until the Engine or the GlobalLogger
    /// is alive
    /// @see GlobalLogger
    /// </summary>
    std::shared_ptr<FileLogger> _fileLogger;

    static std::string _debugEnginePath;

    public:
    SearchThreadManager TManager{};

    private:
    // ------------------------------
    // Engine options
    // ------------------------------

    lli _threadCount = 1;
    std::string _debugPath;

    static constexpr const char *_defaultBookPath = "uci_ready_long";

    // Options available in engine
    inline static const OptionT<Option::OptionType::spin> Threads{"Threads", _changeThreadCount, 1, 1024, 1};
    inline static const OptionT<Option::OptionType::string> DebugLogFile{"Debug Log File", _changeOrSetLogFile, ""};
    inline static const OptionT<Option::OptionType::spin> HashSize{"Hash", _changeHashSize, 16, 524289, 16};
    inline static const OptionT<Option::OptionType::check> OwnBook{"OwnBook", _changeBookUsage, true};
    inline static const OptionT<Option::OptionType::button> ClearHash{"Clear Hash", _clearHash};
    inline static const OptionT<Option::OptionType::string> TestEnginePath{
        "Test Engine Path", _changeDebugEnginePath, ""
    };
    inline static const OptionT<Option::OptionType::string> BookPath{"OwnBook Path", _changeBookPath, _defaultBookPath};

    inline static const EngineInfo engineInfo = {
        .author = "Jakub Lisowski, Lukasz Kryczka, Jakub Pietrzak Warsaw University of Technology",
        .name   = "ChessEngine development version 0.16",
        .options =
            std::map<std::string, const Option *>{
                                                  std::make_pair<std::string, const Option *>("Threads", &Threads),
                                                  std::make_pair<std::string, const Option *>("Debug Log File", &DebugLogFile),
                                                  std::make_pair<std::string, const Option *>("Hash", &HashSize),
                                                  std::make_pair<std::string, const Option *>("OwnBook", &OwnBook),
                                                  std::make_pair<std::string, const Option *>("Clear Hash", &ClearHash),
                                                  std::make_pair<std::string, const Option *>("Test Engine Path", &TestEnginePath),
                                                  std::make_pair<std::string, const Option *>("OwnBook Path", &BookPath),
                                                  },
    };
};

template <bool LogToOut> double Engine::GoPerft(const int depth)
{
    const auto t1 = std::chrono::steady_clock::now();
    auto moves    = GetPerft(depth);
    const auto t2 = std::chrono::steady_clock::now();

    uint64_t totalSum{};
    for (const auto &[moveStr, moveCount] : moves)
    {
        if constexpr (LogToOut)
            GlobalLogger.LogStream << std::format("{}: {}\n", moveStr, moveCount);
        totalSum += moveCount;
    }

    double spentTime = static_cast<double>((t2 - t1).count()) * 1e-6;
    if constexpr (LogToOut)
        GlobalLogger.LogStream << std::format("Calculated moves: {} in time: {}ms\n", totalSum, spentTime);

    return spentTime;
}

#endif // ENGINE_H
