
add(EXECUTABLE
        launcher
    LINK_LIBRARIES
        engine
        game
    SOURCES
        Vulkan.cpp
        Vulkan.h
        Win32.h
        main.cpp
    )
