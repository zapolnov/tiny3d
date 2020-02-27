
project(bzip2 C)
cmake_policy(SET CMP0063 NEW) # Honor visibility for all target types

file(GLOB root_files *.c *.h)
source_group("Source Files" FILES ${root_files})

add_library(libbzip2 STATIC EXCLUDE_FROM_ALL
    blocksort.c
    bzlib.c
    bzlib.h
    bzlib_private.h
    compress.c
    crctable.c
    decompress.c
    huffman.c
    randtable.c
)

set_target_properties(libbzip2 PROPERTIES OUTPUT_NAME "bzip2")
target_include_directories(libbzip2 BEFORE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
