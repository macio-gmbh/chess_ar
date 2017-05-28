cmake_minimum_required(VERSION 3.7)

if(TARGET shared_lib)
    #message("shared_lib is already defined")
else()
    file( GLOB LIB_SOURCES *.cpp rabbitmq/*.cpp)
    file( GLOB LIB_HEADERS *.h  rabbitmq/*h)
    add_library( shared_lib ${LIB_SOURCES} ${LIB_HEADERS} ChessColor.h Zobrist.cpp Zobrist.h)
    TARGET_LINK_LIBRARIES(shared_lib  ${RabbitMQ_LIBRARIES})

endif()

