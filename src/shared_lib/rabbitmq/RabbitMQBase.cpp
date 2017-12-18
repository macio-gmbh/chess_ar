//
// Created by marco on 28.05.2017.
//

#include "RabbitMQBase.h"




RabbitMQBase::RabbitMQBase(const char *hostname, int port, const char *exchange) : hostname(hostname), port(port),
                                                                                   exchange(exchange)
{

    conn = amqp_new_connection();

    socket = amqp_tcp_socket_new(conn);
    if (!socket) {
        //problem with creating TCP socket
        fprintf(stderr, "Problem with creating TCP socket \n");
        return;
    }

    status = amqp_socket_open(socket, hostname, port);
    if (status) {
        fprintf(stderr, "Problem with opening TCP socket \n");
        return;
    }

    // connect to the server
    amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
    amqp_channel_open(conn, 1);

    // create the exchange (won't create a 2nd one if its already there)
    amqp_exchange_declare(conn, 1, amqp_cstring_bytes(exchange), amqp_cstring_bytes("fanout"),
                          0, 1, 0, 0, amqp_empty_table);

}

RabbitMQBase::~RabbitMQBase() {
    // disconnect the client
    amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);
}
