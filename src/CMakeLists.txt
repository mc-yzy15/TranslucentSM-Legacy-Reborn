cmake_minimum_required(VERSION 3.10)
project(TranslucentSM)

set(CMAKE_CXX_STANDARD 17)

# 确保所有源文件和头文件都被正确包含
include_directories(${CMAKE_SOURCE_DIR}/TranslucentSM)

add_executable(TranslucentSM main.cpp TranslucentSM.cpp TranslucentSM.h)