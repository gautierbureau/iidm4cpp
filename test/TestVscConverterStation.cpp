#include <gtest/gtest.h>
#include <iidm/VscConverterStation.h>
#include <iidm/HvdcLine.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class VscConverterStationTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle VSC_H  = 10;
    static constexpr ObjectHandle TERM_H = 20;
    static constexpr ObjectHandle HVDC_H = 30;
    static constexpr ObjectHandle LIMITS_H = 40;

    void SetUp() override {
        backend.strings[{VSC_H, prop::ID}]                     = "VSC1";
        backend.strings[{VSC_H, prop::NAME}]                    = "VSC Station 1";
        backend.doubles[{VSC_H, prop::CONVERTER_STATION_LOSS_FACTOR}]         = 0.011f;
        backend.related[{VSC_H, prop::REACTIVE_LIMITS}]         = LIMITS_H;
        backend.related[{VSC_H, prop::MIN_MAX_REACTIVE_LIMITS}] = LIMITS_H;
        backend.doubles[{LIMITS_H, prop::RL_MIN_Q}]             = -200.0;
        backend.doubles[{LIMITS_H, prop::RL_MAX_Q}]             = 200.0;
        backend.bools  [{LIMITS_H, prop::MIN_MAX_REACTIVE_LIMITS}] = true;
        backend.related[{VSC_H, prop::REL_TERMINAL}]            = TERM_H;
        backend.related[{VSC_H, prop::REL_HVDC_LINE}]           = HVDC_H;
        backend.related[{VSC_H, prop::REL_REGULATING_TERMINAL}] = TERM_H;
    }
};

TEST_F(VscConverterStationTest, MinMaxReactiveLimits) {
    VscConverterStation vsc(VSC_H, &backend);
    ReactiveLimits limits = vsc.getReactiveLimits();
    EXPECT_TRUE(limits.is<MinMaxReactiveLimits>());
    EXPECT_FALSE(limits.is<ReactiveCapabilityCurve>());
    EXPECT_DOUBLE_EQ(limits.as<MinMaxReactiveLimits>().getMinQ(), -200.0);
    EXPECT_DOUBLE_EQ(vsc.getReactiveLimits<MinMaxReactiveLimits>().getMaxQ(), 200.0);
}

TEST_F(VscConverterStationTest, GetTerminalAndHvdcLine) {
    backend.doubles[{TERM_H, prop::TERMINAL_P}] = 100.0;
    backend.strings[{HVDC_H, prop::ID}] = "HVDC1";
    VscConverterStation vsc(VSC_H, &backend);
    EXPECT_TRUE(vsc.getTerminal().isValid());
    EXPECT_TRUE(static_cast<bool>(vsc.getHvdcLine()));
}

