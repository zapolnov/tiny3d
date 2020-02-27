
project(tinyxml)
cmake_policy(SET CMP0063 NEW) # Honor visibility for all target types

file(GLOB files *.cpp *.h)
source_group("Source Files" FILES ${files})

add_library(tinyxml STATIC ${files})
target_include_directories(tinyxml PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
