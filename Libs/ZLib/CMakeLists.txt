
project(zlib C)
cmake_policy(SET CMP0063 NEW) # Honor visibility for all target types

file(GLOB source_files *.c *.h)
source_group("Source Files" FILES ${source_files})

add_library(zlib STATIC ${source_files})
set_target_properties(zlib PROPERTIES OUTPUT_NAME "z" FOLDER "libs")
target_include_directories(zlib BEFORE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

include(CheckIncludeFile)
check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    target_compile_definitions(zlib PUBLIC "Z_HAVE_UNISTD_H")
endif()

include(CheckTypeSize)
set(CMAKE_REQUIRED_DEFINITIONS "-D_LARGEFILE64_SOURCE=1")
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
    target_compile_definitions(zlib PUBLIC "_LARGEFILE64_SOURCE=1")
endif()
