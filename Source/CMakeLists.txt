
include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

if(APPLE)
    add_definitions(-fobjc-arc)
endif()

add_subdirectory(Engine)
add_subdirectory(Game)
