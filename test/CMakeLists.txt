# GTest is found or fetched by the parent CMakeLists.txt.

set(TEST_SOURCES
    TestNetwork.cpp
    TestBattery.cpp
    TestReactiveLimits.cpp
    TestTapChangerObjects.cpp
    TestThreeWindingsTransformerLeg.cpp
    TestThreeWindingsTransformer.cpp
    TestTopologyViews.cpp
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
    TestTwoWindingsTransformer.cpp
    TestTwoWindingsTransformerTapChanger.cpp
    TestBusbarSection.cpp
    TestConnect.cpp
    TestBackendSelector.cpp
    TestIntegration.cpp
    TestShuntCompensator.cpp
    TestShuntCompensatorNonLinear.cpp
    TestStaticVarCompensator.cpp
    TestLccConverterStation.cpp
    TestVscConverterStation.cpp
    TestHvdcLine.cpp
    TestDanglingLine.cpp
    TestSubstation.cpp
    TestCurrentLimits.cpp
    TestEnums.cpp
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

# ── Coverage ──────────────────────────────────────────────────────────────────
if(IIDM_BRIDGE_ENABLE_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "Coverage requires GCC or Clang; IIDM_BRIDGE_ENABLE_COVERAGE ignored.")
    else()
        target_compile_options(iidmbridge      PRIVATE --coverage)
        target_link_options   (iidmbridge      PRIVATE --coverage)
        target_compile_options(iidmbridge_tests PRIVATE --coverage)
        target_link_options   (iidmbridge_tests PRIVATE --coverage)

        find_program(GCOVR_EXECUTABLE gcovr)
        if(GCOVR_EXECUTABLE)
            add_custom_target(coverage
                COMMAND ${GCOVR_EXECUTABLE}
                    --root ${CMAKE_SOURCE_DIR}/src
                    --object-directory ${CMAKE_BINARY_DIR}
                    --exclude "${CMAKE_SOURCE_DIR}/src/jni/.*"
                    --exclude "${CMAKE_SOURCE_DIR}/src/graalvm/.*"
                    --html-details ${CMAKE_BINARY_DIR}/coverage/index.html
                    --print-summary
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating HTML coverage report in ${CMAKE_BINARY_DIR}/coverage/"
                DEPENDS iidmbridge_tests
            )
        else()
            message(STATUS "gcovr not found; 'coverage' target unavailable. Install gcovr to generate HTML reports.")
        endif()
    endif()
endif()

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
