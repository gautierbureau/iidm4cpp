add_executable(example_embedded DynawoEmbedded.cpp)
target_link_libraries(example_embedded PRIVATE iidmbridge)
target_include_directories(example_embedded PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${JNI_INCLUDE_DIRS}
)

# Shared library for Java JNI invocation (loaded via System.loadLibrary)
add_library(example_embedded_jni SHARED DynawoEmbedded.cpp)
set_target_properties(example_embedded_jni PROPERTIES OUTPUT_NAME "example_embedded")
target_link_libraries(example_embedded_jni PRIVATE iidmbridge)
target_include_directories(example_embedded_jni PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${JNI_INCLUDE_DIRS}
)
