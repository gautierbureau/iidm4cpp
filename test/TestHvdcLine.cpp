#include <gtest/gtest.h>
#include <iidm/HvdcLine.h>
#include <iidm/VscConverterStation.h>
#include <iidm/LccConverterStation.h>
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

// ── Converter station getHvdcLine ─────────────────────────────────────────────

TEST_F(HvdcLineTest, VscGetHvdcLine) {
    static constexpr ObjectHandle VSC_H = 80;
    backend.strings[{VSC_H, prop::ID}] = "VSC1";
    backend.related[{VSC_H, prop::REL_HVDC_LINE}] = HVDC_HANDLE;
    VscConverterStation vsc(VSC_H, &backend);
    HvdcLine line = vsc.getHvdcLine();
    EXPECT_TRUE(line.isValid());
    EXPECT_EQ(line.getId(), "HVDC1");
}

TEST_F(HvdcLineTest, LccGetHvdcLine) {
    static constexpr ObjectHandle LCC_H = 81;
    backend.strings[{LCC_H, prop::ID}] = "LCC1";
    backend.related[{LCC_H, prop::REL_HVDC_LINE}] = HVDC_HANDLE;
    LccConverterStation lcc(LCC_H, &backend);
    HvdcLine line = lcc.getHvdcLine();
    EXPECT_TRUE(line.isValid());
    EXPECT_EQ(line.getId(), "HVDC1");
}

TEST_F(HvdcLineTest, GetName) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(hvdc.getName(), "HVDC Line 1");
}

TEST_F(HvdcLineTest, GetRNominalVMaxP) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(hvdc.getR(), 1.0);
    EXPECT_DOUBLE_EQ(hvdc.getNominalV(), 320.0);
    EXPECT_DOUBLE_EQ(hvdc.getMaxP(), 800.0);
}

TEST_F(HvdcLineTest, GetSetActivePowerSetpoint) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(hvdc.getActivePowerSetpoint(), 500.0);
    hvdc.setActivePowerSetpoint(600.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{HVDC_HANDLE, prop::HVDC_ACTIVE_POWER_SETPOINT}]), 600.0);
}

TEST_F(HvdcLineTest, GetSetConvertersMode) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(hvdc.getConvertersMode(), HvdcConverterStationMode::RECTIFIER);
    hvdc.setConvertersMode(HvdcConverterStationMode::INVERTER);
    EXPECT_EQ((backend.ints[{HVDC_HANDLE, prop::HVDC_CONVERTERS_MODE}]), 1);
}

TEST_F(HvdcLineTest, IsValid) {
    HvdcLine valid(HVDC_HANDLE, &backend);
    HvdcLine invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
