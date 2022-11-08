cmake_minimum_required(VERSION 2.8.12)
project(SLIME)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

include_directories(
        .
        include
        )

set(CMAKE_CXX_FLAGS "-std=c++11 -D LOG -static")
add_executable(slime
        src/Ls.cc
        src/Main.cc
        src/Options.cc
        src/SimpSolver.cc
        src/Solver.cc
        )

# find_package(MPI REQUIRED)
# if (MPI_FOUND)
#     set(CMAKE_CXX_COMPILER mpicxx)
#     set(CMAKE_CXX_FLAGS "-std=c++11 -D LOG -D MASSIVE")
#     add_executable(slime
#             src/Ls.cc
#             src/Main.cc
#             src/Options.cc
#             src/SimpSolver.cc
#             src/Solver.cc
#             )
# endif (MPI_FOUND)