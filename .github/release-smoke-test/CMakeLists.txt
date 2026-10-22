# A consumer of an iidm-bridge release package, built by the release workflow against the
# extracted package (CMAKE_PREFIX_PATH) with the package's C++ standard (CMAKE_CXX_STANDARD).
cmake_minimum_required(VERSION 3.14)
project(iidm_bridge_smoke_test CXX)

find_package(IidmBridge REQUIRED)
find_package(JNI REQUIRED)

add_executable(smoke_test main.cpp)
target_include_directories(smoke_test PRIVATE ${JNI_INCLUDE_DIRS})
# main.cpp starts the JVM: libjvm only (JNI_LIBRARIES also holds libjawt, which needs X11)
target_link_libraries(smoke_test PRIVATE IidmBridge::iidmbridge ${JAVA_JVM_LIBRARY})
