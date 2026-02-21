cmake_minimum_required(VERSION 3.10)
project(superbuild NONE)

include(ExternalProject)

# Linux native build
ExternalProject_Add(linux_build
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/src"
    BINARY_DIR "${CMAKE_BINARY_DIR}/linux"
    CMAKE_ARGS
        # "-DCMAKE_BUILD_TYPE=Release"
        "-DPROJECT_ROOT=${CMAKE_SOURCE_DIR}"
    INSTALL_COMMAND ""
)

# Windows (MinGW) build
ExternalProject_Add(windows_build
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/src"
    BINARY_DIR "${CMAKE_BINARY_DIR}/windows"
    CMAKE_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/toolchain-mingw64.cmake"
        "-DCMAKE_BUILD_TYPE=Release"
        "-DPROJECT_ROOT=${CMAKE_SOURCE_DIR}"
    INSTALL_COMMAND ""
)

add_custom_target(all_platforms
    DEPENDS linux_build windows_build
)

add_dependencies(all_platforms linux_build windows_build)
