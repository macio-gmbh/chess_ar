#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/rabbitmq/RabbitMQReceiver.h"



Zobrist currentZobrist;

// source: https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
// Includes required for SO example: https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
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

std::string askStockfishForBestMove(const char* fen) {
    std::string cmd = "stockfish << EOF\n";
    cmd += "setoption name Hash value 128\n";
    cmd += "setoption name threads value 1\n";
    cmd += "setoption name Contempt Factor value 50\n";
    cmd += "position fen ";
    cmd += fen;
    cmd += "\n";
    cmd += "go depth 4\n";
    cmd += "EOF";
    std::string res = exec(cmd.c_str());
    std::size_t pos = res.find("bestmove");

    res = res.substr (pos);

    return res;
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
    std::string sendStr = currentBoard.toString()+ " " +askStockfishForBestMove(currentTestBoard.toString().c_str());
    std::cout<< "sendStr Test:      " << sendStr <<std::endl;
    guiSender.Send(sendStr.c_str());

    std::string testString =  "rnbqkbnr/pp1ppppp/8/2p5/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";

    while (true) {
      std::string nextBoard = eyeReceiver.Receive();
      Zobrist newZobrist(nextBoard);

      if(currentZobrist.zobristHash != newZobrist.zobristHash) {
           currentZobrist = newZobrist;
           currentBoard = ChessBoard(nextBoard);
            /*
             * TODO:
             * 1. Check if the next board is a valid
             * 2. Check if there is still a casteling right
             *    -set The coressponding values in the board
             * 3. Check if there was an enpassent
             *    -set the coressponding values in the board
             * 4. Set the current moves color in the board
             * 5. Set the current move number in the board
             * 6. Set the current half move number in the board
             */
           //Send and receive best move Request
           sendStr = currentBoard.toString()+ " " +askStockfishForBestMove(currentTestBoard.toString().c_str());
           guiSender.Send(sendStr.c_str());
        }
   }
    return 0;
}