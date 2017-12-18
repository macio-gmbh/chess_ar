#include <iostream>
#include <memory>


#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"
#include "../shared_lib/ChessFigure.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/rabbitmq/RabbitMQReceiver.h"
#include "ChessEngineCommunicator.h"

struct ChessField{
	ChessFigure figure;
	int field = 0;
};

enum Error {
	NO_ERROR = 0,
	MISSING_FIGURE = 1,
	INVALID_BOARD = 2,
	WRONG_MOVE = 3,
	INVALID_SWAP = 4,
	WRONG_PLAYER = 5,
	CHECKMATED = 6
};

std::vector<ChessField> CalculateDifference(ChessBoard lastBoard, ChessBoard currentBoard);

Error ValidateMove(std::vector<ChessField> differentFields, ChessColor currentPlayer, bool kingInChess, ChessBoard lastBoard) {
	ChessFigure firstFigure = differentFields.at(0).figure;
	int firstField = differentFields.at(0).field;

	ChessFigure secondFigure = differentFields.at(1).figure;
	int secondField = differentFields.at(1).field;

	if (!(firstFigure.figure_type != EMPTY && secondFigure.figure_type != EMPTY)) {
		ChessFigure currentFigure;
		int field;
		if (firstFigure.figure_type != EMPTY) {
			currentFigure = firstFigure;
			field = firstField;
		}
		else {
			currentFigure = secondFigure;
			field = secondField;
		}
		//Falsche Spielerfigur wurde bewegt
		if (currentFigure.color != currentPlayer) {
			return WRONG_PLAYER;
		}
		//Spieler schlägt seine eigene Figur
		if (lastBoard.GetBoard().at(field).color == currentFigure.color) {
			return WRONG_MOVE;
		}
		//Auge hat eine falsche Figur generiert, wenn zwei Figuren auf einem Feld stehen
		if ((lastBoard.GetBoard().at(firstField).figure_type != currentFigure.figure_type) ||
			(lastBoard.GetBoard().at(secondField).figure_type != currentFigure.figure_type)) {
			return INVALID_BOARD;
		}

		//Richtig gezogen
		int firstColumn = differentFields.at(0).field % 8;
		int firstLine = differentFields.at(0).field / 8;
		int secondColumn = differentFields.at(1).field % 8;
		int secondLine = differentFields.at(1).field / 8;

		int diffCol = firstColumn < secondColumn ? secondColumn - firstColumn : firstColumn - secondColumn;
		int diffLine = firstLine < secondLine ? secondLine - firstLine : firstLine - secondLine;

		switch (currentFigure.figure_type) {
		case KING:
			if (!((diffCol == 1) || (diffLine == 1))) {
				return WRONG_MOVE;
			}

			//if (KingInCheckedmate(lastBoard, field, currentPlayer)) {
			//	return CHECKMATED;
			//}
			break;
		case QUEEN:
			if (!((diffCol == 0 || diffLine == 0) || (diffCol == diffLine))) {
				return WRONG_MOVE;
			}
			break;
		case ROOK:
			if (!(diffCol == 0 || diffLine == 0)) {
				return WRONG_MOVE;
			}
			break;
		case BISHOP:
			if (!(diffCol == diffLine)) {
				return WRONG_MOVE;
			}
			break;
		case KNIGHT:
			if (!((diffCol == 1 && diffLine == 2) || (diffCol == 2 && diffLine == 1))) {
				return WRONG_MOVE;
			}
			break;
		case PAWN: break;
		default:
			return INVALID_BOARD;
		}


	}
	else {
		return INVALID_SWAP;
	}

	return NO_ERROR;
}

bool KingInCheckedmate(ChessBoard lastBoard, int field, ChessColor currentPlayer) {
	int col = field % 8;
	int line = field / 8;
	bool nothingBetween = false;
	//ROOK or QUEEN left on Line
	for (int i = 0; i < col; i++) {
		ChessFigure figure = lastBoard.GetBoard().at((line * 8) + col);
		if (figure.color != currentPlayer) {
			if (figure.figure_type == ROOK || figure.figure_type == QUEEN) {
				return true;
			}
		}
	}

	return false;
}

int main() {



    std::cout << "Hello, World from controller! Initialising ..." << std::endl;
    std::string initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    ChessBoard currentBoard= ChessBoard(initialBoard);
    ChessBoard lastBoard= ChessBoard(initialBoard);

    std::cout<< "Initial Setup Test: " << currentBoard.toString() <<std::endl;
    bool inital = true;

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

      if(currentZobrist.zobristHash != newZobrist.zobristHash || inital) {

          if(!inital ) {
              lastBoard = currentBoard;
              currentZobrist = newZobrist;
              currentBoard = ChessBoard(nextBoard);
              if (currentColor == ChessColor::WHITE) {
                  currentColor = ChessColor::BLACK;

              } else {
                  currentColor = ChessColor::WHITE;
                  fullMove += 1;
              }
              currentBoard.fullMove = fullMove;
              currentBoard.currentMove = currentColor;
          } else {
              inital = false;
          }
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


std::vector<ChessField> CalculateDifference(ChessBoard lastBoard, ChessBoard currentBoard) {
	std::vector<ChessField> difference;

	std::array<ChessFigure, 64> lastBoardSituation = lastBoard.GetBoard();
	std::array<ChessFigure, 64> currentBoardSituation = currentBoard.GetBoard();

	ChessField aField;

	for (int field = 0; field < 64; field++) {
		if (lastBoardSituation.at(field).color != currentBoardSituation.at(field).color &&
			lastBoardSituation.at(field).figure_type != currentBoardSituation.at(field).figure_type) {
			aField.figure = currentBoardSituation.at(field);
			aField.field = field;

			difference.push_back(aField);
		}
	}

	return difference;
}
