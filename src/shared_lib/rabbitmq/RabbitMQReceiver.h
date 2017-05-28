//
// Created by marco on 28.05.2017.
//

#ifndef CHESS_AR_RABBITMQRECEIVER_H
#define CHESS_AR_RABBITMQRECEIVER_H

#include "RabbitMQBase.h"
#include <functional>

class RabbitMQReceiver: RabbitMQBase {
public:
    RabbitMQReceiver(const char *hostname, int port, const char *exchange);
    //std::function<void(const char *message)> onReceive;
    const char* Receive();

protected:
    amqp_bytes_t queuename;
};


#endif //CHESS_AR_RABBITMQRECEIVER_H
