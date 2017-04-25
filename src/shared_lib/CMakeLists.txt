cmake_minimum_required(VERSION 3.7)

if(TARGET shared_lib)
    message("shared_lib is already defined")
else()
    file( GLOB LIB_SOURCES *.cpp )
    file( GLOB LIB_HEADERS *.h )
    add_library( shared_lib ${LIB_SOURCES} ${LIB_HEADERS} )
endif()

