//
// Created by lisa on 6/19/17.
//

#include "ChessEngneCommunicator.h"

ChessEngneCommunicator::ChessEngneCommunicator() {}

ChessEngneCommunicator::ChessEngneCommunicator(std::string chessEngineName) {
    engine = chessEngineName;
}


// source: https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
// Includes required for SO example: https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
std::string ChessEngneCommunicator::exec(const char* cmd) {
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

std::string ChessEngneCommunicator::askStockfishForBestMove(const char* fen) {
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