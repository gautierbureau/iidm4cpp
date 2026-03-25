find_package(GTest REQUIRED)

set(TEST_SOURCES
    TestNetwork.cpp
    TestGenerator.cpp
    TestLoad.cpp
    TestTerminal.cpp
    TestBus.cpp
    TestLine.cpp
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
gtest_discover_tests(iidmbridge_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
