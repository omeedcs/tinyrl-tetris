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

# Find SDL2 and SDL2_ttf
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Find Python (before pybind11)
set(PYBIND11_FINDPYTHON ON)
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

# find pybind11
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../extern/pybind11 ${CMAKE_CURRENT_BINARY_DIR}/pybind11)

# Common source files (without loop dependencies)
set(COMMON_SOURCES
    timeManager.cpp
)

# Create a separate tetrisGame for SDL (without loop function)
add_library(tetris_game_lib OBJECT
    tetrisGame.cpp
)

target_compile_definitions(tetris_game_lib PRIVATE NO_TERMINAL_LOOP)

target_include_directories(tetris_game_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# SDL version (GUI)
add_executable(tetris_sdl
    main_sdl.cpp
    sdl_renderer.cpp
    ${COMMON_SOURCES}
    $<TARGET_OBJECTS:tetris_game_lib>
)

target_compile_definitions(tetris_sdl PRIVATE NO_TERMINAL_LOOP)

target_include_directories(tetris_sdl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(tetris_sdl
    ${SDL2_LIBRARIES}
    SDL2_ttf::SDL2_ttf
)

# Terminal version (legacy) - disabled, needs loop() function
# add_executable(tetris_terminal
#     main.cpp
#     renderer.cpp
#     input.cpp
#     ${COMMON_SOURCES}
# )
#
# target_include_directories(tetris_terminal PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# Python bindings
pybind11_add_module(tinyrl_tetris
    bindings.cpp
    tetrisGame.cpp
    ${COMMON_SOURCES}
)

target_compile_definitions(tinyrl_tetris PRIVATE NO_TERMINAL_LOOP)

target_include_directories(tinyrl_tetris PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
