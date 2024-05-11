//
// Created by Jlisowskyy on 3/13/24.
//

#include "../include/Search/ZobristHash.h"

#include <random>

ZobristHasher ZHasher{ZobristHasher::BaseSeed};

ZobristHasher::ZobristHasher(const uint64_t engineSeed)
{
    std::mt19937_64 randEngine{engineSeed};

    // filling board hashes except sentinel board
    for (size_t bd = 0; bd < Board::BitBoardsCount; ++bd)
        for (size_t field = 0; field < Board::BitBoardFields; ++field) _mainHashes[bd][field] = randEngine();

    // sentinel board should not affect
    for (size_t field = 0; field < Board::BitBoardFields; ++field) _mainHashes[Board::SentinelBoardIndex][field] = 0;

    _colorHash = randEngine();

    // filling castlings withou sentinel field
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
            boardMap ^= maxMsbPossible >> boardMapPos;
        }
    }

    // hashing color
    if (board.MovingColor == BLACK)
        hash ^= _colorHash;

    // hashing castling possibilites
    hash ^= _castlingHashes[board.Castlings.to_ullong()];

    // hashing el passant field
    hash ^= _elPassantHashes[ExtractMsbPos(board.ElPassantField)];

    return hash;
}