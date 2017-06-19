cmake_minimum_required(VERSION 3.6)

add_subdirectory(./../shared_lib shared_lib)
include_directories(${SHARED_LIB_INCLUDE_DIR})

add_executable(controller main.cpp ChessEngneCommunicator.cpp ChessEngneCommunicator.h)
target_link_libraries(controller shared_lib)


