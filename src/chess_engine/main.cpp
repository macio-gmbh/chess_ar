#include <iostream>
#include <memory>


#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"
#include "../shared_lib/ChessFigure.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/rabbitmq/RabbitMQReceiver.h"
#include "ChessEngineCommunicator.h"

#include <chrono>

int main() {
    ChessEngineCommunicator engineCommunicator = ChessEngineCommunicator();
    RabbitMQReceiver engineReceiver("localhost", 5672, "ControllerToEngine");
	RabbitMQSender engineSender("localhost", 5672, "EngineToController");
	
	//engineCommunicator.setLevel(1);

	std::string input;

	std::string sendStr;
	bool moveIsValid = false;
	bool isCheck = false;

	while (true) {
		input = engineReceiver.Receive();
		__int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		std::cout << now << std::endl;
		
		vector<std::string> stringVector;
		boost::split(stringVector, input, boost::is_any_of(">"));

		if (stringVector[0] == "askStockfish") {
			std::cout << stringVector[1].c_str() << "\n";
			sendStr = engineCommunicator.askStockfishForBestMove(stringVector[1].c_str());
			std::cout << "askStockfish:" << sendStr << "\n";
			engineSender.Send(sendStr.c_str());
		}
		else if (stringVector[0] == "moveIsValid") {
			std::cout << stringVector[1].c_str() << "\n";
			std::cout << stringVector[2].c_str() << "\n";
			moveIsValid = engineCommunicator.moveIsValid(stringVector[1].c_str(), stringVector[2].c_str());
			std::cout << "moveIsValid:" << moveIsValid << "\n";
			sendStr = moveIsValid ? "true" : "false";
			engineSender.Send(sendStr.c_str());
		}
		else if (stringVector[0] == "isCheck") {
			std::cout << stringVector[1].c_str() << "\n";
			isCheck = engineCommunicator.isCheck(stringVector[1].c_str());
			std::cout << "isCheck:" << isCheck << "\n";
			sendStr = isCheck ? "true" : "false";
			engineSender.Send(sendStr.c_str());
		}
		else {
			std::cout << "Error occured\n";
		}
	}
	return 0;
}