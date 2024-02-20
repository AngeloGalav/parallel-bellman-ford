cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_REQUIRED ON)

project(bellmanford-omp C)

file(GLOB_RECURSE SRC_FILES src/*.c*)
file(GLOB_RECURSE HEADER_FILES include/*)

find_package(OpenMP COMPONENTS C REQUIRED)

add_executable(bellmanford-omp ${SRC_FILES})

target_include_directories(bellmanford-omp
PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(bellmanford-omp
PUBLIC
OpenMP::OpenMP_C)
