
project(assimp)

set(GIT_BRANCH "master")
set(GIT_COMMIT_HASH 0)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/revision.h.in" "${CMAKE_CURRENT_BINARY_DIR}/revision.h")

include_directories(code/BoostWorkaround)

add_definitions(
    -DASSIMP_BUILD_BOOST_WORKAROUND
    -DASSIMP_BUILD_NO_OWN_ZLIB
    -DASSIMP_BUILD_NO_EXPORT
    )

file(GLOB source_files
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    code/*.cpp
    code/*.h
    code/BoostWorkaround/boost/math/*.hpp
    code/BoostWorkaround/boost/tuple/*.hpp
    code/BoostWorkaround/boost/*.hpp
    contrib/ConvertUTF/*.c contrib/ConvertUTF/*.h
    contrib/clipper/*.cpp contrib/clipper/*.hpp
    contrib/irrXML/*.cpp contrib/irrXML/*.h
    contrib/poly2tri/poly2tri/common/*.cc
    contrib/poly2tri/poly2tri/common/*.h
    contrib/poly2tri/poly2tri/sweep/*.cc
    contrib/poly2tri/poly2tri/sweep/*.h
    contrib/poly2tri/poly2tri/*.h
)

file(GLOB code_inl_files code/*.inl)
set_source_files_properties(${code_inl_files} PROPERTIES HEADER_FILE_ONLY TRUE)

add(STATIC_LIBRARY
        assimp
    LINK_LIBRARIES
        libunzip
        zlib
    PUBLIC_INCLUDE_DIRS
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    SOURCES
        ${source_files}
        ${code_inl_files}
    )
