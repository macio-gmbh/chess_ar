//
// Created by lisa on 5/27/17.
// This class implements a simple form of Zobrist hashing.
// To create the Hash values it uses the original Zobrist hashing random values for
// calculating a hash value.
// This version is able to calculate a Zobrist hash from a FEN notation or from the Chess Board.
//
////
//

#ifndef CHESS_AR_ZOBRIST_H
#define CHESS_AR_ZOBRIST_H


#include <cstdint>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/container/vector.hpp>
#include "ChessBoard.h"

using namespace boost::container;

class Zobrist {

public:
    Zobrist();
    Zobrist(std::string fen);
    Zobrist(ChessBoard board);
    uint64_t  zobristHash;
private:
    static const std::uint64_t pawnZobristKeys[2][64];
    static const std::uint64_t rookZobristKeys[2][64];
    static const std::uint64_t bishopZobristKeys[2][64];
    static const std::uint64_t knightZobristKeys[2][64];
    static const std::uint64_t queenZobristKeys[2][64];
    static const std::uint64_t kingZobristKeys[2][64];

    static  const std::uint64_t whiteMove = 0xf8d626aaaf278509;
    void createZobristFromFen(std::string& fen);
    void createZobristFromBoard(ChessBoard& board);

};


#endif //CHESS_AR_ZOBRIST_H
