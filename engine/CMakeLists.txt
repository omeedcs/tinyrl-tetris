cmake_minimum_required(VERSION 3.18)
project(TetrisEngine LANGUAGES CXX)

cmake_minimum_required(VERSION 3.10)
project(TetrisTinyRL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Source files
set(SOURCES
    main.cpp
    timeManager.cpp
    tetrisGame.cpp
    renderer.cpp
    input.cpp
)

# Executable
add_executable(tetris ${SOURCES})

# Include directories
target_include_directories(tetris PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
