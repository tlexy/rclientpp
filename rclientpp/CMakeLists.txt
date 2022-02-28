cmake_minimum_required(VERSION 3.14)

project(rclientpp)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11   -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -O2")

FILE (GLOB RCLIENTPP_HEADERS "./core/*.h" "./ds/*.h" "./parser/*.h" "./middleware/*.h")

FILE (GLOB RCLIENTPP_SRCS "./core/*.cpp" "./ds/*.cpp" "./parser/*.cpp" "./middleware/*.cpp")


#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../../libs")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/libs)

add_library(${PROJECT_NAME} STATIC ${RCLIENTPP_HEADERS} ${RCLIENTPP_SRCS})
