cmake_minimum_required(VERSION 3.15)
project(AudioAnalyzer)

set(SOURCES src/main.cpp)

add_executable(AudioAnalyzer 
    src/main.cpp 
    src/header.cpp
    src/header_helper.cpp
)
