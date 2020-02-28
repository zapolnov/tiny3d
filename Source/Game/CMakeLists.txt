
add(STATIC_LIBRARY
        game
    LINK_LIBRARIES
        engine
        resources
    SOURCES
        Game.cpp
        Game.h
        Level.cpp
        Level.h
    )
