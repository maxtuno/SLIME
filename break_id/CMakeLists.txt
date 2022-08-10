cmake_minimum_required(VERSION 3.14)
project(break_id)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-mtune=generic -static")

add_executable(break_id
        Theory.cpp
        saucy.cpp
        Graph.cpp
        global.cpp
        Breaking.cpp
        BreakID.hh
        Algebraic.cpp
        )