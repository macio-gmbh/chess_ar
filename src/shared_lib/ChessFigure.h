//
// Created by marco on 25.04.2017.
//

#ifndef CHESS_AR_CHESSFIGURE_H
#define CHESS_AR_CHESSFIGURE_H

#include "ChessColor.h"

enum FigureType {
    EMPTY,
    KING,
    QUEEN,
    ROOK,
    BISHOP,
    KNIGHT,
    PAWN,
};

struct ChessFigure {
    FigureType figure_type = EMPTY;
    ChessColor color       = NONE;
};

#endif //CHESS_AR_CHESSFIGURE_H
