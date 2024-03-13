//
// Created by Jlisowskyy on 3/13/24.
//

#include "../include/Search/ZobristHash.h"

#include <random>

ZobristHasher ZHasher{};

ZobristHasher::ZobristHasher(const uint64_t engineSeed) {
    std::mt19937_64 randEngine{engineSeed};

    // filling board hashes except sentinel board
    for (size_t bd = 0; bd < Board::BoardsCount; ++bd)
        for (size_t field = 0; field < Board::BoardFields; ++field)
            _mainHashes[bd][field] = randEngine();

    // sentinel board should not affect
    for (size_t field = 0; field < Board::BoardFields; ++field)
        _mainHashes[Board::SentinelBoardIndex][field] = 0;

    _colorHash = randEngine();

    // filling castlings withou sentinel field
    static constexpr size_t noSentinelCastling = CastlingHashesCount / 2;
    for (size_t castling = 0; castling < noSentinelCastling; ++castling)
        _castlingHashes[castling] = randEngine();

    // copying distinct hashes to their sentinel guarded copies
    for (size_t castling = 0; castling < noSentinelCastling; ++castling) {
        _castlingHashes[noSentinelCastling + castling] = _castlingHashes[castling];
    }

    // filling el passant boards
    for (unsigned long & _elPassantHashe : _elPassantHashes)
        _elPassantHashe = randEngine();
}

uint64_t ZobristHasher::GenerateHash(const Board& board) const {
    uint64_t hash{};

    // hashing pieces
    for (size_t boardInd = 0; boardInd < Board::BoardsCount; ++boardInd) {
        uint64_t boardMap = board.boards[boardInd];

        while (boardMap) {
            const int boardMapPos = ExtractMsbPos(boardMap);
            hash ^= _mainHashes[boardInd][boardMapPos];
            boardMap ^= maxMsbPossible >> boardMapPos;
        }
    }

    // hashing color
    if (board.movColor == BLACK) hash ^= _colorHash;

    // hashing castling possibilites
    hash ^= _castlingHashes[board.Castlings.to_ullong()];

    // hashing el passant field
    hash ^= _elPassantHashes[ExtractMsbPos(board.elPassantField)];

    return hash;
}

uint64_t ZobristHasher::UpdateHash(uint64_t oldHash, const Move mv, const uint64_t oldElPassant,
                                   const std::bitset<Board::CastlingCount + 1> oldCastlings) const {
    oldHash ^= _colorHash;
    oldHash ^= _mainHashes[mv.GetStartField()][mv.GetStartBoardIndex()]; // placing figure on target square
    oldHash ^= _mainHashes[mv.GetTargetField()][mv.GetTargetBoardIndex()]; // removing figure from start square
    oldHash ^= _mainHashes[mv.GetKilledFigureField()][mv.GetKilledBoardIndex()]; // removing killed figure from board

    // TODO: Reconsider and test if usage of lines below is actually needed:

    oldHash ^= _elPassantHashes[ExtractMsbPos(oldElPassant)]; // removing old elPassantField
    oldHash ^= _elPassantHashes[mv.GetElPassantField()]; // placing new elPassantFiled

    oldHash ^= _castlingHashes[oldCastlings.to_ullong()]; // removing old castlings
    oldHash ^= _castlingHashes[mv.GetCastlingRights().to_ullong()]; // placing new ones

    return oldHash;
}
