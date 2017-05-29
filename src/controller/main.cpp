#include <iostream>
#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/rabbitmq/RabbitMQReceiver.h"


Zobrist currentZobrist;


int main() {

    RabbitMQSender guiSender("localhost", 5672, "ControllerToGui");


    std::cout << "Hello, World from controller! Initialising ..." << std::endl;
    std::string initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::cout<< "Initial Setup Test: " << initialBoard <<std::endl;
    ChessBoard currentBoard= ChessBoard(initialBoard);
    currentZobrist = Zobrist(initialBoard);

    guiSender.Send(currentBoard.toString().c_str());

    std::string testString =  "rnbqkbnr/pp1ppppp/8/2p5/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";
    currentBoard=ChessBoard(testString);
    guiSender.Send(currentBoard.toString().c_str());
    return 0;
}