cmake_minimum_required(VERSION 3.26)
project(KalmannFilter)

# Set the C++ standard to C++14
set(CMAKE_CXX_STANDARD 14)

# Find Eigen3 package
find_package(Eigen3 REQUIRED)

# Add JsonFetcher directories (for common includes in the project)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/GTFS)

# Include Eigen headers
include_directories(${EIGEN3_INCLUDE_DIRS})

# Define the main source files
add_executable(KalmannFilter
        ${CMAKE_SOURCE_DIR}/src/Application.cpp
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/MutablePriorityQueue.h
        ${CMAKE_SOURCE_DIR}/src/Parser.h
        ${CMAKE_SOURCE_DIR}/src/GTFS/agency.h
        src/GTFS/calendar.h
        src/GTFS/route.h
        src/GTFS/stops.h
        src/GTFS/trips.h
        src/RealTime/vehicle.h
        src/Utils.h
        src/KalmannFilter.h
        src/KalmannFilter.cpp
)