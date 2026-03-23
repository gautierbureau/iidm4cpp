add_executable(example_standalone main.cpp)
target_link_libraries(example_standalone PRIVATE iidmbridge)
target_include_directories(example_standalone PRIVATE ${CMAKE_SOURCE_DIR}/include)
