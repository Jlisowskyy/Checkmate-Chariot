//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <chrono>
#include <map>

#include "../include/ThreadManagement/SearchThreadManager.h"
#include "EngineUtils.h"
#include "Interface/FenTranslator.h"
#include "Interface/Logger.h"
#include "Interface/UCIOptions.h"
#include "OpeningBook/OpeningBook.h"

class Engine
{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------

    public:
    Engine() = default;

    Engine(const Board &bd) : _board(bd), _startingBoard(bd) {}

    ~Engine() = default;

    void Initialize();

    // ------------------------------
    // Type interaction
    // ------------------------------

    void WriteBoard() const;

    std::map<std::string, uint64_t> GetPerft(int depth);

    template <bool LogToOut = true> double GoPerft(int depth);

    void SetFenPosition(const std::string &fenStr);

    void SetStartPos();

    static const EngineInfo &GetEngineInfo();

    bool ApplyMoves(const std::vector<std::string> &UCIMoves);

    void RestartEngine();

    [[nodiscard]] Board GetUnderlyingBoardCopy() const;

    [[nodiscard]] int GetMovingColor() const { return _board.MovingColor; }

    [[nodiscard]] std::string GetFenTranslation() const;

    void StopSearch();

    void GoInfinite();

    void Go(const GoInfo& info, const std::vector<std::string> &moves);

    // ------------------------------
    // private methods
    // ------------------------------

    private:
    bool _applyMove(Board &board, const std::string &move);

    static void _changeDebugState(Engine &eng, std::string &nPath);

    static void _changeHashSize([[maybe_unused]] Engine &eng, lli size);

    static void _changeBookUsage(Engine &eng, bool newValue);

    static void _changeThreadCount([[maybe_unused]] Engine &eng, const lli tCount)
    {
        std::cout << "New thread count: " << tCount << '\n';
    }

    // ------------------------------
    // private fields
    // ------------------------------

    Board _board{};
    Board _startingBoard{};
    OpeningBook _book{"uci_ready_long", OpeningBook::bookFileType::text_uci};

    bool _isStartPosPlayed                            = true;
    uint16_t _age                                     = 1;
    static constexpr std::string_view _startposPrefix = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq";
    bool UseOwnBook                                   = false;

    public:
    SearchThreadManager TManager{};

    private:
    // ------------------------------
    // Engine options
    // ------------------------------

    lli _threadCount = 1;
    std::string _debugPath;

    // Options available in engine
    inline static const OptionT<Option::OptionType::spin> Threads{"Threads", _changeThreadCount, 1, 1024, 1};
    inline static const OptionT<Option::OptionType::string> DebugLogFile{"Debug Log File", _changeDebugState, ""};
    inline static const OptionT<Option::OptionType::spin> HashSize{"Hash", _changeHashSize, 16, 524289, 16};
    inline static const OptionT<Option::OptionType::check> OwnBook{"OwnBook", _changeBookUsage, true};

    inline static const EngineInfo engineInfo = {
        .author = "Jakub Lisowski, Lukasz Kryczka, Jakub Pietrzak Warsaw University of Technology",
        .name   = "ChessEngine development version 0.16",
        .options =
            std::map<std::string, const Option *>{
                                                  std::make_pair<std::string, const Option *>("Threads", &Threads),
                                                  std::make_pair<std::string, const Option *>("Debug Log File", &DebugLogFile),
                                                  std::make_pair<std::string, const Option *>("Hash", &HashSize),
                                                  std::make_pair<std::string, const Option *>("OwnBook", &OwnBook)},
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
            GlobalLogger << std::format("{}: {}\n", moveStr, moveCount);
        totalSum += moveCount;
    }

    double spentTime = static_cast<double>((t2 - t1).count()) * 1e-6;
    if constexpr (LogToOut)
        GlobalLogger << std::format("Calculated moves: {} in time: {}ms\n", totalSum, spentTime);

    return spentTime;
}

#endif // ENGINE_H
