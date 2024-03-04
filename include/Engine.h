//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <chrono>
#include <map>

#include "Interface/UCIOptions.h"
#include "EngineTypeDefs.h"
#include "Interface/FenTranslator.h"
#include "Interface/Logger.h"
#include "OpeningBook/OpeningBook.h"

class Engine
{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------
public:
    Engine() = default;

    Engine(const Board&bd): _board(bd), _startingBoard(bd)
    {
    }

    ~Engine() = default;

    void Initialize();

    // ------------------------------
    // Type interaction
    // ------------------------------


    // Implemented function
    void writeBoard() const;

    std::map<std::string, uint64_t> GetPerft(int depth);

    template<
        bool LogToOut = true
        >
    double GoPerft(int depth);

    void SetFenPosition(const std::string&fenStr);

    static const EngineInfo& GetEngineInfo();

    bool ApplyMoves(const std::vector<std::string>&UCIMoves);

    void RestartEngine();

    [[nodiscard]] Board GetUnderlyingBoardCopy() const;

    [[nodiscard]] std::string GetFenTranslation() const;

    void GoMoveTime(lli time, const std::vector<std::string>& moves) const;

    void GoDepth(const int depth, const std::vector<std::string>& moves) const;


    // TODO: next goals:
    void StopSearch() { std::cout << "stop search resullt! " << std::endl; }
    void GoInfinite() { std::cout << "go infinite result! " << std::endl; }

    // ------------------------------
    // private methods
    // ------------------------------
private:
    static bool _applyMove(Board&board, const std::string&move, uint64_t oldPos, uint64_t newPos);

    static void _changeDebugState(Engine&eng, std::string&nPath);

    static void _changeThreadCount(Engine&eng, const lli tCount)
    {
        std::cout << "New thread count: " << tCount << '\n';
    }

    // ------------------------------
    // private fields
    // ------------------------------

    Board _board{};
    Board _startingBoard{};
    OpeningBook _book{"uci_ready_long", OpeningBook::bookFileType::text_uci} ;

    // ------------------------------
    // Engine options
    // ------------------------------

    lli _threadCount = 1;
    std::string _debugPath;

    // Options available in engine
    inline static const OptionT<Option::OptionType::spin> Threads{"Threads", _changeThreadCount, 1, 1024, 1};
    inline static const OptionT<Option::OptionType::string> DebugLogFile{"Debug Log File", _changeDebugState, ""};

    inline static const EngineInfo engineInfo = {
        .author = "Jakub Lisowski, Warsaw University of Technology",
        .name = "ChessEngine development version 0.04",
        .options = std::map<std::string, const Option *>({
            std::make_pair("Threads", &Threads),
            std::make_pair("Debug Log File", &DebugLogFile),
        }),
    };
};

template<bool LogToOut>
double Engine::GoPerft(const int depth)
{
    const auto t1 = std::chrono::steady_clock::now();
    auto moves = GetPerft(depth);
    const auto t2 = std::chrono::steady_clock::now();

    uint64_t totalSum{};
    for (const auto&[moveStr, moveCount]: moves)
    {
        if constexpr (LogToOut) GlobalLogger.StartLogging() << std::format("{}: {}\n", moveStr, moveCount);
        totalSum += moveCount;
    }

    double spentTime = static_cast<double>((t2 - t1).count()) * 1e-6;
    if constexpr (LogToOut) GlobalLogger.StartLogging() << std::format("Calculated moves: {} in time: {}ms\n", totalSum,
                                                                       spentTime);

    return spentTime;
}

#endif //ENGINE_H
