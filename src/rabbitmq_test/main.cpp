//
// Created by marco on 28.05.2017.
//

#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/rabbitmq/RabbitMQReceiver.h"


int main() {
    // Sends and recives a message via RabbitMQ, RabbitMQ server needs to be running
    RabbitMQReceiver receiver("localhost", 5672, "eyeExchange");
    RabbitMQSender sender("localhost", 5672, "eyeExchange");
    sender.Send("Rabbit MQ test message");
    printf(receiver.Receive());
    return 0;
}


