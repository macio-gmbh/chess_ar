cmake_minimum_required(VERSION 3.7)
project(chess_ar)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

add_subdirectory(src/shared_lib)
add_subdirectory(src/controller)
add_subdirectory(src/eye)
add_subdirectory(src/gui)



