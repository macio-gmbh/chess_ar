#Find the Rabbitmq C library


if(Rabbitmqc_FOUND)
	message("Test")
else()
	message(ERROR " This is bad!")
endif()

# Find the include directories
FIND_PATH(Rabbitmqc_INCLUDE_DIR
	NAMES amqp.h
    HINTS ${Rabbitmqc_DIR}/include
	)

FIND_LIBRARY(Rabbitmqc_LIBRARY
	NAMES rabbitmq
    HINTS ${Rabbitmqc_DIR}/lib
	)

SET(Rabbitmqc_PROCESS_INCLUDES Rabbitmqc_INCLUDE_DIR)
SET(Rabbitmqc_PROCESS_LIBS Rabbitmqc_LIBRARY)
