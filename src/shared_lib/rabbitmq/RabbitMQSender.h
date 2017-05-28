//
// Created by marco on 28.05.2017.
//

#ifndef CHESS_AR_RABBITMQSENDER_H
#define CHESS_AR_RABBITMQSENDER_H

#include "RabbitMQBase.h"

class RabbitMQSender: RabbitMQBase {

public:
    RabbitMQSender(const char *hostname, int port, const char *exchange);
    void Send(char const *message);
};


#endif //CHESS_AR_RABBITMQSENDER_H
