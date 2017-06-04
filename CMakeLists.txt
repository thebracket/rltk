cmake_minimum_required(VERSION 3.1)
project("rltk")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wpedantic -march=native -mtune=native -g")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -L/opt/local/lib -L/usr/lib/i386-linux-gnu")

add_library(rltk 	rltk/rltk.cpp
					rltk/texture_resources.cpp
					rltk/color_t.cpp
					rltk/virtual_terminal.cpp
					rltk/rng.cpp
					rltk/geometry.cpp
					rltk/input_handler.cpp
					rltk/font_manager.cpp
					rltk/gui.cpp
					rltk/layer_t.cpp
					rltk/gui_control_t.cpp
					rltk/virtual_terminal_sparse.cpp
					rltk/ecs.cpp
					rltk/xml.cpp
					rltk/perlin_noise.cpp
					rltk/rexspeeder.cpp
					rltk/scaling.cpp
)

# Require cereal
find_package(Cereal REQUIRED)
include_directories(${CEREAL_INCLUDE_DIR})

# Require SFML
find_package(SFML 2 COMPONENTS system window graphics REQUIRED)
include_directories(${SFML_INCLUDE_DIR})

# Add all of the example executables and their library dependency
add_executable(ex1 examples/ex1/main.cpp)
add_executable(ex2 examples/ex2/main.cpp)
add_executable(ex3 examples/ex3/main.cpp)
add_executable(ex4 examples/ex4/main.cpp)
add_executable(ex5 examples/ex5/main.cpp)
add_executable(ex6 examples/ex6/main.cpp)
add_executable(ex7 examples/ex7/main.cpp)
add_executable(ex8 examples/ex8/main.cpp)
add_executable(ex9 examples/ex9/main.cpp)
add_executable(ex10 examples/ex10/main.cpp)
add_executable(ex11 examples/ex11/main.cpp)
target_link_libraries(rltk ${SFML_LIBRARIES})
target_link_libraries(ex1 rltk)
target_link_libraries(ex2 rltk)
target_link_libraries(ex3 rltk)
target_link_libraries(ex4 rltk)
target_link_libraries(ex5 rltk)
target_link_libraries(ex6 rltk)
target_link_libraries(ex7 rltk)
target_link_libraries(ex8 rltk)
target_link_libraries(ex9 rltk)
target_link_libraries(ex10 rltk)
target_link_libraries(ex11 rltk)

# We depend upon zlib
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
	target_link_libraries(rltk ${ZLIB_LIBRARIES})
endif()
