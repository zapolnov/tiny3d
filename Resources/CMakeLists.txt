
file(GLOB_RECURSE levels RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS Levels/*)
file(GLOB_RECURSE shaders RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS Shaders/*)

set(gen
    Compiled/Levels.cpp
    Compiled/Levels.h
    Compiled/Shaders.cpp
    Compiled/Shaders.h
    Compiled/Shaders_Metal.cpp
    Compiled/Shaders_Metal.h
    )

add(STATIC_LIBRARY
        resources
    PUBLIC_INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
    LINK_LIBRARIES
        glm
    SOURCES
        ${gen}
        ${levels}
        ${shaders}
        assets.xml
    )

set(first TRUE)
set(output)
set(byproducts)
foreach(f ${gen})
    if(first)
        set(first FALSE)
        set(output "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
    else()
        list(APPEND byproducts "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
    endif()
endforeach()

add_custom_command(OUTPUT "${output}"
        BYPRODUCTS ${byproducts}
        COMMAND cmake -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/Compiled"
        COMMAND cmake -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/Temp"
        COMMAND importer
        MAIN_DEPENDENCY assets.xml
        DEPENDS ${levels} assets.xml importer
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Compiling assets"
    )
