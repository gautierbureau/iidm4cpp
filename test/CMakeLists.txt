# GTest is found or fetched by the parent CMakeLists.txt.

set(TEST_SOURCES
    TestNetwork.cpp
    TestGenerator.cpp
    TestActivePowerControl.cpp
    TestLoad.cpp
    TestTerminal.cpp
    TestBus.cpp
    TestLine.cpp
    TestConnect.cpp
    TestBackendSelector.cpp
    TestIntegration.cpp
)

add_executable(iidmbridge_tests ${TEST_SOURCES})

target_include_directories(iidmbridge_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(iidmbridge_tests
    PRIVATE
        iidmbridge
        GTest::GTest
        GTest::Main
)

include(GoogleTest)
# PRE_TEST defers test discovery to ctest time instead of running the
# executable immediately after the build.  On Windows this is required
# because iidmbridge.dll must be on PATH, which is only set when ctest runs.
gtest_discover_tests(iidmbridge_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DISCOVERY_MODE PRE_TEST
)
