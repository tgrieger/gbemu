cmake_minimum_required(VERSION 3.31)
project(gbemu)

set(CMAKE_CXX_STANDARD 20)

add_executable(gbemu main.cpp
        register.h
        opcode.h)
