//
// Created by Jlisowskyy on 12/27/23.
//

#ifndef FENTRANSLATOR_H
#define FENTRANSLATOR_H

#include <format>
#include <string>

#include "../BitOperations.h"
#include "../EngineUtils.h"

struct FenTranslator
{
    // ------------------------------
    // Class interaction
    // ------------------------------

    static INLINE Board GetDefault()
    {
        Board rv;
        Translate(StartingPosition, rv);
        return rv;
    }

    static bool Translate(const std::string &fenPos, Board &bd)
        // Function simply translates position from FEN notation into inner representation.
        ;

    static Board GetTranslated(const std::string& fenPos);

    static std::string Translate(const Board &board);

    // ------------------------------
    // Inner types
    // ------------------------------

    private:
    enum FieldOccup
    {
        empty,
        occupied
    };

    // ------------------------------
    // private methods
    // ------------------------------

    static size_t _processMovesCounts(size_t pos, const std::string &fenPos, int &counter);

    static std::string _extractCastling(const Board &bd);

    static void _extractFiguresEncoding(const Board &bd, std::string &fenPos)
        // pastes occupancy of whole board straight to pass string buffer
        ;

    static std::tuple<FieldOccup, char, Color> _extractSingleEncoding(const Board &bd, int bInd)
        // extracts information of occupancy on passed board indexed field
        ;

    static size_t _processElPassant(Board &bd, size_t pos, const std::string &fenPos)
        // Function reads from fenPos ElPassant field specifying substring
        // and saves this field inside inner board representation.
        // Returns index of first blank character after that substring or EndOfString.
        ;

    static size_t _processCastlings(Board &bd, size_t pos, const std::string &fenPos)
        // Function reads from fenPos castling specifying substring and applies possibilites accordingly to that string.
        // Returns index of first blank character after that substring or EndOfString.
        ;

    static size_t _processMovingColor(Board &bd, size_t pos, const std::string &fenPos)
        // Function validates and applies moving color from fen notation into inner representation.
        // Returns first blank character after color specifying character, that is pos + 1.
        ;

    static size_t _processPositions(Board &bd, size_t pos, const std::string &fenPos)
        // Function translates fen figure representation to inner board representation.
        // Returns index of first blank character after the solid position substring or EndOfString.
        ;

    static void _addFigure(const std::string &pos, char fig, Board &bd)
        // Function simply adds a figure encoded in 'fig' to board using map translating
        // character encoding to actual figure representation. String 'pos' contains position
        // encoded in string also used to retrieve inner board representation using translating map.
        ;

    static size_t _skipBlanks(size_t pos, const std::string &fenPos)
        // Function returns the first non-blank character inside fenPos substring,
        // which starts at 'pos' index and ends naturally
        ;

    // ------------------------------
    // class fields
    // ------------------------------

    static constexpr char CastlingNames[]{
        'K',
        'Q',
        'k',
        'q',
    };

    public:
    static constexpr auto StartingPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
};

#endif // FENTRANSLATOR_H
