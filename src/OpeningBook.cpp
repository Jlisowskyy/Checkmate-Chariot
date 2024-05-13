//
// Created by Jlisowskyy on 2/26/24.
//

#include "../include/OpeningBook/OpeningBook.h"

#include <format>
#include <fstream>
#include <random>

#include "../include/Interface/Logger.h"
#include "../include/ParseTools.h"

OpeningBook::OpeningBook(const std::string &bookPath, const bookFileType type) { LoadBook(bookPath, type); }

void OpeningBook::SaveToBinary([[maybe_unused]] const std::string &outputPath) const
{
    throw std::runtime_error("[ ERROR ] Not yet implemented!");
}

bool OpeningBook::IsLoadedCorrectly() const { return _isCorrectlyLoaded; }

const std::string &OpeningBook::GetRandomNextMove(const std::vector<std::string> &moves) const
{
    static std::string _emptyStr{};
    static std::mt19937_64 randEng((std::chrono::steady_clock::now().time_since_epoch().count()));

    const auto &nextMoves = _moveTrie.FindNextMoves(moves);
    if (nextMoves.empty())
        return _emptyStr;

    return nextMoves[randEng() % nextMoves.size()];
}

const std::vector<std::string> &OpeningBook::GetAllPossibleNextMoves(const std::vector<std::string> &moves) const
{
    return _moveTrie.FindNextMoves(moves);
}

void OpeningBook::_readBinaryBook([[maybe_unused]] const std::string &bookPath)
{
    throw std::runtime_error("[ ERROR ] Not yet implemented!");
}

void OpeningBook::_readTextBook(const std::string &bookPath)
{
    std::ifstream stream(bookPath);

    if (!stream)
        throw std::runtime_error(
            std::format("[ ERROR ] Not able to correctly open book from passed path:\n\t{}\n", bookPath)
        );

    for (std::string lineBuff{}; std::getline(stream, lineBuff);)
    {
        auto moveVect = ParseTools::Split(lineBuff);
        _moveTrie.AddMoveSequence(moveVect);
    }

    _moveTrie.OptimiseMemoryUsage();
    stream.close();
}
