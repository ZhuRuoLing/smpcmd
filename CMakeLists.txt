cmake_minimum_required(VERSION 3.23)
project(smpcmd)

set(CMAKE_CXX_STANDARD 20)

add_executable(smpcmd smpcmd.cpp smpcmd.h library.cpp)
#add_executable(test library.cpp)
