//
// Created by marco on 25.04.2017.
//

#ifndef CHESS_AR_CHESSBOARD_H
#define CHESS_AR_CHESSBOARD_H

#include <string>
#include "ChessFigure.h"

class ChessBoard {
public:
    ChessFigure board[64];
    int last_hash;

    std::string toString();
};


#endif //CHESS_AR_CHESSBOARD_H
