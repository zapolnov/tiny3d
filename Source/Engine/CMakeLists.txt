
set(src_macos
    Renderer/Metal/MetalRenderBuffer.h
    Renderer/Metal/MetalRenderBuffer.mm
    Renderer/Metal/MetalRenderDevice.h
    Renderer/Metal/MetalRenderDevice.mm
    )

add(STATIC_LIBRARY
        engine
    LINK_LIBRARIES
        glm
    SOURCES
        ${src_macos}
        Core/Engine.cpp
        Core/Engine.h
        Core/IGame.h
        Renderer/IRenderBuffer.h
        Renderer/IRenderDevice.h
    )

if(APPLE)
    find_library(LIBOBJC objc)
    find_library(APPKIT AppKit)
    find_library(METAL Metal)
    find_library(METALKIT MetalKit)
    target_link_libraries(engine PUBLIC ${LIBOBJC} ${APPKIT} ${METALKIT} ${METAL})
else()
    set_source_files_properties(${src_macos} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
