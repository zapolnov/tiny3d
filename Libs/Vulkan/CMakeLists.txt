
project(vulkan)

add(HEADER_ONLY_LIBRARY
        vulkanHeaders
    FOLDER
        "libs"
    PUBLIC_INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES
        vulkan/vk_platform.h
        vulkan/vulkan.h
    )
