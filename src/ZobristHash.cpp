//
// Created by Jlisowskyy on 3/13/24.
//

#include "../include/Search/ZobristHash.h"

#include <random>

ZobristHasher ZHasher{ZobristHasher::BaseSeed};

ZobristHasher::ZobristHasher(const uint64_t engineSeed) { RollParameters(engineSeed); }

uint64_t ZobristHasher::GenerateHash(const Board &board) const
{
    uint64_t hash{};

    // hashing pieces
    for (size_t boardInd = 0; boardInd < Board::BitBoardsCount; ++boardInd)
    {
        uint64_t boardMap = board.BitBoards[boardInd];

        while (boardMap)
        {
            const int boardMapPos = ExtractMsbPos(boardMap);
            hash ^= _mainHashes[boardInd][boardMapPos];
            boardMap ^= MaxMsbPossible >> boardMapPos;
        }
    }

    // hashing color
    if (board.MovingColor == BLACK)
        hash ^= _colorHash;

    // hashing castling possibilites
    const unsigned long long rights = board.Castlings.to_ullong();
    hash ^= _castlingHashes[rights];

    // hashing el passant field
    hash ^= _elPassantHashes[ExtractMsbPos(board.ElPassantField)];

    return hash;
}
void ZobristHasher::RollParameters(const uint64_t seed)
{
    std::mt19937_64 randEngine{seed};

    // filling board hashes except sentinel board
    for (size_t bd = 0; bd < Board::BitBoardsCount; ++bd)
        for (size_t field = 0; field < Board::BitBoardFields; ++field) _mainHashes[bd][field] = randEngine();

    _colorHash = randEngine();

    // filling castlings without sentinel field
    static constexpr size_t noSentinelCastling = CastlingHashesCount / 2;
    for (size_t castling = 0; castling < noSentinelCastling; ++castling) _castlingHashes[castling] = randEngine();

    // copying distinct hashes to their sentinel guarded copies
    for (size_t castling = 0; castling < noSentinelCastling; ++castling)
    {
        _castlingHashes[noSentinelCastling + castling] = _castlingHashes[castling];
    }

    // filling el passant BitBoards
    for (auto &_elPassantHash : _elPassantHashes) _elPassantHash = randEngine();
}

#define CheckNum(a, b)                                                                                                 \
    if (_validateNum(a, b, diffBits, sameCount, log) == false)                                                         \
        return false;
bool ZobristHasher::_validateCase(const uint64_t num, const int diffBits, const bool log) const
{
    int sameCount{};

    // filling board hashes except sentinel board
    for (size_t bd = 0; bd < Board::BitBoardsCount; ++bd)
        for (size_t field = 0; field < Board::BitBoardFields; ++field) CheckNum(_mainHashes[bd][field], num);

    CheckNum(_colorHash, num);

    // filling castlings without sentinel field
    static constexpr size_t noSentinelCastling = CastlingHashesCount / 2;
    for (size_t castling = 0; castling < noSentinelCastling; ++castling) CheckNum(_castlingHashes[castling], num);

    // filling el passant BitBoards
    for (auto &_elPassantHash : _elPassantHashes) CheckNum(_elPassantHash, num);

    return true;
}

bool ZobristHasher::_validateNum(const uint64_t a, const uint64_t b, const int diffBits, int &sameCount, const bool log)
{
    const int diff = CountSameBits(a, b);

    if (diff == 0 && ++sameCount == 2)
    {
        if (log)
            GlobalLogger.LogStream << "Two same hashes found!" << std::endl;
        return false;
    }

    const bool result = diff >= diffBits;

    if (log && !result)
        GlobalLogger.LogStream << std::format("Hashes differ by {} bits", diff) << std::endl;

    return result;
}

#define CheckCase(num)                                                                                                 \
    if (_validateCase(num, diffBits, log) == false)                                                                    \
        return false;
bool ZobristHasher::ValidateQuality(const int diffBits, bool log) const
{
    // filling board hashes except sentinel board
    for (size_t bd = 0; bd < Board::BitBoardsCount; ++bd)
        for (size_t field = 0; field < Board::BitBoardFields; ++field) CheckCase(_mainHashes[bd][field]);

    CheckCase(_colorHash);

    // filling castlings without sentinel field
    static constexpr size_t noSentinelCastling = CastlingHashesCount / 2;
    for (size_t castling = 0; castling < noSentinelCastling; ++castling) CheckCase(_castlingHashes[castling]);

    // filling el passant BitBoards
    for (auto &_elPassantHash : _elPassantHashes) CheckCase(_elPassantHash);

    return true;
}

uint64_t ZobristHasher::SearchForSeed(const uint64_t startSeed, const int bitDiffs, const bool log)
{
    std::mt19937_64 randEngine{static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())};
    uint64_t seed = startSeed;

    for (int triesLeft = MaxRetries; triesLeft > 0; --triesLeft)
    {
        if (log)
            GlobalLogger.LogStream << std::format("Starting test on seed: {}", seed) << std::endl;
        ZobristHasher hasher{seed};

        if (hasher.ValidateQuality(bitDiffs, log))
        {
            GlobalLogger.LogStream << std::format("Seed found: {}", seed) << std::endl;
            return seed;
        }

        seed = randEngine();
    }

    GlobalLogger.LogStream << "Seed not found!" << std::endl;
    return seed;
}
