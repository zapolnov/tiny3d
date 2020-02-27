
project(glm)

file(GLOB_RECURSE source_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" glm/*)

add(HEADER_ONLY_LIBRARY
        glm
    PUBLIC_INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES
        ${source_files}
    )
