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
	EYE_ERROR = 4
};

bool blackKingSideCastelling;
bool blackQueenSideCastelling;
bool whiteKingSideCastelling;
bool whiteQueenSideCastelling;

std::string enPassant;

ChessColor  currentColor;

int64_t halfMove;

int64_t fullMove;

std::vector<ChessField> CalculateDifference(ChessBoard lastBoard, ChessBoard currentBoard);

Error ValidateMove(std::vector<ChessField> difference, ChessBoard lastBoard, ChessEngineCommunicator engineCommunicator);

std::string ColumnToString(int column);

/*
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
}*/

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

    blackKingSideCastelling = true;
    blackQueenSideCastelling = true;
    whiteKingSideCastelling = true;
    whiteQueenSideCastelling = true;

    //white is first color to move
    currentColor = ChessColor::WHITE;

    //ist the number of halfmoves after the last captured pawn
    halfMove = 0;

    //allways increases after Black moves
    fullMove = 1;



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

Error ValidateMove(std::vector<ChessField> difference, ChessBoard lastBoard, ChessEngineCommunicator engineCommunicator, const char* fen) {
	std::string move;
	if (difference.size == 1) {
		return MISSING_FIGURE;
	}
	else if (difference.size > 4 || difference.size == 3) {
		return INVALID_BOARD;
	}
	else if (difference.size == 4) {
		int king = 0;
		int rook = 0;
		ChessField currentKingField;
		int lastKingField;
		for (int i = 0; i < difference.size; i++) {
			if (difference.at(i).figure.figure_type == ROOK) {
				rook++;
			}
			else if (difference.at(i).figure.figure_type == KING) {
				king++;
				currentKingField = difference.at(i);
			}
		}
		if (king != 1 && rook != 1) {
			return INVALID_BOARD;
		}
		for (int i = 0; i < lastBoard.GetBoard().size; i++) {
			if (lastBoard.GetBoard().at(i).figure_type == KING) {
				if (lastBoard.GetBoard().at(i).color == currentColor) {
					lastKingField = i;
				}
			}
		}
		//validity check
		int lastKingFieldLine = lastKingField / 8;
		int currentKingFieldLine = currentKingField.field / 8;
		move = ColumnToString(lastKingField % 8) + std::to_string(lastKingFieldLine) +
			   ColumnToString(currentKingField.field % 8) + std::to_string(currentKingFieldLine);
		
		if (engineCommunicator.moveIsValid(fen, move.c_str())) {
			//set Castling
			if (currentColor == WHITE) {
				if (currentKingField.field % 8 == 2) {
					whiteQueenSideCastelling = false;
				}
				else {
					whiteKingSideCastelling = false;
				}
			}
			else {
				if (currentKingField.field % 8 == 2) {
					blackQueenSideCastelling = false;
				}
				else {
					blackKingSideCastelling = false;
				}
			}
			return NO_ERROR;
		}
	}
	else if (difference.size == 2) {
		ChessFigure currentFigure;
		int currentField;
		ChessFigure lastFigure;
		int lastField;

		if (difference.at(0).figure.figure_type != EMPTY && difference.at(1).figure.figure_type != EMPTY) {
			return WRONG_MOVE;
		}
		//which one has moved
		if (difference.at(0).figure.figure_type != EMPTY) {
			currentFigure = difference.at(0).figure;
			currentField = difference.at(0).field;
			lastFigure = difference.at(1).figure;
			lastField = difference.at(1).field;
		}
		else {
			currentFigure = difference.at(1).figure;
			currentField = difference.at(1).field;
			lastFigure = difference.at(0).figure;
			lastField = difference.at(0).field;
		}
		//is the right tracked figure
		if (currentFigure.figure_type != lastBoard.GetBoard().at(lastField).figure_type) {
			return EYE_ERROR;
		}
		//validity check
		int lastLine = lastField / 8;
		int currentLine = currentField / 8;
		move = ColumnToString(lastField % 8) + std::to_string(lastLine) +
			   ColumnToString(currentField % 8) + std::to_string(currentLine);

		if (engineCommunicator.moveIsValid(fen, move.c_str())) {
			//enPassant
			if (currentFigure.figure_type == PAWN) {
				//reset halfMove
				halfMove = 0;
				int diffLine = currentLine - lastLine;
				if (diffLine == 2 || diffLine == -2) {
					int fieldBehind = diffLine > 0 ? -1 : 1;
					enPassant = ColumnToString(currentField % 8) + std::to_string(currentLine - fieldBehind);
				}
				else {
					enPassant = "-";
				}
			}
			else {
				enPassant = "-";
			}
			//strike, reset halfMove
			if (currentColor != lastBoard.GetBoard().at(currentField).color &&
				lastBoard.GetBoard().at(currentField).color != NONE) {
				halfMove = 0;
			}
			return NO_ERROR;
		}
		return WRONG_MOVE;
	}
	return INVALID_BOARD;
}

std::string ColumnToString(int column) {
	switch (column) {
	case 0: return "a";
	case 1: return "b";
	case 2: return "c";
	case 3: return "d";
	case 4: return "e";
	case 5: return "f";
	case 6: return "g";
	case 7: return "h";
	default: return "X";
	}
}