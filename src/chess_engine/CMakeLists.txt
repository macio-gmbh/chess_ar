cmake_minimum_required(VERSION 3.6)

add_subdirectory(./../shared_lib shared_lib)
include_directories(${SHARED_LIB_INCLUDE_DIR})

add_executable(chess_engine main.cpp ChessEngineCommunicator.cpp ChessEngineCommunicator.h)
target_link_libraries(chess_engine shared_lib)
target_link_libraries(chess_engine ${Boost_LIBRARIES})
