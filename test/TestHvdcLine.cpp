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

TEST_F(HvdcLineTest, ConverterStations) {
    static constexpr ObjectHandle VSC_H = 80;
    static constexpr ObjectHandle LCC_H = 81;
    backend.related[{HVDC_HANDLE, prop::REL_CONVERTER_STATION1}] = VSC_H;
    backend.related[{HVDC_HANDLE, prop::REL_CONVERTER_STATION2}] = LCC_H;
    backend.strings[{VSC_H, prop::ID}] = "VSC1";
    backend.ints[{LCC_H, prop::CONVERTER_STATION_HVDC_TYPE}] = static_cast<int>(HvdcType::LCC);
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(hvdc.getConverterStation1().getId(), "VSC1");
    EXPECT_EQ(hvdc.getConverterStation2().getHvdcType(), HvdcType::LCC);
    EXPECT_EQ(hvdc.getConverterStation2(), HvdcConverterStation(LCC_H, &backend));
}

TEST_F(HvdcLineTest, Setters) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(&hvdc.setR(2.0).setNominalV(400.0).setMaxP(900.0).setActivePowerSetpoint(600.0), &hvdc);
    EXPECT_DOUBLE_EQ(hvdc.getR(), 2.0);
    EXPECT_DOUBLE_EQ(hvdc.getNominalV(), 400.0);
    EXPECT_DOUBLE_EQ(hvdc.getMaxP(), 900.0);
    EXPECT_DOUBLE_EQ(hvdc.getActivePowerSetpoint(), 600.0);
}

TEST_F(HvdcLineTest, ConverterStationBySide) {
    static constexpr ObjectHandle CS2_H = 82;
    backend.relatedByIndex[std::make_tuple(ObjectHandle(HVDC_HANDLE), prop::HVDC_CONVERTER_STATION, 1)] = CS2_H;
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(hvdc.getConverterStation(TwoSides::TWO), HvdcConverterStation(CS2_H, &backend));
}

// Connects or disconnects both: the line comes back when this changed them
TEST_F(HvdcLineTest, ConnectConverterStations) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_TRUE(hvdc.disconnectConverterStations());
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(HVDC_HANDLE), prop::ACT_DISCONNECT));
    backend.unchanged.insert(ObjectHandle(HVDC_HANDLE));
    EXPECT_FALSE(hvdc.connectConverterStations());
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(HVDC_HANDLE), prop::ACT_CONNECT));
}

// ── Converter station getHvdcLine ─────────────────────────────────────────────

TEST_F(HvdcLineTest, VscGetHvdcLine) {
    static constexpr ObjectHandle VSC_H = 80;
    backend.strings[{VSC_H, prop::ID}] = "VSC1";
    backend.related[{VSC_H, prop::REL_HVDC_LINE}] = HVDC_HANDLE;
    VscConverterStation vsc(VSC_H, &backend);
    iidm::optional<HvdcLine> line = vsc.getHvdcLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(line->getId(), "HVDC1");
}

TEST_F(HvdcLineTest, LccGetHvdcLine) {
    static constexpr ObjectHandle LCC_H = 81;
    static constexpr ObjectHandle VSC_H = 80;
    backend.strings[{LCC_H, prop::ID}] = "LCC1";
    backend.related[{LCC_H, prop::REL_HVDC_LINE}] = HVDC_HANDLE;
    backend.related[{LCC_H, prop::REL_OTHER_CONVERTER_STATION}] = VSC_H;
    LccConverterStation lcc(LCC_H, &backend);
    iidm::optional<HvdcLine> line = lcc.getHvdcLine();
    ASSERT_TRUE(line);
    EXPECT_EQ(line->getId(), "HVDC1");
    EXPECT_EQ(*lcc.getOtherConverterStation(), HvdcConverterStation(VSC_H, &backend));
}

TEST_F(HvdcLineTest, StationWithoutLine) {
    VscConverterStation vsc(82, &backend);
    EXPECT_FALSE(vsc.getHvdcLine());
    EXPECT_FALSE(vsc.getOtherConverterStation());
}

TEST_F(HvdcLineTest, Remove) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.remove();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(HVDC_HANDLE), prop::ACT_REMOVE));
}

TEST_F(HvdcLineTest, GetName) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_EQ(hvdc.getNameOrId(), "HVDC Line 1");
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
