#include <iostream>
#include <memory>


#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/rabbitmq/RabbitMQReceiver.h"
#include "ChessEngineCommunicator.h"


int main() {



    std::cout << "Hello, World from controller! Initialising ..." << std::endl;
    std::string initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    ChessBoard currentBoard= ChessBoard(initialBoard);
    ChessBoard lastBoard= ChessBoard(initialBoard);

    std::cout<< "Initial Setup Test: " << currentBoard.toString() <<std::endl;

    Zobrist currentZobrist;

    ChessEngineCommunicator engineCommunicator = ChessEngineCommunicator();
    RabbitMQSender guiSender("localhost", 5672, "ControllerToGui");
    RabbitMQReceiver eyeReceiver("localhost",5672, "EyeToController");

    bool blackKingSideCastelling = true;
    bool blackQueenSideCastelling = true;
    bool whiteKingSideCastelling = true;
    bool whiteQueenSideCastelling = true;

    //white is first color to move
    ChessColor  currentColor = ChessColor::WHITE;

    //ist the number of halfmoves after the last captured pawn
    int64_t         halfMove     = 0;

    //allways increases after Black moves
    int64_t         fullMove     = 1;



    std::string sendStr = currentBoard.toString()+ " " +engineCommunicator.askStockfishForBestMove(currentBoard.toString().c_str());
    guiSender.Send(sendStr.c_str());
    while (true) {
      std::string nextBoard = eyeReceiver.Receive();
      Zobrist newZobrist(nextBoard);

      if(currentZobrist.zobristHash != newZobrist.zobristHash) {

           lastBoard = currentBoard;
           currentZobrist = newZobrist;
           currentBoard = ChessBoard(nextBoard);
          if(currentColor == ChessColor::WHITE) {
              currentColor = ChessColor::BLACK;

          } else {
              currentColor = ChessColor::WHITE;
              fullMove +=1;
          }
          currentBoard.fullMove = fullMove;
          currentBoard.currentMove = currentColor;
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
           sendStr = currentBoard.toString()+ " " +engineCommunicator.askStockfishForBestMove(currentBoard.toString().c_str());
           guiSender.Send(sendStr.c_str());
        }
   }
    return 0;
}