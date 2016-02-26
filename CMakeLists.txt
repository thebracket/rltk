cmake_minimum_required(VERSION 3.1)
project("rltk")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wpedantic -march=native -mtune=native -g -I/opt/local/include")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -L/opt/local/lib -L/usr/lib/i386-linux-gnu -lSDL2 -lSDL2_image -lSDL2_ttf")

add_library(rltk rltk/sdl2.cpp)

add_executable(ex1 examples/ex1/main.cpp)
target_link_libraries(ex1 rltk)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(rltk ${SDL2_LIBRARY})

include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)

include_directories(${SDL2IMAGE_INCLUDE_DIRS})
target_link_libraries(rltk ${SDL2IMAGE_LIBRARIES})

include_directories(${SDL2TTF_INCLUDE_DIRS})
target_link_libraries(rltk ${SDL2TTF_LIBRARIES})

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
find_package(Boost)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(rltk ${Boost_LIBRARIES})
endif()

