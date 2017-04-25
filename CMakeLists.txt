cmake_minimum_required(VERSION 3.7)
project(chess_ar)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

add_executable(controller src/controller/main.cpp)
add_executable(eye src/eye/main.cpp)
add_executable(gui src/gui/main.cpp)