
cmake_minimum_required(VERSION 3.16)
project(Game)

include(CMake/Common.cmake)

set_directory_properties(PROPERTIES
    VS_STARTUP_PROJECT "Game")

add_subdirectory(Libs)
