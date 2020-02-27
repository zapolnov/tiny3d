
set(src_macos
    Platform/MacOS/AppDelegate.h
    Platform/MacOS/AppDelegate.mm
    Platform/MacOS/ViewController.h
    Platform/MacOS/ViewController.mm
    Platform/MacOS/main.mm
    )

add(STATIC_LIBRARY
        engine
    SOURCES
        ${src_macos}
    )

if(APPLE)
    find_library(LIBOBJC objc)
    find_library(APPKIT AppKit)
    find_library(METAL Metal)
    find_library(METALKIT MetalKit)
    target_link_libraries(engine ${LIBOBJC} ${APPKIT} ${METALKIT} ${METAL})
else()
    set_source_files_properties(${src_macos} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
