//
// Created by marco on 25.04.2017.
//

#ifndef CHESS_AR_CHESSBOARD_H
#define CHESS_AR_CHESSBOARD_H


#include <boost/algorithm/string.hpp>
#include <string>
#include <array>
#include <stdexcept>
#include <boost/container/vector.hpp>
#include <boost/lexical_cast.hpp>
#include "ChessFigure.h"

using namespace boost::container;
class ChessBoard {
public:
    std::array<ChessFigure,64>  board;

    //default to white, since it is the starting color
    ChessColor currentMove = ChessColor::WHITE;


    bool blackKingSideCastelling = true;
    bool blackQueenSideCastelling = true;
    bool whiteKingSideCastelling = true;
    bool whiteQueenSideCastelling = true;

    //is -1 when no enpassent. enpassent is the array index of the field
    int_fast8_t  enpassent = -1;

    uint16_t  halfMove = 0;

    uint16_t  fullMove = 0;
    /**
     *
     * Default constructor
     * @return
     */
    ChessBoard Chessboard();
    
    /**
     * Receives a FEN-String of a board and converts it to a  chess board.
     * Only the current Player and the Order on the Chess board are currently stored.
     * @param fen Is a string in fen Notation
     */

    ChessBoard(std::string&  fen);

    /**
    * Receives an array of Chess figures
     * @param board Is a string in fen Notation
    */
    ChessBoard Chessboard(
            std::array<ChessFigure,64>  &newBoard,
            ChessColor curMove,
            bool newblackKingSideCastelling ,
            bool newblackQueenSideCastelling ,
            bool newwhiteKingSideCastelling ,
            bool newwhiteQueenSideCastelling ,
            int_fast8_t  newenpassent,
            uint16_t  newhalfMove,
            uint16_t  newfullMove);


    std::string toString();

private:


    void emtyTileSetter(int8_t emptyTileCtr, uint8_t & runner);

};


#endif //CHESS_AR_CHESSBOARD_H
