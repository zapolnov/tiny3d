
add(EXECUTABLE
        importer
    CONSOLE
    FOLDER
        "tools"
    LINK_LIBRARIES
        tinyxml
        engine
    SOURCES
        ConfigFile.cpp
        ConfigFile.h
        LevelProcessor.cpp
        LevelProcessor.h
        Util.cpp
        Util.h
        main.cpp
    )
