
cmake_minimum_required(VERSION 3.16)
project(Game)

include(CMake/Common.cmake)

set_directory_properties(PROPERTIES
    VS_STARTUP_PROJECT "Game")

include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    )

add_subdirectory(Libs)
add_subdirectory(Resources)
add_subdirectory(Source)
