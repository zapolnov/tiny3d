set(SOURCES ossource.cpp ../osinclude.h)

add_library(OSDependent STATIC ${SOURCES})
set_property(TARGET OSDependent PROPERTY FOLDER "libs/glslang")
set_property(TARGET OSDependent PROPERTY POSITION_INDEPENDENT_CODE ON)

# MinGW GCC complains about function pointer casts to void*.
# Turn that off with -fpermissive.
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(OSDependent PRIVATE -fpermissive)
endif()

if(WIN32)
    source_group("Source" FILES ${SOURCES})
endif(WIN32)

if(ENABLE_GLSLANG_INSTALL)
    install(TARGETS OSDependent EXPORT OSDependentTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(EXPORT OSDependentTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)
endif(ENABLE_GLSLANG_INSTALL)
