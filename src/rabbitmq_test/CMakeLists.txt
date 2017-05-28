cmake_minimum_required(VERSION 3.7)

add_subdirectory(./../shared_lib shared_lib)
include_directories(${SHARED_LIB_INCLUDE_DIR})

add_executable(rabbitmq-test main.cpp)
target_link_libraries(rabbitmq-test shared_lib)




