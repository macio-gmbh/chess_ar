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
    std::cout << fen << std::endl;
    std::cout << std::endl;
    vector<std::string> fenList;
    boost::split(fenList, fen, boost::is_any_of(" "));
  //  std::cout << fenList.size()<< std::endl;

    int  row = 8;
    std::cout <<"               A   B   C   D   E   F   G   H"<< std::endl;
    std::cout << "         "<<row <<"   | ";
    row-=1;
    for (std::string::iterator it = fenList[0].begin(); it != fenList[0].end(); ++it) {

        if ((*it) == '/') {
            std::cout << std::endl;

            std::cout << "             ---------------------------------" << std::endl;
            std::cout << "         "<<row <<"   | ";
            row-=1;
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



    if (fenList.size() > 7) {
        std::cout << "Next recommened move is:  " << fenList[7]<<std::endl;
    }


}

int main() {
    RabbitMQReceiver guiReceiver("localhost", 5672, "ControllerToGui");

    std::cout << "Hello, World from gui!" << std::endl;
    std::cout << "--------------- This is a test output---------------" << std::endl;


    while (true) {
        boardPrinter(guiReceiver.Receive());
    }
    return 0;
}


