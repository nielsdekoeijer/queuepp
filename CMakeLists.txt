cmake_minimum_required(VERSION 3.16)
project(queuepp_bench CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(Threads REQUIRED)

add_executable(queue_bench src/bench.cpp)

target_include_directories(queue_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(queue_bench PRIVATE Threads::Threads)
target_compile_options(queue_bench PRIVATE -O3 -march=native -Wall -Wextra)
