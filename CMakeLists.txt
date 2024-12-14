cmake_minimum_required(VERSION 3.27)
project(__c__)

set(CMAKE_CXX_STANDARD 17)

add_executable(__c__ main.cpp
        Data_base.h
        Date_base.cpp
        prepoccess.h
        prepoccess.cpp)
