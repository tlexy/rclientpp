cmake_minimum_required(VERSION 3.14)

if (CMAKE_BUILD_TYPE STREQUAL Release) 
	set(NODE_TARGET_NAME rclient-main)
else()
	set(CMAKE_BUILD_TYPE Debug)
	set(NODE_TARGET_NAME rclient-maind)
endif()

project(examples)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/rclientpp/)

if(UNIX)
    SET(CMAKE_C_FLAGS "-std=c11 -O2 -W -w")
    SET(CMAKE_CXX_FLAGS "-std=c++11 -W -w")
    set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -O0 -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

SET(EXTRA_LIBS rclientpp)

aux_source_directory(. SRCS)

message(STATUS ${SRCS})

link_directories(${CMAKE_SOURCE_DIR}/libs)

# export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
add_executable(${NODE_TARGET_NAME} ${SRCS})
target_link_libraries(${NODE_TARGET_NAME} ${EXTRA_LIBS})
