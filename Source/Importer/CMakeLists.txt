
add(EXECUTABLE
        importer
    CONSOLE
    LINK_LIBRARIES
        tinyxml
        engine
    SOURCES
        ConfigFile.cpp
        ConfigFile.h
        LevelMeshBuilder.cpp
        LevelMeshBuilder.h
        LevelProcessor.cpp
        LevelProcessor.h
        Util.cpp
        Util.h
        main.cpp
    )
