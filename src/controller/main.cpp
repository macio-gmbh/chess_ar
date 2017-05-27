#include <iostream>
#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"


int main() {
    std::cout << "Hello, World from controller! Initialising ..." << std::endl;
    std::string initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string zobrBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::cout<< "Initial Setup Test: " << initialBoard <<std::endl;

    Zobrist initialZobrist = Zobrist(initialBoard);
    Zobrist testZobrist = Zobrist(zobrBoard);
    ChessBoard  currentBoard = ChessBoard(initialBoard, initialZobrist.zobristHash);
    if (initialZobrist.zobristHash == testZobrist.zobristHash) {
        std::cout<< "Zobrist is equal" <<std::endl;
    }

    std::string res = currentBoard.toString();
    std::cout<< "Initial to_string Test : " << res <<std::endl;



    std::string testBoard = "rnbqkbnr/pp1ppppp/8/2p5/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::cout<< "Initial Setup Test: " << testBoard <<std::endl;

    if (currentBoard.currentHash != Zobrist(testBoard).zobristHash) {
        std::cout<< "Set new Board. Hashes are not equal" <<std::endl;
        currentBoard = ChessBoard(testBoard, Zobrist(testBoard).zobristHash);
    }
    if (currentBoard.currentHash != Zobrist(testBoard).zobristHash) {
        std::cout<< "Set new Board. Hashes are not equal" <<std::endl;
        currentBoard = ChessBoard(testBoard, Zobrist(testBoard).zobristHash);
    }else {
        std::cout<< "Do not set new Board. Hash is equal" <<std::endl;
    }

    std::string res2 = currentBoard.toString();
   std::cout<< "Initial to_string Test : " << res2 <<std::endl;

    return 0;
}