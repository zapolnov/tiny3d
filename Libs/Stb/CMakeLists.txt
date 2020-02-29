
project(stb)

add(STATIC_LIBRARY
        stb_image
    FOLDER
        "libs"
    PUBLIC_INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES
        stb_image.c
        stb_image.h
    )
