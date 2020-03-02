
add(EXECUTABLE
        launcher
    LINK_LIBRARIES
        engine
        game
    SOURCES
        Vulkan.cpp
        Win32.h
        main.cpp
    )
