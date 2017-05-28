//
// Created by marco on 28.05.2017.
//

#include "RabbitMQSender.h"

RabbitMQSender::RabbitMQSender(const char *hostname, int port, const char *exchange) : RabbitMQBase(hostname, port,
                                                                                                    exchange)

{
}

void RabbitMQSender::Send(char const *message)
{
    // prepare the message and send it
    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("text/plain");
    props.delivery_mode = 1; /* non persistent delivery mode */
    amqp_basic_publish(conn,
                        1,
                        amqp_cstring_bytes(exchange),
                        amqp_cstring_bytes(routingkey),
                        0,
                        0,
                        &props,
                        amqp_cstring_bytes(message));
}
