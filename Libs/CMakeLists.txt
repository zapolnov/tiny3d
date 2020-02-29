
cmake_minimum_required(VERSION 3.2)

if(MSVC)
    add_definitions(/wd4047 /wd4244 /wd4267 /wd4305 /wd4456 /wd4458)
else()
    add_definitions(-w)
endif()

add_subdirectory(Assimp)
add_subdirectory(BZip2)
add_subdirectory(GLM)
add_subdirectory(MiniZip)
add_subdirectory(Stb)
add_subdirectory(TinyXML)
add_subdirectory(ZLib)
