//
// Created by marco on 28.05.2017.
//

#ifndef CHESS_AR_RABBITMQBASE_H
#define CHESS_AR_RABBITMQBASE_H

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <functional>

// needs rabbitmq-c library from https://github.com/alanxz/rabbitmq-c
class RabbitMQBase {

public:
    RabbitMQBase(char const *hostname, int port, char const *exchange);
    virtual ~RabbitMQBase();

protected:
    char const *hostname;
    int port, status;
    char const *exchange;
    char const *routingkey = "";
    char const *messagebody;
    amqp_socket_t *socket ;
    amqp_connection_state_t conn;
};

#endif //CHESS_AR_RABBITMQBASE_H
