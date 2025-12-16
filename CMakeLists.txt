cmake_minimum_required(VERSION 3.16)
project(c_maze CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build raylib examples" OFF)
option(BUILD_GAMES "Build raylib games" OFF)
option(BUILD_SHARED_LIBS "Build raylib as a shared library" OFF)

find_package(raylib QUIET)

if (NOT raylib_FOUND)
    include(FetchContent)
    message(STATUS "raylib not found; fetching via FetchContent")
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
    )
    FetchContent_MakeAvailable(raylib)
endif ()

add_executable(c_maze
    src/main.cpp
    src/game.cpp
    src/game.h
    src/maze_builder.cpp
    src/maze_builder.h
)
target_link_libraries(c_maze PRIVATE raylib)

# Copy assets next to the built executable so relative paths work when running from the build tree.
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
add_custom_target(
    copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(c_maze copy_assets)
