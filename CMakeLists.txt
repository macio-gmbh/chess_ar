cmake_minimum_required(VERSION 3.6)
project(chess_ar)

set(CMAKE_CXX_STANDARD 11)   #C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

IF (WIN32)
    set(BOOST_ROOT CACHE PATH "Root folder from Boost")
endif()

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

# this needs to be here so it can be accessed from all targets
find_package(Boost 1.50.0 COMPONENTS filesystem serialization REQUIRED)
find_package(Boost REQUIRED COMPONENTS system iostreams)
include_directories(${Boost_INCLUDE_DIRS})

# RabbitMQ C library dependency
FIND_PACKAGE(RabbitMQ REQUIRED)
INCLUDE_DIRECTORIES( ${RabbitMQ_INCLUDE_DIR})

add_subdirectory(src/shared_lib)
add_subdirectory(src/controller)
add_subdirectory(src/eye)
add_subdirectory(src/gui)
add_subdirectory(src/rabbitmq_test)
add_subdirectory(src/chess_engine)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    message(STATUS "${COMPILER_SUPPORTS_CXX11}")
else(COMPILER_SUPPORTS_CXX11)
    message(FATAL_ERROR "${COMPILER_SUPPORTS_CXX11}")
endif(COMPILER_SUPPORTS_CXX11)



