cmake_minimum_required(VERSION 4.0)
project(PETI_TO_P2CE C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Compiling on native Linux using GCC, crosscompiling on Linux for Windows using MINGW, native Windows using MSVC, and native Windows using Clang.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Compiling for Linux using GCC!")
    message(STATUS "Adding GCC compile flags.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Ofast")
elseif (MINGW)
    message(STATUS "Compiling for Windows using MingW Clang!")
    message(STATUS "Adding MingW Clang compile flags.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -static -Wall -g -Wunicode -mwindows")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static -Wall -g -Ofast -Wunicode -mwindows")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Compiling for Windows using MSVC!")
    message(STATUS "Adding Windows MSVC compile flags.")
    add_compile_options(/W4 /GF /utf-8 $<$<CONFIG:Debug>:/Zi>)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Compiling for Windows using Clang or some other compiler!")
    message(STATUS "Adding Clang compile flags.")
    add_compile_options(/W4 /GF /utf-8 $<$<CONFIG:Debug>:/Zi>)
endif ()

include_directories(.)

set(PROJECT_SRC
    main.cpp
    process_man.cpp
    logger.cpp
)

add_executable(vbsp ${PROJECT_SRC})
add_executable(vvis ${PROJECT_SRC})
add_executable(vrad ${PROJECT_SRC})

# Need to use this Python script to append the PyInstaller bytes at the end of the EXE so whenever BEEMod exports it doesnt replace these compilers.
find_package(Python3 REQUIRED)

add_custom_command(
        TARGET vbsp POST_BUILD
        COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/append_bytes.py "$<TARGET_FILE:vbsp>"
)

add_custom_command(
        TARGET vrad POST_BUILD
        COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/append_bytes.py "$<TARGET_FILE:vrad>"
)
