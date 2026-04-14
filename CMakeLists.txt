cmake_minimum_required(VERSION 3.20)
project(iidm-bridge-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(IIDM_BRIDGE_ENABLE_GRAALVM "Build the GraalVM backend" ON)
option(IIDM_BRIDGE_ENABLE_JNI     "Build the JNI backend" ON)
option(IIDM_BRIDGE_BUILD_TESTS    "Build unit tests" ON)
option(IIDM_BRIDGE_BUILD_EXAMPLES "Build examples" ON)

# ── JNI (always needed for AUTO mode probe) ──────────────────────────────────
find_package(JNI REQUIRED)

# ── GraalVM ──────────────────────────────────────────────────────────────────
if(IIDM_BRIDGE_ENABLE_GRAALVM)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    find_package(GraalVM)
    if(NOT GraalVM_FOUND)
        message(WARNING "GraalVM not found; GraalVM backend will compile without SDK headers. "
                        "Set GRAALVM_HOME to enable full GraalVM support.")
    endif()
endif()

# ── Library sources ──────────────────────────────────────────────────────────
set(IIDM_BRIDGE_SOURCES
    src/Network.cpp
    src/NetworkFactory.cpp
    src/BackendSelector.cpp
    src/Extension.cpp
    src/Generator.cpp
    src/Load.cpp
    src/Terminal.cpp
    src/Bus.cpp
    src/Substation.cpp
    src/VoltageLevel.cpp
    src/Line.cpp
    src/TwoWindingsTransformer.cpp
    src/ThreeWindingsTransformer.cpp
    src/HvdcLine.cpp
    src/DanglingLine.cpp
    src/ShuntCompensator.cpp
    src/StaticVarCompensator.cpp
    src/VscConverterStation.cpp
    src/LccConverterStation.cpp
)

if(IIDM_BRIDGE_ENABLE_GRAALVM)
    list(APPEND IIDM_BRIDGE_SOURCES
        src/graalvm/GraalVMBackend.cpp)
endif()

if(IIDM_BRIDGE_ENABLE_JNI)
    list(APPEND IIDM_BRIDGE_SOURCES
        src/jni/JNIBackend.cpp)
endif()

add_library(iidmbridge SHARED ${IIDM_BRIDGE_SOURCES})

target_include_directories(iidmbridge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${JNI_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
        $<$<BOOL:${IIDM_BRIDGE_ENABLE_GRAALVM}>:${GRAALVM_INCLUDE_DIRS}>
)

target_link_libraries(iidmbridge
    PRIVATE
        ${JNI_LIBRARIES}
        ${CMAKE_DL_LIBS}
)

target_compile_definitions(iidmbridge PRIVATE
    $<$<BOOL:${IIDM_BRIDGE_ENABLE_GRAALVM}>:IIDM_BRIDGE_GRAALVM_ENABLED>
    $<$<BOOL:${IIDM_BRIDGE_ENABLE_JNI}>:IIDM_BRIDGE_JNI_ENABLED>
)

# ── Installation & CMake config ───────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS iidmbridge EXPORT IidmBridgeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT IidmBridgeTargets
    FILE IidmBridgeTargets.cmake
    NAMESPACE IidmBridge::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/IidmBridge
)
configure_file(cmake/IidmBridgeConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/IidmBridgeConfig.cmake" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/IidmBridgeConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/IidmBridge
)

# ── Tests ─────────────────────────────────────────────────────────────────────
if(IIDM_BRIDGE_BUILD_TESTS)
    enable_testing()
    find_package(GTest)
    if(GTest_FOUND)
        add_subdirectory(test)
    else()
        message(WARNING "GTest not found; tests will not be built.")
    endif()
endif()

# ── Examples ──────────────────────────────────────────────────────────────────
if(IIDM_BRIDGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
