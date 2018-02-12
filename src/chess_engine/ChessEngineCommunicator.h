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

#include <thread>
#include <mutex> 

#ifndef CHESS_AR_CHESSENGNECOMMUNICATOR_H
#define CHESS_AR_CHESSENGNECOMMUNICATOR_H

// boost includes
#include <boost/process.hpp>
#include <boost/process/pipe.hpp>
//#include <boost/process/mitigate.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

// std includes
#include <string>
#include <sstream>

using namespace boost::process;
using namespace boost::iostreams;

using std::string;
using std::stringstream;

class ChessEngineCommunicator {

    std::string engine;
public:
    ChessEngineCommunicator();
	//virtual ~ChessEngineCommunicator();

    ChessEngineCommunicator(std::string chessEngineName);
    std::string exec(const char* cmd);

    /*
     * kleiner HACK
     */
    std::string askStockfishForBestMove(const char* fen);

	bool moveIsValid(const char* fen, const char* move);

	bool isCheck(const char* fen);
	
	void clearInputFile(const char *outFilename);

	void setLevel(int level);

private:
	const char* inputFile = "input.txt";

	int halfmove = 0;
};


#endif //CHESS_AR_CHESSENGNECOMMUNICATOR_H
