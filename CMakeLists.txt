cmake_minimum_required(VERSION 3.7)
project(chess_ar)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

file( GLOB LIB_SOURCES src/lib/*.cpp )
file( GLOB LIB_HEADERS src/lib/*.h )
add_library( chess_ar_lib ${LIB_SOURCES} ${LIB_HEADERS} )

add_executable(controller src/controller/main.cpp)
add_executable(eye src/eye/main.cpp)
add_executable(gui src/gui/main.cpp)

TARGET_LINK_LIBRARIES(controller chess_ar_lib)
TARGET_LINK_LIBRARIES(eye chess_ar_lib)
TARGET_LINK_LIBRARIES(gui chess_ar_lib)