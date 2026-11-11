cmake_minimum_required(VERSION 3.21)

# Select the toolchain before project() initializes the compiler. Also repair
# the path produced by the README command when VCPKG_ROOT is unset.
if(NOT CMAKE_TOOLCHAIN_FILE OR
   CMAKE_TOOLCHAIN_FILE STREQUAL "/scripts/buildsystems/vcpkg.cmake")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE
            "${CMAKE_CURRENT_SOURCE_DIR}/.vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "vcpkg toolchain" FORCE)
        message(STATUS "Using project-local vcpkg: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
endif()

if(IS_ABSOLUTE "${CMAKE_TOOLCHAIN_FILE}" AND NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    message(FATAL_ERROR
        "CMAKE_TOOLCHAIN_FILE does not exist: ${CMAKE_TOOLCHAIN_FILE}. "
        "Install vcpkg and set VCPKG_ROOT to its directory (see README.md). "
        "Clear the CMake cache before configuring again.")
endif()

project(RType
    VERSION 0.1.0
    DESCRIPTION "Networked multiplayer shoot'em up built on a custom game engine"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# The engine is a standalone project; R-Type is only data (game/data), Python scripts (game/scripts) and assets.
add_subdirectory(engine)

engine_add_game(rtype
    CLIENT r-type_client
    SERVER r-type_server
    DATA game/data
    SCRIPTS game/scripts
    ASSETS game/assets)

engine_add_plugin(wobble
    SOURCES plugins/wobble/src/WobbleSystem.cpp plugins/wobble/src/WobblePlugin.cpp
    INCLUDES plugins/wobble/include)

engine_add_game(skyhop
    LOCAL skyhop
    PLUGINS wobble
    DATA game2/data
    SCRIPTS game2/scripts
    ASSETS game2/assets)
