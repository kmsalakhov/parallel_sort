cmake_minimum_required(VERSION 4.1)
project(parallel_sort)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third_party/googletest)
add_subdirectory(third_party/parlay)
add_subdirectory(third_party/benchmark)
add_subdirectory(sortlib)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} sortlib)

enable_testing()

add_executable(test_sort test/test_sort.cpp)

target_link_libraries(test_sort PRIVATE
    sortlib
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_sort)

add_executable(sort_benchmark benchmark/sort_benchmark.cpp)
target_link_libraries(sort_benchmark PRIVATE sortlib parlay benchmark::benchmark)