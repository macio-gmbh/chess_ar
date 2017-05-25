cmake_minimum_required(VERSION 3.7)
project(chess_ar)

set(CMAKE_CXX_STANDARD 11)   #C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

add_subdirectory(src/shared_lib)
add_subdirectory(src/controller)
add_subdirectory(src/eye)
add_subdirectory(src/gui)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    message(STATUS "${COMPILER_SUPPORTS_CXX11}")
else(COMPILER_SUPPORTS_CXX11)
    message(FATAL_ERROR "${COMPILER_SUPPORTS_CXX11}")
endif(COMPILER_SUPPORTS_CXX11)


