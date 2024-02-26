//
// Created by Jlisowskyy on 2/26/24.
//

#include "../include/OpeningBook/OpeningBook.h"

#include <fstream>
#include <format>

#include "../include/ParseTools.h"


// OpenningBook::OpenningBook(const std::string& bookPath, const bookFileType type):
//     _moveTrie(std::move(type == bookFileType::text_uci ? _readTextBook(bookPath) : _readBinaryBook(bookPath)))
// {}

OpenningBook::OpenningBook(const std::string& bookPath, const bookFileType type)
{
    switch (type)
    {
        case bookFileType::binary:
            _readBinaryBook(bookPath);
            break;
        case bookFileType::text_uci:
            _readTextBook(bookPath);
            break;
    }
}

void OpenningBook::SaveToBinary(const std::string&outputPath) const
{
    throw std::runtime_error("[ ERROR ] Not yet implemented!");
}

const std::string& OpenningBook::GetRandomNextMove(const std::vector<std::string>&moves) const
{
    static std::mt19937_64 randEng((std::chrono::steady_clock::now().time_since_epoch().count()));

    auto nextMoves = _moveTrie.FindNextMoves(moves);
    if (nextMoves.empty()) return "";

    return nextMoves[randEng() % nextMoves.size()];
}

const std::vector<std::string>& OpenningBook::GetAllPossibleNextMoves(const std::vector<std::string>& moves) const
{
    return _moveTrie.FindNextMoves(moves);
}

void OpenningBook::_readBinaryBook(const std::string&bookPath)
{
    throw std::runtime_error("[ ERROR ] Not yet implemented!");
}

void OpenningBook::_readTextBook(const std::string&bookPath)
{
    std::ifstream stream(bookPath);

    if (!stream)
        throw std::runtime_error(std::format("[ ERROR ] Not able to correctly open book from passed path:\n\t{}\n", bookPath));

    for (std::string lineBuff{}; std::getline(stream, lineBuff);)
    {
        auto moveVect = ParseTools::Split(lineBuff);
        _moveTrie.AddMoveSequence(moveVect);
    }

    _moveTrie.OptimiseMemoryUsage();
    stream.close();
}
