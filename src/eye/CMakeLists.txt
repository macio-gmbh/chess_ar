cmake_minimum_required(VERSION 3.7)

find_package(OpenCV REQUIRED)

add_subdirectory(./../shared_lib shared_lib)
include_directories(${SHARED_LIB_INCLUDE_DIR})

add_executable(eye main.cpp )
add_executable(opencv-test TestOpenCV.cpp )

target_link_libraries(eye shared_lib)
target_link_libraries(eye ${OpenCV_LIBS})
target_link_libraries(opencv-test ${OpenCV_LIBS})

