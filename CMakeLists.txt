cmake_minimum_required(VERSION 2.8.12)
project(SLIME)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

include_directories(. include)

option(ENABLE_MPI "Enable MPI parallelization" OFF)

set(CMAKE_CXX_STANDARD 14)

if(ENABLE_MPI)
    find_package(MPI REQUIRED)
    include_directories(${MPI_INCLUDE_PATH})
    set(CMAKE_C_FLAGS "${CMAKE_FLAGS} ${MPI_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_EXE_LINKER_FLAGS}")
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
    target_compile_options(slime_cloud PRIVATE -D MASSIVE -DLOG)
endif(ENABLE_MPI)

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
target_compile_options(slime PUBLIC -DLOG -static)