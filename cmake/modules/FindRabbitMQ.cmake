# FindRabbitMQ.cmake
# --
# Find the RabbitMQ library
#
# This module defines:
#   RabbitMQ_INCLUDE_DIRS - where to find amqp.h
#   RabbitMQ_LIBRARIES    - the rabbitmq library
#   RabbitMQ_FOUND        - True if RabbitMQ was found
#   RabbitMQ_LIBS         - the lib files for RabbitMQ

Include(FindModule)
FIND_MODULE(RabbitMQ amqp.h "" "" librabbitmq.so.4 "" "")

if (RabbitMQ_FOUND)
    message("found RabbitMQ")
else()
    message("RabbitMQ Not found")
endif()


