#include <gtest/gtest.h>
#include <iidm/LccConverterStation.h>
#include <iidm/HvdcLine.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class LccConverterStationTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle LCC_H  = 10;
    static constexpr ObjectHandle TERM_H = 20;
    static constexpr ObjectHandle HVDC_H = 30;

    void SetUp() override {
        backend.strings[{LCC_H, prop::ID}]           = "LCC1";
        backend.strings[{LCC_H, prop::NAME}]          = "LCC Station 1";
        backend.doubles[{LCC_H, prop::LCC_LOSS_FACTOR}]  = 0.02f;
        backend.doubles[{LCC_H, prop::LCC_POWER_FACTOR}] = 0.95f;
        backend.related[{LCC_H, prop::REL_TERMINAL}]    = TERM_H;
        backend.bools  [{TERM_H, prop::TERMINAL_CONNECTED}] = true;
        backend.related[{LCC_H, prop::REL_HVDC_LINE}] = HVDC_H;
    }
};

TEST_F(LccConverterStationTest, GetId) {
    LccConverterStation lcc(LCC_H, &backend);
    EXPECT_EQ(lcc.getId(), "LCC1");
}

TEST_F(LccConverterStationTest, GetLossAndPowerFactor) {
    LccConverterStation lcc(LCC_H, &backend);
    // The underlying powsybl-iidm-api stores loss/power factor as float, so the
    // value carries float precision even though the C++ API exposes double.
    EXPECT_FLOAT_EQ(static_cast<float>(lcc.getLossFactor()),  0.02f);
    EXPECT_FLOAT_EQ(static_cast<float>(lcc.getPowerFactor()), 0.95f);
}

TEST_F(LccConverterStationTest, GetTerminal) {
    backend.doubles[{TERM_H, prop::TERMINAL_P}] = 100.0;
    LccConverterStation lcc(LCC_H, &backend);
    EXPECT_TRUE(lcc.getTerminal().isValid());
}

TEST_F(LccConverterStationTest, GetHvdcLine) {
    backend.strings[{HVDC_H, prop::ID}] = "HVDC1";
    LccConverterStation lcc(LCC_H, &backend);
    EXPECT_TRUE(lcc.getHvdcLine().isValid());
}

TEST_F(LccConverterStationTest, ConnectDisconnect) {
    LccConverterStation lcc(LCC_H, &backend);
    EXPECT_TRUE(lcc.isConnected());
    lcc.disconnect();
    EXPECT_FALSE((backend.bools[{TERM_H, prop::TERMINAL_CONNECTED}]));
    lcc.connect();
    EXPECT_TRUE((backend.bools[{TERM_H, prop::TERMINAL_CONNECTED}]));
}

TEST_F(LccConverterStationTest, IsValid) {
    LccConverterStation valid(LCC_H, &backend);
    LccConverterStation invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
