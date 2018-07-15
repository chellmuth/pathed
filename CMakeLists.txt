cmake_minimum_required(VERSION 3.8.2)
project (pathed)

set (CMAKE_CXX_STANDARD 11)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(pathed ${SOURCES})
