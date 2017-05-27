//
// Created by marco on 25.04.2017.
//

#ifndef CHESS_AR_CHESSBOARD_H
#define CHESS_AR_CHESSBOARD_H


#include <boost/algorithm/string.hpp>
#include <boost/container/vector.hpp>
#include "ChessFigure.h"

using namespace boost::container;
class ChessBoard {
public:
    ChessFigure board[64];

    //default to white, since it is the starting color
    ChessColor currentMove = ChessColor::WHITE;

    uint64_t currentHash;

    /**
     * Receives a FEN-String of a board and converts it to a  chess board.
     * Only the current Player and the Order on the Chess board are currently stored.
     * @param fen Is a string in fen Notation
     */

    ChessBoard(std::string&  fen, std::uint64_t hash);

    std::string toString();

private:

    void emtyTileSetter(int8_t emptyTileCtr, uint8_t & runner);
};


#endif //CHESS_AR_CHESSBOARD_H
