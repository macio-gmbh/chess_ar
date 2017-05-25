#include <iostream>
#include "../shared_lib/ChessBoard.h"


int main() {
    std::cout << "Hello, World from controller! Initialising ..." << std::endl;
    std::string initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::cout<< "Initial Setup Test: " << initialBoard <<std::endl;
    //ChessBoard currentBoard = ChessBoard(initialBoard);
    //std::string res = currentBoard.toString();
    //std::cout<< "Initial to_string Test : " << res <<std::endl;



    std::string testBoard = "rnbqkbnr/pp1ppppp/8/2p5/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::cout<< "Initial Setup Test: " << testBoard <<std::endl;
    ChessBoard currentBoard = ChessBoard(testBoard);
    std::string res2 = currentBoard.toString();
    std::cout<< "Initial to_string Test : " << res2 <<std::endl;

    return 0;
}