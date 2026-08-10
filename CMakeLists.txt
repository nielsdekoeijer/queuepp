cmake_minimum_required(VERSION 3.16)
project(queuepp_bench CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(Threads REQUIRED)

add_executable(bench src/bench.cpp)
add_library(symbols src/symbols.cpp)

target_include_directories(bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(bench PRIVATE Threads::Threads)
target_compile_options(bench PRIVATE -O3 -march=native -Wall -Wextra)

target_include_directories(symbols PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(symbols PRIVATE Threads::Threads)
target_compile_options(symbols PRIVATE -O3 -march=native -Wall -Wextra -g)
