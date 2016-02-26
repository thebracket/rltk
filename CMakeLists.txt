cmake_minimum_required(VERSION 3.1)
project("rltk")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake_modules)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wpedantic -march=native -mtune=native -g -I/opt/local/include")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -L/opt/local/lib -L/usr/lib/i386-linux-gnu -lSDL2 -lSDL2_image -lSDL2_ttf")

add_library(rltk rltk/stub.cpp)
add_executable(ex1 examples/ex1/main.cpp)
target_link_libraries(ex1 rltk)

