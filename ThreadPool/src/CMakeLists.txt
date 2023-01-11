cmake_minimum_required(VERSION 3.24)
project(src)

set(CMAKE_CXX_STANDARD 20)

add_executable(src thread_pool.h thread_pool.cpp)
