//
// Created by lisa on 6/19/17.
//

#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"

#ifndef CHESS_AR_CHESSENGNECOMMUNICATOR_H
#define CHESS_AR_CHESSENGNECOMMUNICATOR_H


class ChessEngineCommunicator {

    std::string engine;
public:
    ChessEngineCommunicator();

    ChessEngineCommunicator(std::string chessEngineName);
    std::string exec(const char* cmd);

    /*
     * kleiner HACK
     */
    std::string askStockfishForBestMove(const char* fen);

	bool moveIsValid(const char* fen, const char* move);
	
	void clearInputFile(const char *outFilename);
private:
	const char* inputFile = "input.txt";
};


#endif //CHESS_AR_CHESSENGNECOMMUNICATOR_H
