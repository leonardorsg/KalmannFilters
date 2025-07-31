cmake_minimum_required(VERSION 3.26)
project(KalmannFilter)

# Set the C++ standard to C++17 (needed for structured bindings)
set(CMAKE_CXX_STANDARD 17)

# Find required packages
# find_package(Eigen3 CONFIG REQUIRED)  # Commented out - using local Eigen
# find_package(CURL REQUIRED)  # Commented out - using manual curl setup

# Add Eigen (local installation)
include_directories(${CMAKE_SOURCE_DIR}/extern/eigen)

# Add nlohmann/json (local installation)
include_directories(${CMAKE_SOURCE_DIR}/extern/json/single_include)

# Add CURL (MSYS2 installation)
include_directories("C:/msys64/ucrt64/include")
link_directories("C:/msys64/ucrt64/lib")

# Add JsonFetcher directories (for common includes in the project)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/GTFS)
include_directories(${CMAKE_SOURCE_DIR}/src/services)

# Main KalmannFilter executable
add_executable(KalmannFilter
        ${CMAKE_SOURCE_DIR}/src/Application.cpp
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/services/Parser.h
        ${CMAKE_SOURCE_DIR}/src/GTFS/agency.h
        src/GTFS/calendar.h
        src/GTFS/route.h
        src/GTFS/stops.h
        src/GTFS/trips.h
        src/RealTime/vehicle.h
        src/services/Utils.h
        src/services/KalmannFilter.h
        src/services/KalmannFilter.cpp
)

# JsonDataFetcher executable
add_executable(JsonDataFetcher
        ${CMAKE_SOURCE_DIR}/JsonFetcher/JsonDataFetcher.cpp
)

# Link libraries
# target_link_libraries(KalmannFilter PRIVATE Eigen3::Eigen)  # Eigen is header-only, no linking needed
target_link_libraries(JsonDataFetcher PRIVATE curl)