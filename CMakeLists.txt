cmake_minimum_required(VERSION 3.10)
project(gen_interface)

set(CMAKE_CXX_STANDARD 17)

add_executable(gen_interface main.cpp)
add_executable(test test.cpp)