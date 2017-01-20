cmake_minimum_required(VERSION 3.0)
project(RandomMicroBench CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(ComparisonStructures comparison_structures.cpp)

add_executable(StdAsyncThreadCreation std_async_thread_creation.cpp)
target_link_libraries(StdAsyncThreadCreation ${CMAKE_THREAD_LIBS_INIT})

add_executable(StdVectorCapacityResize std_vector_capacity_resize.cpp)

add_executable(MemCpyBenchmark memcpy_benchmark.cpp)

add_executable(FileIOBenchmark file_io_benchmark.cpp)

add_executable(MemCpyStackOverflow memcpy_stackoverflow.cpp)
