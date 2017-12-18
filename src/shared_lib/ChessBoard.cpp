//
// Created by marco on 25.04.2017.
//

#include <cstdint>
#include <iostream>
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
               board.at(iArray).color = ChessColor::BLACK;
               board.at(iArray).figure_type = FigureType::PAWN;
                iArray += 1;
                break;
            }
            case 'r' : {
               board.at(iArray).color = ChessColor::BLACK;
               board.at(iArray).figure_type = FigureType::ROOK;
                iArray += 1;
                break;
            }
            case 'n' : {
               board.at(iArray).color = ChessColor::BLACK;
               board.at(iArray).figure_type = FigureType::KNIGHT;
                iArray += 1;
                break;
            }
            case 'b' : {
               board.at(iArray).color = ChessColor::BLACK;
               board.at(iArray).figure_type = FigureType::BISHOP;
                iArray += 1;
                break;
            }
            case 'q' : {
               board.at(iArray).color = ChessColor::BLACK;
               board.at(iArray).figure_type = FigureType::QUEEN;
                iArray += 1;
                break;
            }
            case 'k' : {
               board.at(iArray).color = ChessColor::BLACK;
               board.at(iArray).figure_type = FigureType::KING;
                iArray += 1;
                break;
            }
            case 'P' : {
               board.at(iArray).color = ChessColor::WHITE;
               board.at(iArray).figure_type = FigureType::PAWN;
                iArray += 1;
                break;
            }
            case 'R' : {
               board.at(iArray).color = ChessColor::WHITE;
               board.at(iArray).figure_type = FigureType::ROOK;
                iArray += 1;
                break;
            }
            case 'N' : {
               board.at(iArray).color = ChessColor::WHITE;
               board.at(iArray).figure_type = FigureType::KNIGHT;
                iArray += 1;
                break;
            }
            case 'B' : {
               board.at(iArray).color = ChessColor::WHITE;
               board.at(iArray).figure_type = FigureType::BISHOP;
                iArray += 1;
                break;
            }
            case 'Q' : {
               board.at(iArray).color = ChessColor::WHITE;
               board.at(iArray).figure_type = FigureType::QUEEN;
                iArray += 1;
                break;
            }
            case 'K' : {
               board.at(iArray).color = ChessColor::WHITE;
               board.at(iArray).figure_type = FigureType::KING;
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

    if (fenList.size() > 2) {

        if(fenList[2].at(0) == 'K') {
            whiteKingSideCastelling =true;
        } else {
            whiteKingSideCastelling =false;
        }

        if(fenList[2].at(1) == 'Q') {
            whiteQueenSideCastelling = true;
        } else {
            whiteQueenSideCastelling = false;
        }
        if(fenList[2].at(2) == 'k') {
            blackKingSideCastelling =true;
        } else {
            blackKingSideCastelling =false;
        }
        if(fenList[2].at(3) == 'q') {
            blackQueenSideCastelling = true;
        } else {
            blackQueenSideCastelling = false;
        }
    }

    if (fenList.size() > 3) {
        for (std::string::iterator it = fenList[3].begin(); it != fenList[3].end(); ++it) {
            switch (*it) {
                case '-':
                    enpassent = -1;
                    break;
                case 'a':
                    enpassent = 0;
                    break;
                case 'b':
                    enpassent = 1;
                    break;
                case 'c':
                    enpassent = 2;
                    break;
                case 'd':
                    enpassent = 3;
                    break;
                case 'e':
                    enpassent = 4;
                    break;
                case 'f':
                    enpassent = 5;
                    break;
                case 'g':
                    enpassent = 6;
                    break;
                case 'h':
                    enpassent = 7;
                    break;
                case '3':
                    enpassent += 16;
                    break;
                case '6':
                    enpassent += 40;
                    break;

                default:
                    currentMove = ChessColor::NONE;

            }
        }
    }

    if (fenList.size() > 4) {
        try {
                 halfMove = std::atoi(fenList[4].c_str());

        } catch ( const std::out_of_range& e) {
            std::cerr << "Out of Range error: " << e.what() << '\n';
        }
    }

    if (fenList.size() > 5) {
        try {
                fullMove = std::atoi(fenList[5].c_str());

        } catch (  const std::out_of_range& e) {
            std::cerr << "Out of Range error: " << e.what() << '\n';
        }
    }

}

std::string ChessBoard::toString() {

    int8_t emptyCtr = 0;
    int8_t tileCtr = 0;
    std::string fenString;

    while (tileCtr < 64) {

        switch (board.at(tileCtr).figure_type) {
            case FigureType::PAWN : {
                if (board.at(tileCtr).color == ChessColor::BLACK) {
                    fenString = fenString + "p";
                } else {
                    fenString = fenString + "P";
                }
                break;
            }
            case FigureType::ROOK : {
                if (board.at(tileCtr).color == ChessColor::BLACK) {
                    fenString = fenString + "r";
                } else {
                    fenString = fenString + "R";
                }
                break;
            }
            case FigureType::KNIGHT : {
                if (board.at(tileCtr).color == ChessColor::BLACK) {
                    fenString = fenString + "n";
                } else {
                    fenString = fenString + "N";
                }
                break;
            }
            case FigureType::BISHOP : {
                if (board.at(tileCtr).color == ChessColor::BLACK) {
                    fenString = fenString + "b";
                } else {
                    fenString = fenString + "B";
                }
                break;
            }
            case FigureType::QUEEN : {
                if (board.at(tileCtr).color == ChessColor::BLACK) {
                    fenString = fenString + "q";
                } else {
                    fenString = fenString + "Q";
                }
                break;
            }
            case FigureType::KING : {
                if (board.at(tileCtr).color == ChessColor::BLACK) {
                    fenString = fenString + "k";
                } else {
                    fenString = fenString + "K";
                }
                break;
            }
            default: {

                emptyCtr++;

                if (emptyCtr <= 8 && ((tileCtr == 63)
                                      || (!board.at(tileCtr + 1).figure_type == FigureType::EMPTY) ||
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


    //castelling

        if(whiteKingSideCastelling) {
            fenString = fenString + " K";
        } else {
            fenString = fenString + " -";
        }

        if(whiteQueenSideCastelling) {
            fenString = fenString + "Q";
        } else {
            fenString = fenString + "-";
        }
        if(blackKingSideCastelling) {
            fenString = fenString + "k";
        } else {
            fenString = fenString + "-";
        }
        if(blackQueenSideCastelling) {
            fenString = fenString + "q";
        } else {
            fenString = fenString + "-";
        }


    //enpassent
    if(enpassent ==-1 || enpassent >63) {
        fenString = fenString + " -";
    } else {
        int mod= enpassent % 8;

        switch (mod) {
            case 0:
                fenString += " a";
                break;
            case 1:
                fenString += " b";
                break;
            case 2:
                fenString += " c";
                break;
            case 3:
                fenString += " d";
                break;
            case 4:
                fenString += " e";
                break;
            case 5:
                fenString += " f";
                break;
            case 6:
                fenString += " g";
                break;
            case 7:
                fenString += " h";
                break;
        }
        if(enpassent >= 16 && enpassent < 24) {
            fenString += "3";
        }
        if(enpassent >= 40 && enpassent < 48) {
            fenString += "6";
        }
    }

    fenString = fenString + " " + std::to_string(halfMove);

    fenString = fenString + " " + std::to_string(fullMove);
    return fenString;


}

void ChessBoard::emtyTileSetter(int8_t emptyTileCtr, uint8_t &runner) {

    for (int8_t i = 0; i < emptyTileCtr; i++) {
        board.at(runner).color = ChessColor::NONE;
        board.at(runner).figure_type = FigureType::EMPTY;
        runner++;
    }

}

std::array<ChessFigure, 64> ChessBoard::GetBoard() {
	return this->board;
}



