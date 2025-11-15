add_library(sortlib sort.cpp)

target_compile_definitions(sortlib PRIVATE $<$<CONFIG:Debug>:DEBUG>)

target_link_libraries(sortlib PUBLIC parlay)
target_include_directories(sortlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})