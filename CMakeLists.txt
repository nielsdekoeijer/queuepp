cmake_minimum_required(VERSION 3.16)
project(queuepp_bench C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(Threads REQUIRED)

add_executable(bench src/bench.cpp)
add_library(symbols_c src/symbols_c.c)
add_library(symbols_cpp src/symbols_cpp.cpp)

target_include_directories(bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(bench PRIVATE Threads::Threads)
target_compile_options(bench PRIVATE -O3 -Wall -Wextra -mcpu=cortex-a53 -march=armv8-a+crc+crypto)

target_include_directories(symbols_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(symbols_c PRIVATE -O3 -Wall -Wextra -g -mcpu=cortex-a53 -march=armv8-a+crc+crypto)

target_include_directories(symbols_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(symbols_cpp PRIVATE Threads::Threads)
target_compile_options(symbols_cpp PRIVATE -O3 -Wall -Wextra -g -mcpu=cortex-a53 -march=armv8-a+crc+crypto)
