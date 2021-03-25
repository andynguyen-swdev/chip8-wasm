cmake_minimum_required(VERSION 3.17)
project(Chip8)

set(CMAKE_CXX_STANDARD 14)

include_directories(., /Users/andy/emsdk/upstream/emscripten/system/include)

add_executable(Chip8
        Chip8.cpp
        Chip8.h main.cpp)
