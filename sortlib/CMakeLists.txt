add_library(sortlib sort.cpp)

target_link_libraries(sortlib PRIVATE parlay)
target_include_directories(sortlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})