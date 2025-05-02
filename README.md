# KalmanFilters

Real-time bus tracking with Kalman filtering.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Dependencies](#dependencies)
- [Build Instructions](#build-instructions)
- [Usage](#usage)
- [Documentation](#documentation)
- [License](#license)

## Overview
This project implements a real-time bus tracking system using Kalman filtering. It features a robust Kalman filter implementation and a JSON data fetcher for retrieving vehicle data.

## Features
- **Kalman filter** for real-time tracking.
- **JSON data fetching** using `libcurl`.
- **Modular design** with reusable components.
- **Doxygen-generated documentation** for easy reference.

## Project Structure
```
├── JsonFetcher/         # JSON data fetching logic
├── src/                 # Core application and Kalman filter implementation
├── docs/                # Doxygen-generated documentation
├── build/               # Build artifacts
├── cmake-build-debug/   # Debug build artifacts
├── Dataset/             # Dataset files (e.g., Porto dataset)
├── CMakeLists.txt       # CMake build configuration
├── Doxyfile             # Doxygen configuration
└── README.md            # Project README
```

## Dependencies
- **C++14** or later
- [Eigen3](https://eigen.tuxfamily.org/) (for matrix operations)
- [libcurl](https://curl.se/libcurl/) (for JSON data fetching)
- [CMake](https://cmake.org/) (minimum version 3.26)

## Build Instructions
1. Clone the repository:
    ```bash
    git clone <repository-url>
    cd KalmanFilters
    ```

2. Create a build directory and configure the project:
    ```bash
    mkdir build
    cd build
    cmake ..
    ```

3. Build the project:
    ```bash
    make
    ```

4. Run the executables:
    ```bash
    ./KalmanFilter
    ./JsonDataFetcher
    ```

## Documentation
The project uses **Doxygen** for documentation. To generate the documentation:

1. Install Doxygen.
2. Run the following command in the project root:
    ```bash
    doxygen Doxyfile
    ```

## License
This project is licensed under the **MIT License**. See the [LICENSE](./LICENSE) file for details.