cmake_minimum_required(VERSION 3.7)

# dirty windows hack change that later to be a environment variable
IF (WIN32)
    set(OpenCV_FOUND 1)
    set("OpenCV_DIR" "D:/Projekte/Cpp/opencv/build")
endif()

find_package(OpenCV REQUIRED)

add_subdirectory(./../shared_lib shared_lib)
include_directories(${SHARED_LIB_INCLUDE_DIR})

add_executable(eye main.cpp )
target_link_libraries(eye shared_lib)
target_link_libraries(eye ${OpenCV_LIBS})
