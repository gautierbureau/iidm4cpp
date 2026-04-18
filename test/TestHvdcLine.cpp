#include <gtest/gtest.h>
#include <iidm/HvdcLine.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class HvdcLineTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle HVDC_HANDLE = 70;

    void SetUp() override {
        backend.strings[{HVDC_HANDLE, prop::ID}]   = "HVDC1";
        backend.strings[{HVDC_HANDLE, prop::NAME}]  = "HVDC Line 1";
        backend.doubles[{HVDC_HANDLE, prop::HVDC_R}]                    = 1.0;
        backend.doubles[{HVDC_HANDLE, prop::HVDC_NOMINAL_V}]            = 320.0;
        backend.doubles[{HVDC_HANDLE, prop::HVDC_ACTIVE_POWER_SETPOINT}] = 500.0;
        backend.doubles[{HVDC_HANDLE, prop::HVDC_MAX_P}]                = 800.0;
        backend.ints   [{HVDC_HANDLE, prop::HVDC_CONVERTERS_MODE}]      = 0;
    }
};

TEST_F(HvdcLineTest, GetId) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(hvdc.getId(), "HVDC1");
}

TEST_F(HvdcLineTest, GetConverterStation1Id) {
    backend.strings[{HVDC_HANDLE, prop::HVDC_CONVERTER_STATION1_ID}] = "VSC1";
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(hvdc.getConverterStation1Id(), "VSC1");
}

TEST_F(HvdcLineTest, GetConverterStation2Id) {
    backend.strings[{HVDC_HANDLE, prop::HVDC_CONVERTER_STATION2_ID}] = "VSC2";
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(hvdc.getConverterStation2Id(), "VSC2");
}

TEST_F(HvdcLineTest, GetConverterStationIdsEmpty) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(hvdc.getConverterStation1Id(), "");
    EXPECT_EQ(hvdc.getConverterStation2Id(), "");
}
