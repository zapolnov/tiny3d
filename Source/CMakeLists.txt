
if(APPLE)
    add_definitions(-fobjc-arc -stdlib=libc++)
endif()

add_subdirectory(Engine)
add_subdirectory(Game)
add_subdirectory(Importer)
add_subdirectory(Platform)
