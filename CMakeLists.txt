cmake_minimum_required(VERSION 3.0)
project(RandomMicroBench CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED iostreams)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(ComparisonStructures comparison_structures.cpp)

add_executable(StdAsyncThreadCreation std_async_thread_creation.cpp)
target_link_libraries(StdAsyncThreadCreation pthread)

add_executable(StdVectorCapacityResize std_vector_capacity_resize.cpp)

add_executable(io_microbench io_microbench.cpp)
target_link_libraries(io_microbench ${Boost_LIBRARIES})