//
// Created by marco on 25.04.2017.
//

#include <cstdint>
#include "ChessBoard.h"

ChessBoard::ChessBoard(std::string &fen) {
    char currentLetter;
    vector<std::string> fenList;
    boost::split(fenList, fen,boost::is_any_of(" "));

    uint8_t runner = 0;

    while(runner < 64) {
        currentLetter = fenList[0].at(runner);

        switch (currentLetter) {
            case 'p' : {
                board[runner].color = ChessColor ::BLACK;
                board[runner].figure_type = FigureType::PAWN;
                break;
            }
            case 'r' : {
                board[runner].color = ChessColor ::BLACK;
                board[runner].figure_type = FigureType::ROOK;
                break;
            }
            case 'n' : {
                board[runner].color = ChessColor ::BLACK;
                board[runner].figure_type = FigureType::KNIGHT;
                break;
            }
            case 'b' : {
                board[runner].color = ChessColor ::BLACK;
                board[runner].figure_type = FigureType::BISHOP;
                break;
            }
            case 'q' : {
                board[runner].color = ChessColor ::BLACK;
                board[runner].figure_type = FigureType::QUEEN;
                break;
            }
            case 'k' : {
                board[runner].color = ChessColor ::BLACK;
                board[runner].figure_type = FigureType::KING;
                break;
            }
            case 'P' : {
                board[runner].color = ChessColor ::WHITE;
                board[runner].figure_type = FigureType::PAWN;
                break;
            }
            case 'R' : {
                board[runner].color = ChessColor ::WHITE;
                board[runner].figure_type = FigureType::ROOK;
                break;
            }
            case 'N' : {
                board[runner].color = ChessColor ::WHITE;
                board[runner].figure_type = FigureType::KNIGHT;
                break;
            }
            case 'B' : {
                board[runner].color = ChessColor ::WHITE;
                board[runner].figure_type = FigureType::BISHOP;
                break;
            }
            case 'Q' : {
                board[runner].color = ChessColor ::WHITE;
                board[runner].figure_type = FigureType::QUEEN;
                break;
            }
            case 'K' : {
                board[runner].color = ChessColor ::WHITE;
                board[runner].figure_type = FigureType::KING;
                break;
            }
            case '/' : {
                runner--;
                break;
            }
            case '1' : {
                emtyTileSetter(1, runner);
                break;
            }
            case '2' : {
                emtyTileSetter(2, runner);
                break;
            }
            case '3' : {
                emtyTileSetter(3, runner);
                break;
            }
            case '4' : {
                emtyTileSetter(4, runner);
                break;
            }
            case '5' : {
                emtyTileSetter(5, runner);
                break;
            }
            case '6' : {
                emtyTileSetter(6, runner);
                break;
            }
            case '7' : {
                emtyTileSetter(7, runner);
                break;
            }
            case '8' : {
                emtyTileSetter(8, runner);
                break;
            }
            default:
                break;
        }
    }


}

std::string ChessBoard::toString() {

    int8_t  emptyCtr = 0;
    int8_t  tileCtr  = 0;
    std::string fenString;

    while(tileCtr < 64) {

        if(emptyCtr > 0
           && emptyCtr <= 8
           && board[tileCtr].figure_type != FigureType::EMPTY){

            fenString = fenString + to_string(emptyCtr);
            emptyCtr = 0;
        } else {

            switch (board[tileCtr].figure_type) {
                case FigureType::PAWN : {
                    if(board[tileCtr].color == ChessColor ::BLACK) {
                        fenString = fenString +"p";
                    } else {
                        fenString = fenString +"P";
                    }
                }
                case FigureType::ROOK : {
                    if(board[tileCtr].color == ChessColor ::BLACK) {
                        fenString = fenString +"r";
                    } else {
                        fenString = fenString +"R";
                    }
                }
                case FigureType::KNIGHT : {
                    if(board[tileCtr].color == ChessColor ::BLACK) {
                        fenString = fenString +"n";
                    } else {
                        fenString = fenString +"N";
                    }
                }
                case FigureType::BISHOP : {
                    if(board[tileCtr].color == ChessColor ::BLACK) {
                        fenString = fenString +"b";
                    } else {
                        fenString = fenString +"B";
                    }
                }
                case FigureType::QUEEN : {
                    if(board[tileCtr].color == ChessColor ::BLACK) {
                        fenString = fenString +"q";
                    } else {
                        fenString = fenString +"Q";
                    }
                }
                case FigureType::KING : {
                    if(board[tileCtr].color == ChessColor ::BLACK) {
                        fenString = fenString +"p";
                    } else {
                        fenString = fenString +"P";
                    }
                }
                default: {
                    emptyCtr++;
                }

            }
        }

        tileCtr++;

        //a row has ended
        if(tileCtr % 8 == 0) {
            fenString = fenString +"/";
        }


    }

    //Momentaner Zug
    if(currentMove == ChessColor::WHITE) {
        fenString = fenString + "  w";
    } else {
        fenString = fenString + "  b";
    }



}

void ChessBoard::emtyTileSetter(int8_t emptyTileCtr, uint8_t & runner) {

    for(int8_t i = 0; i < emptyTileCtr; i++) {
        board[runner].color = ChessColor::NONE;
        board[runner].figure_type = FigureType::EMPTY;
        runner++;
    }

}



