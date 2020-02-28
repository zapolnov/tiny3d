#################################
if(COMMON_CMAKE_INCLUDED)       #
    return()                    #
endif()                         #
set(COMMON_CMAKE_INCLUDED TRUE) #
#################################

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

get_filename_component(ROOT_PATH "${CMAKE_CURRENT_LIST_FILE}" ABSOLUTE)
get_filename_component(ROOT_PATH "${ROOT_PATH}" DIRECTORY)
get_filename_component(ROOT_PATH "${ROOT_PATH}" DIRECTORY)
include_directories("${ROOT_PATH}")

if(MSVC)
    add_definitions(
        -D_SCL_SECURE_NO_WARNINGS=1
        -D_CRT_SECURE_NO_WARNINGS=1
        -D_CRT_SECURE_NO_DEPRECATE=1
        -D_CRT_NONSTDC_NO_DEPRECATE=1
        )
elseif(APPLE)
    add_definitions(
        -Wno-unused-private-field
        )
endif()

get_property(multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT multiconfig AND (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL ""))
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type" FORCE)
    message(WARNING "CMAKE_BUILD_TYPE was not set, defaulting to '${CMAKE_BUILD_TYPE}'.")
endif()

#######################################################################################################################

macro(install_library config targetDir file)
    get_filename_component(name "${file}" NAME)
    if(EXISTS "${file}" AND NOT EXISTS "${targetDir}/${name}")
        message(STATUS "Installing dependency '${name}' (${config})")
        configure_file("${file}" "${targetDir}/${name}" COPYONLY)
    endif()
endmacro()

macro(mingw_install_phtread_library targetDir)
    if(MINGW)
        get_filename_component(gccdir "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach(lib libwinpthread-1)
            install_library("${CMAKE_BUILD_TYPE}" "${targetDir}" "${gccdir}/${lib}.dll")
        endforeach()
    endif()
endmacro()

macro(mingw_install_libraries targetDir)
    if(MINGW)
        get_filename_component(gccdir "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach(lib libgcc_s_dw2-1 libgcc_s_seh-1 libwinpthread-1 libstdc++-6)
            install_library("${CMAKE_BUILD_TYPE}" "${targetDir}" "${gccdir}/${lib}.dll")
        endforeach()
    endif()
endmacro()

macro(maybe_write_file file contents)
    set(do_write TRUE)
    if(EXISTS "${file}")
        file(READ "${file}" old)
        if("${old}" STREQUAL "${contents}")
            set(do_write FALSE)
        endif()
    endif()
    if(do_write)
        file(WRITE "${file}" "${contents}")
    endif()
endmacro()

macro(maybe_c_flag flag)
    include(CheckCCompilerFlag)
    string(MAKE_C_IDENTIFIER "HAVE_CC_FLAG${flag}" var)
    string(TOUPPER "${var}" var)
    check_c_compiler_flag("${flag}" ${var})
    if(${var})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
    endif()
endmacro()

macro(maybe_cxx_flag flag)
    include(CheckCXXCompilerFlag)
    string(MAKE_C_IDENTIFIER "HAVE_CXX_FLAG${flag}" var)
    string(TOUPPER "${var}" var)
    check_cxx_compiler_flag("${flag}" ${var})
    if(${var})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endif()
endmacro()

macro(maybe_flag flag)
    maybe_c_flag("${flag}")
    maybe_cxx_flag("${flag}")
endmacro()

macro(add type target)
    set(noValue
        "CONSOLE"
        )

    set(oneValue
        "FOLDER"
        "SOURCES_PREFIX"
        "OUTPUT_DIRECTORY"
        "OUTPUT_NAME"
        )

    set(multiValue
        "SOURCES"
        "DEPENDS"
        "LINK_LIBRARIES"
        "PRIVATE_DEFINES"
        "PUBLIC_DEFINES"
        "PRIVATE_INCLUDE_DIRS"
        "PUBLIC_INCLUDE_DIRS"
        )

    cmake_parse_arguments(ARG "${noValue}" "${oneValue}" "${multiValue}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments \"${ARG_UNPARSED_ARGUMENTS}\".")
    endif()

    if(NOT ARG_OUTPUT_NAME)
        set(ARG_OUTPUT_NAME "${target}")
    endif()

    if(NOT ARG_OUTPUT_DIRECTORY)
        set(ARG_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    endif()

    #########################################################################################################
    ## Target

    if("${type}" STREQUAL "EXECUTABLE")
        add_executable("${target}")
        if(MINGW)
            mingw_install_phtread_library("${ARG_OUTPUT_DIRECTORY}")
            target_link_libraries("${target}" PRIVATE mingw32)
        endif()
        if(NOT ARG_CONSOLE)
            set_target_properties("${target}" PROPERTIES WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)
        endif()
        if(MINGW OR CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
            set_target_properties("${target}" PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++")
        endif()
    elseif("${type}" STREQUAL "STATIC_LIBRARY")
        cmake_policy(SET CMP0063 NEW) # Honor visibility for all target types
        add_library("${target}" STATIC)
    elseif("${type}" STREQUAL "HEADER_ONLY_LIBRARY")
        if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${target}.c")
            string(MAKE_C_IDENTIFIER "${target}" identifier)
            file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${target}.c" "void headeronlylibrary_${identifier}_() {}\n")
        endif()
        source_group("Generated Files" FILES "${CMAKE_CURRENT_BINARY_DIR}/${target}.c")
        add_library("${target}" STATIC "${CMAKE_CURRENT_BINARY_DIR}/${target}.c")
    else()
        message(FATAL_ERROR "Invalid target type '${type}'.")
    endif()

    #########################################################################################################
    ## Target properties

    set_target_properties("${target}" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${ARG_OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY "${ARG_OUTPUT_DIRECTORY}"
        OUTPUT_NAME "${ARG_OUTPUT_NAME}"
        )

    if(ARG_FOLDER)
        set_target_properties("${target}" PROPERTIES
            FOLDER "${ARG_FOLDER}")
    endif()

    #########################################################################################################
    ## Dependencies

    if(ARG_DEPENDS)
        add_dependencies("${target}" ${ARG_DEPENDS})
    endif()

    #########################################################################################################
    ## Include directories

    if(ARG_PUBLIC_INCLUDE_DIRS)
        target_include_directories("${target}" PUBLIC ${ARG_PUBLIC_INCLUDE_DIRS})
    endif()

    if(ARG_PRIVATE_INCLUDE_DIRS)
        target_include_directories("${target}" PRIVATE ${ARG_PRIVATE_INCLUDE_DIRS})
    endif()

    #########################################################################################################
    ## Defines

    if(ARG_PUBLIC_DEFINES)
        target_compile_definitions("${target}" PUBLIC ${ARG_PUBLIC_DEFINES})
    endif()

    if(ARG_PRIVATE_DEFINES)
        target_compile_definitions("${target}" PRIVATE ${ARG_PRIVATE_DEFINES})
    endif()

    #########################################################################################################
    ## Link libraries

    if(ARG_LINK_LIBRARIES)
        target_link_libraries("${target}" PUBLIC ${ARG_LINK_LIBRARIES})
    endif()

    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        find_library(LIBM m)
        if(LIBM)
            target_link_libraries("${target}" PUBLIC ${LIBM})
        endif()
    endif()

    #########################################################################################################
    ## Source files

    set(src
        ${ARG_SOURCES}
        )

    foreach(srcfile ${src})
        if(NOT ARG_SOURCES_PREFIX)
            set(file "${srcfile}")
        else()
            string(REGEX REPLACE "^${ARG_SOURCES_PREFIX}" "" file "${srcfile}")
        endif()

        get_filename_component(ext "${file}" EXT)
        get_filename_component(path "${file}" DIRECTORY)

        if ("${path}" STREQUAL "")
            string(REPLACE "/" "\\" group "Source Files")
        else()
            string(REPLACE "/" "\\" group "Source Files/${path}")
        endif()
        source_group("${group}" FILES "${srcfile}")
    endforeach()

    target_sources("${target}" PRIVATE ${src})
endmacro()
