cmake_minimum_required(VERSION 3.6)

IF (WIN32)
    set(OpenCV_FOUND 1)
    set("OpenCV_DIR" "D:/Projekte/Cpp/opencv/build")
endif()
find_package(OpenCV REQUIRED)

add_subdirectory(./../shared_lib shared_lib)
include_directories(${SHARED_LIB_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(eye main.cpp FourierDescriptor.h EyeUtils.h )
add_executable(opencv-test TestOpenCV.cpp)
add_executable(train train.cpp FourierDescriptor.h EyeUtils.h)

target_link_libraries(eye shared_lib)
target_link_libraries(eye ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(opencv-test ${OpenCV_LIBS} )
target_link_libraries(train ${OpenCV_LIBS} ${Boost_LIBRARIES})