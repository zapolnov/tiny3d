
add(EXECUTABLE
        launcher
    LINK_LIBRARIES
        engine
        game
    SOURCES
        AppDelegate.h
        AppDelegate.mm
        ViewController.h
        ViewController.mm
        main.mm
    )
