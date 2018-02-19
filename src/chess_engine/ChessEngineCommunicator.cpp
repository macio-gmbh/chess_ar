//
// Created by lisa on 6/19/17.
//
#include <iostream>
#include <fstream>

#include "ChessEngineCommunicator.h"

ChessEngineCommunicator::ChessEngineCommunicator() {}


ChessEngineCommunicator::ChessEngineCommunicator(std::string chessEngineName) {
    engine = chessEngineName;
}


// source: https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
// Includes required for SO example: https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
std::string ChessEngineCommunicator::exec(const char* cmd) {
	char buffer[128];
	std::string result = "";
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) throw std::runtime_error("popen() failed!");
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL) {
				result += buffer;
			}
		}
	}
	catch (...) {
		_pclose(pipe);
		throw;
	}
	_pclose(pipe);
	return result;
}

std::string ChessEngineCommunicator::askStockfishForBestMove(const char* fen) {
	/*
	std::ofstream input(this->inputFile);
	clearInputFile(this->inputFile);
	input << "isready\n";
	input << "setoption name threads value 4\n";
	input << "position fen ";
	input << fen;
	input << "\n";
	input << "go depth 4\n";
	//input << "setoption name Skill Level value 10/n";
	input.close();
	std::string res = exec("stockfish_8_x32.exe < input.txt");//exec(cmd.c_str());
	std::cout << res << "\n";
	std::size_t pos = res.find("bestmove");
	
	if (pos != std::string::npos) {
		res = res.substr(pos).substr(9);
		res = res.substr(0, res.length() - 1);

		vector<std::string> stringVector;
		boost::split(stringVector, res, boost::is_any_of(" "));

		std::cout << res << "\n";
		res = stringVector[0];
	}
	else {
		res = "(none)";
	}
	*/
	std::string res;
	switch (halfmove)
	{
	case 1:
		res = "e7e5";
		break;
	case 2:
		res = "b1c3";
		break;
	case 3:
		res = "h7h6";
		break;
	case 4:
		res = "g1f3";
		break;
	case 5:
		res = "b8c6";
		break;
	case 6:
		res = "f1c4";
		break;
	case 7:
		res = "c8g4";
		break;
	case 8:
		res = "f3e5";
		break;
	case 9:
		res = "g4d1";
		break;
	case 10:
		res = "c4f7";
		break;
	case 11:
		res = "e8e7";
		break;
	case 12:
		res = "c3d5";
		break;
	case 13:
		res = "(none)";
		break;
	default:
		halfmove = 0;
		res = "e2e4";
		break;
	}
	this->halfmove++;
	return res;
}

bool ChessEngineCommunicator::moveIsValid(const char* fen, const char* move) {
	std::cout << move << "\n";
	std::ofstream input(this->inputFile);
	clearInputFile(this->inputFile);
	//input << "setoption name Skill Level value 1";
	//input << "setoption name Hash value 1024\n";
	//input << "setoption name threads value 4\n";
	input << "position fen ";
	input << fen;
	input << "\n";
	input << "go depth 4 searchmoves ";
	input << move;
	input << "\n";
	input.close();

	std::string res = exec("stockfish < input.txt");
	std::cout << res << "\n";
	std::size_t pos = res.find("bestmove");
	if (pos != std::string::npos) {
		res = res.substr(pos).substr(9);
		res = res.substr(0, res.length() - 1);
		std::cout << res << "\n";
		if (res == move) {
			return true;
		}
	}
	return false;
}

void ChessEngineCommunicator::clearInputFile(const char *outFilename) {
	std::ofstream ofs;
	ofs.open(outFilename, std::ofstream::out | std::ofstream::trunc);
	if (!ofs.is_open()) {
		std::cout << "Error! Cannot write to output file <" << outFilename << ">";
		exit(EXIT_FAILURE);
	}
	ofs.close();
}

bool ChessEngineCommunicator::isCheck(const char* fen) {
	std::ofstream input(this->inputFile);
	clearInputFile(this->inputFile);
	//input << "setoption name Hash value 1024\n";
	//input << "setoption name threads value 4\n";
	input << "position fen ";
	input << fen;
	input << "\n";
	input << "d";
	input << "\n";
	input.close();

	std::string res = exec("stockfish < input.txt");
	std::size_t pos = res.find("Checkers:");

	if (pos != std::string::npos) {
		res = res.substr(pos).substr(9);
		std::cout << res << "\n";
		if (res.length() > 2) {
			return true;
		}
	}
	return false;
}

void ChessEngineCommunicator::setLevel(int level) {
	std::ofstream input(this->inputFile);
	clearInputFile(this->inputFile);
	input << "setoption name Skill Level value ";
	input << level;
	input << "\n";
	input.close();

	std::string res = exec("stockfish << setoption name Skill Level value 1\n");

	clearInputFile(this->inputFile);
	input << "isready\n";
	input << "setoption name threads value 4\n";
	input << "position fen ";
	input << "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	input << "\n";
	input << "go depth 4\n";
	//input << "setoption name Skill Level value 10/n";
	input.close();
	res = exec("stockfish < input.txt");//exec(cmd.c_str());
	std::cout << "answer is: " << res << " asd\n";
	std::size_t pos = res.find("bestmove");

	if (pos != std::string::npos) {
		res = res.substr(pos).substr(9);
		res = res.substr(0, res.length() - 1);
		std::cout << res << "\n";
	}
	
	
	/*
	std::cout << res;

	clearInputFile(this->inputFile);
	input << "position startposition";
	input << "go depth 4\n";
	input.close();
	res = exec("stockfish < input.txt");
	std::size_t pos = res.find("bestmove");

	if (pos != std::string::npos) {
		res = res.substr(pos).substr(9);
		res = res.substr(0, res.length() - 1);
	}
	else {
		res = "(none)";
	}
	std::cout << res << "\n";*/
}