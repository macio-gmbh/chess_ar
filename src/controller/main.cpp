#include <iostream>
#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/rabbitmq/RabbitMQReceiver.h"


Zobrist currentZobrist;


int main() {

    RabbitMQSender guiSender("localhost", 5672, "ControllerToGui");
    RabbitMQReceiver eyeReceiver("localhost",5672, "EyeToController");

    std::cout << "Hello, World from controller! Initialising ..." << std::endl;
    std::string initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::cout<< "Initial Setup Test: " << initialBoard <<std::endl;
    ChessBoard currentBoard= ChessBoard(initialBoard);
    currentZobrist = Zobrist(initialBoard);

    std::cout<< "Initial Setup Test: " << currentBoard.toString() <<std::endl;

    initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq f3 0 1";
    std::cout<< "Initial Setup Test: " << initialBoard <<std::endl;
    ChessBoard currentTestBoard= ChessBoard(initialBoard);
    currentZobrist = Zobrist(initialBoard);

    std::cout<< "toString Test:      " << currentTestBoard.toString() <<std::endl;
 //   guiSender.Send(currentBoard.toString().);

//    std::string testString =  "rnbqkbnr/pp1ppppp/8/2p5/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";
//
//    while (true) {
//        std::string nextBoard = eyeReceiver.Receive();
//        Zobrist newZobrist(nextBoard);
//
//        if(currentZobrist.zobristHash != newZobrist.zobristHash) {
//            currentZobrist = newZobrist;
//            currentBoard = ChessBoard(testString);
//
//            //Send and receive best move Request TODO
//            guiSender.Send(currentBoard.toString().c_str());
//        }
//    }
    return 0;
}