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
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
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
	std::ofstream input(this->inputFile);
	clearInputFile(this->inputFile);
	input << "setoption name Hash value 128\n";
	input << "setoption name threads value 1\n";
	input << "position fen ";
	input << fen;
	input << "\n";
	input << "go depth 4\n";
	input.close();
	std::string res = exec("stockfish < input.txt");//exec(cmd.c_str());
	std::size_t pos = res.find("bestmove");

	if (pos != std::string::npos) {
		res = res.substr(pos);
	}
	else {
		res = "bestmove (none)";
	}
	return res;
	/*
	std::string cmd = "stockfish < EOF\n";
	cmd += "setoption name Hash value 128\n";
	cmd += "setoption name threads value 1\n";
	//cmd += "setoption name Contempt Factor value 50\n";
	cmd += "position fen ";
	cmd += fen;
	cmd += "\n";
	cmd += "go depth 4\n";
	cmd += "EOF";
	std::string res = exec("stockfish < input.txt");//exec(cmd.c_str());
	std::size_t pos = res.find("bestmove");

	if (pos != std::string::npos) {
		res = res.substr(pos);
	}
	else {
		res = "bestmove (none)";
	}

	return res;*/
}

bool ChessEngineCommunicator::moveIsValid(const char* fen, const char* move) {
	std::ofstream input(this->inputFile);
	clearInputFile(this->inputFile);
	input << "setoption name Hash value 128\n";
	input << "setoption name threads value 1\n";
	input << "position fen ";
	input << fen;
	input << "\n";
	input << "go depth 4 searchmoves ";
	input << move;
	input << "\n";
	input.close();

	std::string res = exec("stockfish < input.txt");
	std::size_t pos = res.find("bestmove");
	if (pos != std::string::npos) {
		res = res.substr(pos).substr(9, 4);
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