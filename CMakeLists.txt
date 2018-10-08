cmake_minimum_required(VERSION 3.11)
project(clion_lispy)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_BUILD_TYPE Debug)

add_executable(clion_lispy main.cpp interpreter.h)