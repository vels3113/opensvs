cmake_minimum_required(VERSION 3.18)
project(opensvs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT (ENABLE_ASAN OR ENABLE_UBSAN))
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Default Qt6 CMake prefix for Ubuntu 22.04; can be overridden via CLI/env if different.
if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "/usr/lib/x86_64-linux-gnu/cmake/Qt6")
  list(APPEND CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt6")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
