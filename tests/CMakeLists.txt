cmake_minimum_required(VERSION 3.10)
project(TetrisTinyRLTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Catch2
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

# Include engine headers
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine
)

# Engine source files (excluding main.cpp)
set(ENGINE_SOURCES
    ../engine/timeManager.cpp
    ../engine/tetrisGame.cpp
    ../engine/renderer.cpp
    ../engine/input.cpp
)

# Test executable
add_executable(tetris_tests
    engine/test_tetris_game.cpp
    ${ENGINE_SOURCES}
)

# Link Catch2
target_link_libraries(tetris_tests PRIVATE Catch2::Catch2WithMain)

# Enable testing
enable_testing()
add_test(NAME TetrisTests COMMAND tetris_tests)

# Discover tests for CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tetris_tests)
