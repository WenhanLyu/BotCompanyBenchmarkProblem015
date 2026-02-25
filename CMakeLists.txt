cmake_minimum_required(VERSION 3.10)
project(FileStorage)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Collect all source files
file(GLOB_RECURSE SOURCES "*.cpp")

# If no source files found yet, create a placeholder target
if(NOT SOURCES)
    message(WARNING "No .cpp files found. Build system is ready but waiting for source files.")
    # Create a dummy target that will be replaced when source files are added
    file(WRITE ${CMAKE_BINARY_DIR}/placeholder.cpp "int main() { return 0; }")
    set(SOURCES ${CMAKE_BINARY_DIR}/placeholder.cpp)
endif()

# Create executable named 'code'
add_executable(code ${SOURCES})
