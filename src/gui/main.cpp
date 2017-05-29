#include <iostream>
#include <cctype>
#include <boost/algorithm/string.hpp>
#include <boost/container/vector.hpp>
#include <boost/lexical_cast.hpp>
#include "../shared_lib/rabbitmq/RabbitMQReceiver.h"

using namespace boost::container;

static void boardPrinter(std::string fen) {

    std::cout << " ############  Current game status is:  ############" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    vector<std::string> fenList;
    boost::split(fenList, fen, boost::is_any_of(" "));

    std::cout << "         | ";
    for (std::string::iterator it = fenList[0].begin(); it != fenList[0].end(); ++it) {

        if ((*it) == '/') {
            std::cout << std::endl;

            std::cout << "         ---------------------------------" << std::endl;
            std::cout << "         | ";
        } else if (isdigit(*it)) {
            std::stringstream ss;
            ss << *it;
            int emptySpaces = 0;
            ss >> emptySpaces;
            for (int i = 0; i < emptySpaces; i++) {
                std::cout << "0 | ";
            }
        } else {
            std::cout << *it << " | ";
        }
    }


    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Current player to move is: ";

    if (fenList.size() > 1) {
        switch (fenList[1].at(0)) {
            case 'w':
                std::cout << " WHITE " << std::endl;
                break;
            case 'b':
                std::cout << " BLACK " << std::endl;
                break;
            default:
                std::cout << "unknown" << std::endl;

        }
    }

    std::cout << "Next recommened move is:  TODO" <<std::endl;


}

int main() {
    RabbitMQReceiver guiReceiver("localhost", 5672, "ControllerToGui");

    std::cout << "Hello, World from gui!" << std::endl;
    std::cout << "--------------- This is a test output---------------" << std::endl;
    std::string testBoard = "rnbqkbnr/pp1ppppp/8/2p5/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    while (true) {
        boardPrinter(guiReceiver.Receive());
    }
    return 0;
}


