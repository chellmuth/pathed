cmake_minimum_required(VERSION 3.8.2)
project (pathed)

# Compiler settings
set (CMAKE_CXX_STANDARD 11)

# .h files
include_directories(include)
include_directories(vendor)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Core libraries to link
set(APPKIT "-framework AppKit")
set(COREFOUNDATION "-framework CoreFoundation")
set(COREGRAPHICS "-framework CoreGraphics")
set(COREVIDEO "-framework CoreVideo")
set(IOKIT "-framework IOKit")
set(CORELIBS ${COREFOUNDATION} ${COREGRAPHICS} ${COREVIDEO} ${IOKIT} ${APPKIT})

# OpenGL & GLFW
find_package(OPENGL)
find_library(GLAD_LIBRARY "glad" HINTS lib/)
find_library(GLFW_LIBRARY "glfw3" HINTS lib/)

set(CORELIBS ${CORELIBS} ${GLFW_LIBRARY} ${GLAD_LIBRARY} ${OPENGL_LIBRARY})

add_executable(pathed ${SOURCES})
target_link_libraries(pathed ${CORELIBS})
