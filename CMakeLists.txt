cmake_minimum_required(VERSION 4.0)
project(TS_PETI CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Compiling native Linux, crosscompiling on Linux for Windows, and native Windows.
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Compiling for Linux!")
    message(STATUS "Adding Linux compile flags.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Ofast")
elseif (MINGW)
    # Tell MingW that to include defines for Windows and to support Windows functions
    message(STATUS "Compiling for Windows using MingW Clang!")
    message(STATUS "Adding Windows MingW Clang compile flags and definitions.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -static -Wall -O0 -g -Wunicode -mwindows")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static -Wall -Ofast -Wunicode -mwindows")
    add_compile_definitions(_WIN32=1 _WINDOWS=1 NOTIFY_RENDER_OUTSIDE_MAIN_WINDOW=1)
endif ()

include_directories(.)

add_executable(TS_PETI main.cpp)
