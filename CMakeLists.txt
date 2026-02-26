cmake_minimum_required(VERSION 3.10)
project(FileStorage)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_RELEASE "")

# Explicitly list source files to avoid CMake-generated files
set(SOURCES
    main.cpp
    bucket_manager.cpp
)

# Create executable named 'code'
add_executable(code ${SOURCES})
