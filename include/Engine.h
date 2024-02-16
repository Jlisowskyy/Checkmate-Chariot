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
#include "MoveGeneration/BlackPawnMap.h"
#include "MoveGeneration/ChessMechanics.h"
#include "MoveGeneration/WhitePawnMap.h"

class Engine {
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------
public:
    Engine() = default;

    void Initialize();

    // ------------------------------
    // Type interaction
    // ------------------------------


    // Implemented function
    void writeBoard() const;
    std::map<std::string, uint64_t> GetPerft(int depth);
    void GoPerft(int depth);
    void SetFenPosition(const std::string& fenStr);
    static const EngineInfo& GetEngineInfo();
    bool ApplyMoves(const std::vector<std::string>& UCIMoves) {
        Board workBoard = _startingBoard;

        for (auto& move : UCIMoves) {
            const auto [oldPos, newPos] = ExtractPositionsFromEncoding(move);

            DisplayMask(oldPos);
            DisplayMask(newPos);

            if (oldPos == 0 || newPos == 0) return false;
            if (!_applyMove(workBoard, move, oldPos, newPos)) return false;
        }

        _board = workBoard;
        return true;
    }


    // TODO: next goals:
    void RestartEngine() { std::cout << "ucinewgame result! " << std::endl; }
    void StopSearch() { std::cout << "stop search resullt! " << std::endl; }
    void GoDepth(lli depth) { std::cout << "go depth resutl: " << depth << std::endl; }
    void GoMovetime(lli time) { std::cout << "go movetime resutl: " << time << std::endl; }
    void GoInfinite() { std::cout << "go infinite result! " << std::endl; }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    static bool _applyMove(Board& board, const std::string& move, const uint64_t oldPos, const uint64_t newPos) {
        const size_t movingColorIndex = board.movColor*Board::BoardsPerCol;
        size_t movedFigIndex = 6;

        for (size_t i = 0; i < 6; ++i)
            // piece to move found
            if ((board.boards[movingColorIndex + i] & oldPos) != 0)
                movedFigIndex = i;

        // invalid move no figure found
        if (movedFigIndex == 6) return false;

        // fake signal: important property used in further search
        if ((board.boards[movingColorIndex + movedFigIndex] & newPos) != 0) return false;

        // most common situation same board to be placed
        size_t destBoardIndex = movedFigIndex;
        // moving pawn, promotion should be checked additionally
        if (const uint64_t promotingMask = board.movColor == WHITE ? WhitePawnMap::PromotingMask : BlackPawnMap::PromotingMask;
            movedFigIndex == pawnsIndex && (promotingMask & oldPos) != 0)
        {
            // move does not contain promoted figure encoding
            if (move.size() != 5) return false;

            switch (std::toupper(move[4])) {
                case 'R':
                    destBoardIndex = rooksIndex;
                    break;
                case 'N':
                    destBoardIndex = knightsIndex;
                    break;
                case 'Q':
                    destBoardIndex = queensIndex;
                    break;
                case 'B':
                    destBoardIndex = bishopsIndex;
                    break;
                default:
                    return false;
            }
        }

        // Todo: reconsider these two lines below in future:
        Board workBoard = board;
        ChessMechanics mech(workBoard);

        // generating moves
        for (const auto moves = mech.GetPossibleMoveSlow(); const auto& movedBoard : moves)
            // matching board was found
            if ((movedBoard.boards[movingColorIndex + movedFigIndex] & oldPos) == 0
                && (movedBoard.boards[movingColorIndex + destBoardIndex] & newPos) != 0)
            {
                board = movedBoard;
                return true;
            }

        return false;
    }

    static void _changeThreadCount(Engine& eng, const lli tCount) {
        std::cout << "New thread count: " << tCount << '\n';
    }

    static void _changeDebugState(Engine& eng, std::string& nPath);

    // ------------------------------
    // private fields
    // ------------------------------

    Board _board{};
    Board _startingBoard{};


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
        .name = "ChessEngine development version 0.02",
        .options = std::map<std::string, const Option*>({
            std::make_pair("Threads", &Threads),
            std::make_pair("Debug Log File", &DebugLogFile),
        }),
    };
};

#endif //ENGINE_H
