cmake_minimum_required(VERSION 3.26)
project(KalmannFilter)

set(CMAKE_CXX_STANDARD 14)

include_directories(../../../2023-2024.2/DA/KalmannFilters/src)

add_executable(DA_TSP_Proj2
        ../../../2023-2024.2/DA/KalmannFilters/src/Application.cpp
        ../../../2023-2024.2/DA/KalmannFilters/src/Application.h
        ../../../2023-2024.2/DA/KalmannFilters/src/Graph.h
        ../../../2023-2024.2/DA/KalmannFilters/src/main.cpp
        ../../../2023-2024.2/DA/KalmannFilters/src/MutablePriorityQueue.h
        src/Parser.h
)

