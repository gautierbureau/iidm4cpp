#include <gtest/gtest.h>
#include <iidm/Bus.h>
#include <iidm/VoltageLevel.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class BusTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle BUS_HANDLE = 30;

    void SetUp() override {
        backend.strings[{BUS_HANDLE, prop::ID}]    = "BUS1";
        backend.strings[{BUS_HANDLE, prop::NAME}]   = "Bus 1";
        backend.doubles[{BUS_HANDLE, prop::BUS_V}]    = 1.03;
        backend.doubles[{BUS_HANDLE, prop::BUS_ANGLE}] = -0.12;
    }
};

TEST_F(BusTest, GetVoltageLevel) {
    static constexpr ObjectHandle VL_H = 50;
    backend.related[{BUS_HANDLE, prop::REL_VOLTAGE_LEVEL}] = VL_H;
    backend.strings[{VL_H, prop::ID}]          = "VL1";
    backend.doubles[{VL_H, prop::VL_NOMINAL_V}] = 225.0;
    Bus bus(BUS_HANDLE, &backend);
    VoltageLevel vl = bus.getVoltageLevel();
    EXPECT_TRUE(vl.isValid());
    EXPECT_EQ(vl.getId(), "VL1");
}

