cmake_minimum_required(VERSION 3.8.2)
project (pathed)
project (pathed_tests)

# Compiler settings
set (CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -lglfw -ldl")

# .h files
include_directories(include)
include_directories(vendor)

# Source files
file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB TESTS "test/*.cpp")

# Core libraries to link
# set(APPKIT "-framework AppKit")
# set(COREFOUNDATION "-framework CoreFoundation")
# set(COREGRAPHICS "-framework CoreGraphics")
# set(COREVIDEO "-framework CoreVideo")
# set(IOKIT "-framework IOKit")
# set(CORELIBS ${COREFOUNDATION} ${COREGRAPHICS} ${COREVIDEO} ${IOKIT} ${APPKIT})

# OpenGL & GLFW
find_package(OpenGL)
find_package(glfw3)

# set(CORELIBS ${CORELIBS} ${GLFW_LIBRARY} ${GLAD_LIBRARY} ${OPENGL_LIBRARY})
set(CORELIBS glfw ${GLFW_LIBRARY} ${GLAD_LIBRARY} ${OPENGL_LIBRARY})

add_executable(pathed app/main.cpp ${SOURCES})
target_link_libraries(pathed ${CORELIBS})

add_executable(pathed_tests ${TESTS} ${SOURCES})
target_link_libraries(pathed_tests ${CORELIBS})
