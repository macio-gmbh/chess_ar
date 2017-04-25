//
// Created by marco on 25.04.2017.
//

#ifndef CHESS_AR_CHESSFIGURE_H
#define CHESS_AR_CHESSFIGURE_H

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
};

#endif //CHESS_AR_CHESSFIGURE_H
