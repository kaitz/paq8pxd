cmake_minimum_required(VERSION 3.10)
project(paq8pxd)

find_package(ZLIB)
find_package(BZip2)

# Collect preflate sources (excluding main.cpp and main2.cpp which are standalone executables)
file(GLOB PREFLATE_SOURCES 
    "preflate/preflate_*.cpp"
    "preflate/support/*.cpp"
)

# Collect paq8pxd sources from all subdirectories
file(GLOB PRT_SOURCES "prt/*.cpp" "prt/wrt/*.cpp")
file(GLOB FILTER_SOURCES "filters/*.cpp")
file(GLOB PREDICTOR_SOURCES "predictors/*.cpp")
file(GLOB ANALYZER_SOURCES "analyzer/*.cpp" "analyzer/parsers/*.cpp")
file(GLOB STREAM_SOURCES "stream/*.cpp")
file(GLOB MODEL_SOURCES "models/*.cpp")
file(GLOB ZLIB_SOURCES "zlib/*.c")
file(GLOB BZIP2_SOURCES "bzip2/*.c")


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

if (DISABLE_SM)
    add_definitions(-DDISABLE_SM)
endif (DISABLE_SM)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Use portable optimization flags (avoid GCC-only flags like -floop-strip-mine, -fgcse-sm)
set(CMAKE_CXX_FLAGS "-O3 -funroll-loops -ftree-vectorize")

add_executable(paq8pxd 
    paq8pxd.cpp 
    ${PRT_SOURCES}
    ${FILTER_SOURCES}
    ${PREDICTOR_SOURCES}
    ${ANALYZER_SOURCES}
    ${STREAM_SOURCES}
    ${MODEL_SOURCES}
    ${ZLIB_SOURCES}
    ${BZIP2_SOURCES}
    ${PREFLATE_SOURCES}
)

# Include preflate headers
target_include_directories(paq8pxd PRIVATE ${CMAKE_SOURCE_DIR})

if (UNIX)
    if (MT)
        target_link_libraries(paq8pxd ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} pthread)
    else ()
        target_link_libraries(paq8pxd ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})
    endif (MT)
else ()
    target_link_libraries(paq8pxd ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})
endif (UNIX)