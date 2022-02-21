cmake_minimum_required(VERSION 3.14)

project(rclientpp-main)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

if (UNIX)
    if (CMAKE_BUILD_TYPE STREQUAL Release) 
        SET(CMAKE_C_FLAGS "-std=c11 -O2 -W -w")
        SET(CMAKE_CXX_FLAGS "-std=c++11 -O2 -W -w")
    else()
        set(CMAKE_BUILD_TYPE Debug)
        SET(CMAKE_C_FLAGS "-std=c11 -W -w -D_DEBUG -O0 -g -ggdb")
        SET(CMAKE_CXX_FLAGS "-std=c++11 -W -w -D_DEBUG -O0 -g -ggdb")
    endif()
endif()

set(CMAKE_CXX_STANDARD 11)

# libs
add_subdirectory(rclientpp)

# executable files
add_subdirectory(examples)
