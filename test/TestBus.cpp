#include <gtest/gtest.h>
#include <iidm/Bus.h>
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

TEST_F(BusTest, GetId) {
    Bus bus(BUS_HANDLE, &backend);
    EXPECT_EQ(bus.getId(), "BUS1");
}

TEST_F(BusTest, GetVAngle) {
    Bus bus(BUS_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(bus.getV(), 1.03);
    EXPECT_DOUBLE_EQ(bus.getAngle(), -0.12);
}

TEST_F(BusTest, SetVAngle) {
    Bus bus(BUS_HANDLE, &backend);
    bus.setV(1.05).setAngle(0.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{BUS_HANDLE, prop::BUS_V}]), 1.05);
    EXPECT_DOUBLE_EQ((backend.doubles[{BUS_HANDLE, prop::BUS_ANGLE}]), 0.0);
}

TEST_F(BusTest, IsValid) {
    Bus valid(BUS_HANDLE, &backend);
    Bus invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
