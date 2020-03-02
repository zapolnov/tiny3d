
set(src_macos
    Renderer/Metal/MetalPipelineState.h
    Renderer/Metal/MetalPipelineState.mm
    Renderer/Metal/MetalRenderBuffer.h
    Renderer/Metal/MetalRenderBuffer.mm
    Renderer/Metal/MetalRenderDevice.h
    Renderer/Metal/MetalRenderDevice.mm
    Renderer/Metal/MetalShaderProgram.h
    Renderer/Metal/MetalShaderProgram.mm
    Renderer/Metal/MetalTexture.h
    Renderer/Metal/MetalTexture.mm
    )

set(src_vulkan
    Renderer/Vulkan/VulkanPipelineState.h
    Renderer/Vulkan/VulkanPipelineState.cpp
    Renderer/Vulkan/VulkanRenderBuffer.h
    Renderer/Vulkan/VulkanRenderBuffer.cpp
    Renderer/Vulkan/VulkanRenderDevice.h
    Renderer/Vulkan/VulkanRenderDevice.cpp
    Renderer/Vulkan/VulkanShaderProgram.h
    Renderer/Vulkan/VulkanShaderProgram.cpp
    Renderer/Vulkan/VulkanTexture.h
    Renderer/Vulkan/VulkanTexture.cpp
    )

add(STATIC_LIBRARY
        engine
    LINK_LIBRARIES
        glm
    SOURCES
        ${src_macos}
        ${src_vulkan}
        Core/Engine.cpp
        Core/Engine.h
        Core/IGame.h
        Input/InputManager.cpp
        Input/InputManager.h
        Input/Key.h
        Math/Camera.cpp
        Math/Camera.h
        Math/PerspectiveCamera.cpp
        Math/PerspectiveCamera.h
        Mesh/AnimatedMesh.cpp
        Mesh/AnimatedMesh.h
        Mesh/Material.cpp
        Mesh/Material.h
        Mesh/MeshData.h
        Mesh/StaticMesh.cpp
        Mesh/StaticMesh.h
        Renderer/IPipelineState.h
        Renderer/IRenderBuffer.h
        Renderer/IRenderDevice.h
        Renderer/IShaderProgram.h
        Renderer/ITexture.h
        Renderer/ShaderCode.h
        Renderer/VertexFormat.h
        ResMgr/ResourceManager.cpp
        ResMgr/ResourceManager.h
        ResMgr/Shader.h
        ResMgr/Texture.h
    )

if(APPLE)
    find_library(LIBOBJC objc)
    find_library(APPKIT AppKit)
    find_library(METAL Metal)
    find_library(METALKIT MetalKit)
    find_library(CARBON Carbon)
    target_link_libraries(engine PUBLIC ${LIBOBJC} ${APPKIT} ${METALKIT} ${METAL} ${CARBON})
    set_source_files_properties(${src_vulkan} PROPERTIES HEADER_FILE_ONLY TRUE)
else()
    set_source_files_properties(${src_macos} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
