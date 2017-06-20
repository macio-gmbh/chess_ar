//
// Created by marco on 28.05.2017.
//

#include "RabbitMQReceiver.h"

RabbitMQReceiver::RabbitMQReceiver(const char *hostname, int port, const char *exchange) : RabbitMQBase(hostname, port,
                                                                                                        exchange)
{
    // create a neq queue
    amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 0, 1,
                                                    amqp_empty_table);

    if (r == NULL)
    {
        fprintf(stderr, "Cant' create the queue. Maybe rabbitMQ Server is not up? \n");
        return;
    }

    queuename = amqp_bytes_malloc_dup(r->queue);
    if (queuename.bytes == NULL) {
        fprintf(stderr, "Out of memory while copying queue name \n");
        return;
    }

    // binding the queue
    amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(exchange), amqp_cstring_bytes(routingkey),
                    amqp_empty_table);


    // consume the message
    amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
}

/* Convert a amqp_bytes_t to an escaped string form for printing.  We
   use the same escaping conventions as rabbitmqctl.
   Taken from the rabbitmq-c library   */

static char *stringify_bytes(amqp_bytes_t bytes)
{
    /* We will need up to 4 chars per byte, plus the terminating 0 */
    char *res = (char*)malloc(bytes.len * 4 + 1);
    uint8_t *data = (uint8_t*)bytes.bytes;
    char *p = res;
    size_t i;

    for (i = 0; i < bytes.len; i++) {
        if (data[i] >= 32 && data[i] != 127) {
            *p++ = data[i];
        } else {
            *p++ = '\\';
            *p++ = '0' + (data[i] >> 6);
            *p++ = '0' + (data[i] >> 3 & 0x7);
            *p++ = '0' + (data[i] & 0x7);
        }
    }

    *p = 0;
    return res;
}

const char* RabbitMQReceiver::Receive()
{
    const char* message;

    amqp_rpc_reply_t res;
    amqp_envelope_t envelope;

    amqp_maybe_release_buffers(conn);

    res = amqp_consume_message(conn, &envelope, NULL, 0);

    if (AMQP_RESPONSE_NORMAL != res.reply_type) {
        //error
    }

    message = stringify_bytes(envelope.message.body);
    amqp_destroy_envelope(&envelope);
    return message;
}
