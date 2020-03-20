cmake_minimum_required(VERSION 2.8)
project(SLIME)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

include_directories(. include)

set(CMAKE_CXX_COMPILER mpicxx)
set(CMAKE_CXX_FLAGS "-std=c++98 -Os -D NDEBUG -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -D LOG -D MASSIVE")

add_executable(slime
        src/Main.cc
        src/SimpSolver.cc
        src/Solver.cc
        )

add_library(ipasirslimeglue
        src/SimpSolver.cc
        src/Solver.cc
        src/ipasirslimeglue.cc
        )