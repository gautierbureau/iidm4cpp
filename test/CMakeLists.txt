# GTest is found or fetched by the parent CMakeLists.txt.

set(TEST_SOURCES
    TestNetwork.cpp
    TestBattery.cpp
    TestReactiveLimits.cpp
    TestGenerator.cpp
    TestActivePowerControl.cpp
    TestCoordinatedReactiveControl.cpp
    TestHvdcAngleDroopActivePowerControl.cpp
    TestHvdcOperatorActivePowerRange.cpp
    TestVoltagePerReactivePowerControl.cpp
    TestSlackTerminal.cpp
    TestLoad.cpp
    TestTerminal.cpp
    TestBus.cpp
    TestLine.cpp
    TestSwitch.cpp
    TestTwoWindingsTransformerTapChanger.cpp
    TestBusbarSection.cpp
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

# On Windows there is no rpath: copy iidmbridge.dll next to the test
# executable so it is found automatically (Windows searches the exe dir first).
if(WIN32)
    add_custom_command(TARGET iidmbridge_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:iidmbridge>
            $<TARGET_FILE_DIR:iidmbridge_tests>
    )
endif()

include(GoogleTest)
# PRE_TEST defers test discovery to ctest time instead of running the
# executable immediately after the build.  On Windows this is required
# because iidmbridge.dll must be on PATH, which is only set when ctest runs.
gtest_discover_tests(iidmbridge_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DISCOVERY_MODE PRE_TEST
)
