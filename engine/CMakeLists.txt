cmake_minimum_required(VERSION 3.18)
project(TetrisEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    main.cpp
    timeManager.cpp
)

# Executable
add_executable(tetris ${SOURCES})

# Include directories
target_include_directories(tetris PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
