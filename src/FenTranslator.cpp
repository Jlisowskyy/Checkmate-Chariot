//
// Created by Jlisowskyy on 12/27/23.
//

#include "../include/Interface/FenTranslator.h"

#include "../include/BitOperations.h"
#include "../include/Interface/Logger.h"

const Board& FenTranslator::GetDefault() { return StartBoard; }

Board FenTranslator::Translate(const std::string&fenPos)
{
    Board workBoard{};

    try
    {
        size_t pos = 0;

        pos = _skipBlanks(pos, fenPos);
        pos = _processPositions(workBoard, pos, fenPos);
        pos = _skipBlanks(pos, fenPos);
        pos = _processMovingColor(workBoard, pos, fenPos);
        pos = _skipBlanks(pos, fenPos);
        pos = _processCastlings(workBoard, pos, fenPos);
        pos = _skipBlanks(pos, fenPos);
        _processElPassant(workBoard, pos, fenPos);
        workBoard.kingMSBPositions[WHITE] = ExtractMsbPos(workBoard.boards[kingIndex]);
        workBoard.kingMSBPositions[BLACK] = ExtractMsbPos(workBoard.boards[Board::BoardsPerCol + kingIndex]);
    }
    catch (const std::exception&exc)
    {
        GlobalLogger.LogError(exc.what());
        GlobalLogger.LogError("[ INFO ] Loading default layout...");
        workBoard = StartBoard;
    }

    return workBoard;
}

std::string FenTranslator::Translate(const Board&board)
{
    std::string fenPos{};
    _extractFiguresEncoding(board, fenPos);
    fenPos += ' ';
    fenPos += board.movColor == WHITE ? 'w' : 'b';
    fenPos += ' ';
    fenPos += _extractCastling(board);
    fenPos += ' ';
    fenPos += board.elPassantField == INVALID ? "-" : fieldStrMap.at(board.elPassantField);

    // skpping moves counters - not supported
    fenPos += " 0 1";

    return fenPos;
}

std::string FenTranslator::_extractCastling(const Board&bd)
{
    std::string str{};
    for (size_t i = 0; i < Board::CastlingCount; ++i)
        if (bd.Castlings[i])
            str += CastlingNames[i];

    if (str.empty()) str = "-";
    return str;
}

void FenTranslator::_extractFiguresEncoding(const Board&bd, std::string&fenPos)
{
    int inSeries{};

    for (ssize_t y = 7; y >= 0; --y)
    {
        for (ssize_t x = 0; x < 8; ++x)
        {
            const int bInd = static_cast<int>(y * 8 + x);

            // reading figure from the board
            const auto [res, fig, col] = _extractSingleEncoding(bd, bInd);
            if (res == empty)
            {
                ++inSeries;
                continue;
            }

            // eventually adding empty figure FEN offset
            if (inSeries) fenPos += static_cast<char>('0' + inSeries);
            inSeries = 0;

            fenPos += col == WHITE ? static_cast<char>(std::toupper(fig)) : fig;
        }

        // eventually adding empty figure FEN offset
        if (inSeries) fenPos += static_cast<char>('0' + inSeries);
        inSeries = 0;

        // skipping last slash
        if (y != 0) fenPos += '/';
    }
}

std::tuple<FenTranslator::FieldOccup, char, Color> FenTranslator::_extractSingleEncoding(const Board&bd,
    const int bInd)
{
    const uint64_t map = 1LLU << bInd;

    for (size_t i = 0; i < Board::BoardsCount; ++i)
    {
        if ((map & bd.boards[i]) != 0)
        {
            Color col = i >= Board::BoardsPerCol ? BLACK : WHITE;
            char fig;

            switch (i % Board::BoardsPerCol)
            {
                case pawnsIndex:
                    fig = 'p';
                    break;
                case knightsIndex:
                    fig = 'n';
                    break;
                case bishopsIndex:
                    fig = 'b';
                    break;
                case rooksIndex:
                    fig = 'r';
                    break;
                case queensIndex:
                    fig = 'q';
                    break;
                case kingIndex:
                    fig = 'k';
                    break;
            }

            return {occupied, fig, col};
        }
    }

    return {empty, {}, {}};
}

size_t FenTranslator::_processElPassant(Board&bd, const size_t pos, const std::string&fenPos)
{
    if (pos >= fenPos.length())
        throw std::runtime_error("[ ERROR ] Fen position has invalid castling specified!\n");

    if (fenPos[pos] == '-')
    {
        // ElPassant substring consists of '-' character and some other unnecessary one.
        if (pos + 1 < fenPos.length() && !std::isblank(fenPos[pos + 1]))
            throw std::runtime_error("[ ERROR ] Fen position has invalid castling specified!\n");

        return pos + 1;
    }

    if (pos + 1 >= fenPos.length() || (pos + 2 < fenPos.length() && !std::isblank(fenPos[pos + 2])))
        throw std::runtime_error("[ ERROR ] Invalid field description detected on ElPassant field!\n");

    const auto field = fenPos.substr(pos, 2);
    if (!strFieldMap.contains(field))
        throw std::runtime_error("[ ERROR ] Invalid field description detected on ElPassant field!\n");

    bd.elPassantField = strFieldMap.at(field);
    return pos + 2;
}

size_t FenTranslator::_processCastlings(Board&bd, size_t pos, const std::string&fenPos)
{
    if (pos >= fenPos.length())
        throw std::runtime_error("[ ERROR ] Fen position does not contain information about castling!\n");

    // Castlings not possible!
    if (fenPos[pos] == '-')
    {
        // Castling substring consists of '-' character and some other unnecessary one.
        if (pos + 1 < fenPos.length() && !std::isblank(fenPos[pos + 1]))
            throw std::runtime_error("[ ERROR ] Fen position has invalid castling specified!\n");

        return pos + 1;
    }

    // Processing possible castlings
    int processedPos = 0;
    while (pos < fenPos.length() && !std::isblank(fenPos[pos]))
    {
        size_t ind;

        switch (fenPos[pos])
        {
            case 'K':
                ind = WhiteKingSide;
                break;
            case 'k':
                ind = BlackKingSide;
                break;
            case 'Q':
                ind = WhiteQueenSide;
                break;
            case 'q':
                ind = BlackQueenSide;
                break;
            default:
                throw std::runtime_error(
                    std::format("[ ERROR ] Unrecognized castling specifier: {0}!\n", fenPos[pos])
                );
        }

        if (bd.Castlings[ind] != false)
        {
            throw std::runtime_error("[ ERROR ] Repeated same castling at least two time!\n");
        }

        bd.Castlings[ind] = true;
        ++processedPos;
        ++pos;
    }

    // Empty substring
    if (processedPos == 0)
        throw std::runtime_error("[ ERROR ] Castling possiblities not specified!\n");

    return pos;
}

size_t FenTranslator::_processMovingColor(Board&bd, const size_t pos, const std::string&fenPos)
{
    // Too short fenPos string or too long moving color specyfing substring.
    if (pos >= fenPos.length() || (pos + 1 < fenPos.length() && !std::isblank(fenPos[pos + 1])))
        throw std::runtime_error("[ ERROR ] Fen position has invalid moving color specified!\n");

    // Color detection.
    if (fenPos[pos] == 'w')
        bd.movColor = WHITE;
    else if (fenPos[pos] == 'b')
        bd.movColor = BLACK;
    else
        throw std::runtime_error(std::format(
                "[ ERROR ] Fen position contains invalid character ({0}) on moving color field", fenPos[pos])
        );

    return pos + 1;
}

size_t FenTranslator::_processPositions(Board&bd, size_t pos, const std::string&fenPos)
{
    int processedFields = 0;
    std::string posBuffer = "00";

    while (pos < fenPos.length() && !std::isblank(fenPos[pos]))
    {
        // Encoding position into typical chess notation.
        posBuffer[0] = static_cast<char>(processedFields % 8 + 'a');
        posBuffer[1] = static_cast<char>('8' - (processedFields >> 3));

        // Checking possibilites of enocuntered characters.
        // '/' can be safely omitted due to final fields counting with processedFields variable.
        if (const char val = fenPos[pos]; val >= '1' && val <= '8')
            processedFields += val - '0';
        else if (val != '/')
        {
            _addFigure(posBuffer, val, bd);
            ++processedFields;
        }

        if (processedFields > Board::BoardFields)
            throw std::runtime_error(
                std::format("[ ERROR ] Too much fields are used inside passed fen position!\n")
            );

        ++pos;
    }

    if (processedFields < Board::BoardFields)
        throw std::runtime_error(
            std::format("[ ERROR ] Not enugh fields are used inside passed fen position!\n")
        );

    return pos;
}

void FenTranslator::_addFigure(const std::string&pos, char fig, Board&bd)
{
    const auto field = static_cast<uint64_t>(strFieldMap.at(pos));

    if (!figToDescMap.contains(fig))
        throw std::runtime_error(
            std::format("[ ERROR ] Encountered invalid character ({0})inside fen position description!\n", fig)
        );

    bd.boards[figToDescMap.at(fig)] |= field;

    if (fig == 'K')
        bd.kingMSBPositions[WHITE] = ExtractMsbPos(field);
    else if (fig == 'k')
        bd.kingMSBPositions[BLACK] = ExtractMsbPos(field);
}

size_t FenTranslator::_skipBlanks(size_t pos, const std::string&fenPos)
{
    while (pos < fenPos.length() && std::isblank(fenPos[pos])) { ++pos; }
    return pos;
}
