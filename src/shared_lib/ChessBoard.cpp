//
// Created by marco on 25.04.2017.
//

#include <cstdint>
#include "ChessBoard.h"

ChessBoard::ChessBoard(std::string &fen){
    char currentLetter;
    vector<std::string> fenList;
    boost::split(fenList, fen, boost::is_any_of(" "));

    uint8_t iChar = 0;
    uint8_t iArray = 0;

    while (iArray < 64) {
        currentLetter = fenList[0].at(iChar);
        //point to nextChar
        iChar += 1;

        switch (currentLetter) {
            case 'p' : {
                board[iArray].color = ChessColor::BLACK;
                board[iArray].figure_type = FigureType::PAWN;
                iArray += 1;
                break;
            }
            case 'r' : {
                board[iArray].color = ChessColor::BLACK;
                board[iArray].figure_type = FigureType::ROOK;
                iArray += 1;
                break;
            }
            case 'n' : {
                board[iArray].color = ChessColor::BLACK;
                board[iArray].figure_type = FigureType::KNIGHT;
                iArray += 1;
                break;
            }
            case 'b' : {
                board[iArray].color = ChessColor::BLACK;
                board[iArray].figure_type = FigureType::BISHOP;
                iArray += 1;
                break;
            }
            case 'q' : {
                board[iArray].color = ChessColor::BLACK;
                board[iArray].figure_type = FigureType::QUEEN;
                iArray += 1;
                break;
            }
            case 'k' : {
                board[iArray].color = ChessColor::BLACK;
                board[iArray].figure_type = FigureType::KING;
                iArray += 1;
                break;
            }
            case 'P' : {
                board[iArray].color = ChessColor::WHITE;
                board[iArray].figure_type = FigureType::PAWN;
                iArray += 1;
                break;
            }
            case 'R' : {
                board[iArray].color = ChessColor::WHITE;
                board[iArray].figure_type = FigureType::ROOK;
                iArray += 1;
                break;
            }
            case 'N' : {
                board[iArray].color = ChessColor::WHITE;
                board[iArray].figure_type = FigureType::KNIGHT;
                iArray += 1;
                break;
            }
            case 'B' : {
                board[iArray].color = ChessColor::WHITE;
                board[iArray].figure_type = FigureType::BISHOP;
                iArray += 1;
                break;
            }
            case 'Q' : {
                board[iArray].color = ChessColor::WHITE;
                board[iArray].figure_type = FigureType::QUEEN;
                iArray += 1;
                break;
            }
            case 'K' : {
                board[iArray].color = ChessColor::WHITE;
                board[iArray].figure_type = FigureType::KING;
                iArray += 1;
                break;
            }
            case '/' : {
                break;
            }
            case '1' : {
                emtyTileSetter(1, iArray);
                break;
            }
            case '2' : {
                emtyTileSetter(2, iArray);
                break;
            }
            case '3' : {
                emtyTileSetter(3, iArray);
                break;
            }
            case '4' : {
                emtyTileSetter(4, iArray);
                break;
            }
            case '5' : {
                emtyTileSetter(5, iArray);
                break;
            }
            case '6' : {
                emtyTileSetter(6, iArray);
                break;
            }
            case '7' : {
                emtyTileSetter(7, iArray);
                break;
            }
            case '8' : {
                emtyTileSetter(8, iArray);
                break;
            }
            default:
                break;
        }

    }

    if (fenList.size() > 1) {
        switch (fenList[1].at(0)) {
            case 'w':
                currentMove = ChessColor::WHITE;
                break;
            case 'b':
                currentMove = ChessColor::BLACK;
                break;
            default:
                currentMove = ChessColor::NONE;

        }
    }


}

std::string ChessBoard::toString() {

    int8_t emptyCtr = 0;
    int8_t tileCtr = 0;
    std::string fenString;

    while (tileCtr < 64) {

        switch (board[tileCtr].figure_type) {
            case FigureType::PAWN : {
                if (board[tileCtr].color == ChessColor::BLACK) {
                    fenString = fenString + "p";
                } else {
                    fenString = fenString + "P";
                }
                break;
            }
            case FigureType::ROOK : {
                if (board[tileCtr].color == ChessColor::BLACK) {
                    fenString = fenString + "r";
                } else {
                    fenString = fenString + "R";
                }
                break;
            }
            case FigureType::KNIGHT : {
                if (board[tileCtr].color == ChessColor::BLACK) {
                    fenString = fenString + "n";
                } else {
                    fenString = fenString + "N";
                }
                break;
            }
            case FigureType::BISHOP : {
                if (board[tileCtr].color == ChessColor::BLACK) {
                    fenString = fenString + "b";
                } else {
                    fenString = fenString + "B";
                }
                break;
            }
            case FigureType::QUEEN : {
                if (board[tileCtr].color == ChessColor::BLACK) {
                    fenString = fenString + "q";
                } else {
                    fenString = fenString + "Q";
                }
                break;
            }
            case FigureType::KING : {
                if (board[tileCtr].color == ChessColor::BLACK) {
                    fenString = fenString + "k";
                } else {
                    fenString = fenString + "K";
                }
                break;
            }
            default: {

                emptyCtr++;

                if (emptyCtr <= 8 && ((tileCtr == 63)
                                      || (!board[tileCtr + 1].figure_type == FigureType::EMPTY) ||
                                      (tileCtr + 1) % 8 == 0)) {
                    fenString = fenString + std::to_string(emptyCtr);
                    emptyCtr = 0;
                }

            }

        }


        tileCtr++;

        //a row has ended
        if (tileCtr % 8 == 0 && tileCtr !=64) {
            fenString = fenString + "/";
        }


    }

    //current Move
    if (currentMove == ChessColor::WHITE) {
        fenString = fenString + " w";
    } else {
        fenString = fenString + " b";
    }

    return fenString;


}

void ChessBoard::emtyTileSetter(int8_t emptyTileCtr, uint8_t &runner) {

    for (int8_t i = 0; i < emptyTileCtr; i++) {
        board[runner].color = ChessColor::NONE;
        board[runner].figure_type = FigureType::EMPTY;
        runner++;
    }

}



