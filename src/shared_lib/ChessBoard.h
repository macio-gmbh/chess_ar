//
// Created by marco on 25.04.2017.
//

#ifndef CHESS_AR_CHESSBOARD_H
#define CHESS_AR_CHESSBOARD_H

#include <string>
#include <vector>
#include "ChessFigure.h"

class ChessBoard {
public:
    ChessFigure board[64];

    ChessColor currentMove;

    int last_hash;

    /**
     * Receives a FEN-String of a board and conferts it to a  chess board
     * @param fen Is a string in fen Notation
     */

    ChessBoard(std::string&  fen);

    std::string toString();

private:

    void emtyTileSetter(int8_t emptyTileCtr, uint8_t & runner);
};


#endif //CHESS_AR_CHESSBOARD_H
