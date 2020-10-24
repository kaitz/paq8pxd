cmake_minimum_required(VERSION 3.10)
project(paq8pxd)

find_package(ZLIB)

option(UNIX "Whether to build for Unix. Otherwise, build for Windows" OFF)
option(NATIVECPU "Whether to build for your cpu (vs. the general public)" OFF)
option(MT "Whether to enable Multithreading" OFF)
option(DISABLE_SM "Whether to disable faster statemaps" OFF)

if (UNIX)
    add_definitions(-DUNIX)
else ()
    add_definitions(-DWINDOWS)
endif (UNIX)

if (NATIVECPU)
    add_definitions(-march=native -mtune=native)
else ()
    add_definitions(-march=nocona -mtune=generic)
endif (NATIVECPU)

if (MT)
    add_definitions(-DMT)
endif (MT)
if (MT)
    add_definitions(-DMT)
endif (MT)

if (DISABLE_SM)
    add_definitions(-DDISABLE_SM)
endif (DISABLE_SM)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-O3 -floop-strip-mine -funroll-loops -ftree-vectorize -fgcse-sm -falign-loops=16")

add_executable(paq8pxd paq8pxd.cpp)

target_link_libraries(paq8pxd ${ZLIB_LIBRARIES})
