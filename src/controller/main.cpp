#include <iostream>
#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/rabbitmq/RabbitMQReceiver.h"

// Includes required for SO example: https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

Zobrist currentZobrist;

// source: https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    }
    return result;
}

int pipeTest() {
    std::string cmd = "stockfish << EOF\n";
    cmd += "setoption name Hash value 128\n";
    cmd += "setoption name threads value 1\n";
    cmd += "setoption name Contempt Factor value 50\n";
    cmd += "position fen r1bqkbnr/pp1ppppp/2n5/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq - 0 3\n";
    cmd += "go movetime 1999\n";
    cmd += "EOF";
    std::cout << exec(cmd.c_str()) << std::endl;
    return 0;
}

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
    return pipeTest();
}