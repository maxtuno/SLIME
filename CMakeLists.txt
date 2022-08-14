cmake_minimum_required(VERSION 2.8.12)
project(SLIME)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

include_directories(. include)

find_package(MPI REQUIRED)
if (MPI_FOUND)
    add_executable(slime_cloud
            src/Ls.cc
            src/Main.cc
            src/Options.cc
            src/SimpSolver.cc
            src/Solver.cc
            break_id/saucy.cpp
            break_id/global.cpp
            break_id/Graph.cpp
            break_id/Theory.cpp
            break_id/Breaking.cpp
            break_id/Algebraic.cpp
            )
    set(CMAKE_CXX_COMPILER "mpicxx")
    target_compile_options(slime_cloud PRIVATE -std=c++14 -O3 -D MASSIVE -DLOG)
endif (MPI_FOUND)

add_executable(slime
        src/Ls.cc
        src/Main.cc
        src/Options.cc
        src/SimpSolver.cc
        src/Solver.cc
        break_id/saucy.cpp
        break_id/global.cpp
        break_id/Graph.cpp
        break_id/Theory.cpp
        break_id/Breaking.cpp
        break_id/Algebraic.cpp
        )
target_compile_options(slime PUBLIC -std=c++14 -O3 -DLOG -static)