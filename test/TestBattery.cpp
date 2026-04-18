#include <gtest/gtest.h>
#include <iidm/Battery.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class BatteryTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle BAT_HANDLE      = 30;
    static constexpr ObjectHandle TERM_HANDLE     = 31;

    void SetUp() override {
        backend.strings[{BAT_HANDLE, prop::ID}]    = "BAT1";
        backend.strings[{BAT_HANDLE, prop::NAME}]   = "Battery 1";
        backend.doubles[{BAT_HANDLE, prop::BAT_TARGET_P}] = 50.0;
        backend.doubles[{BAT_HANDLE, prop::BAT_TARGET_Q}] = 10.0;
        backend.doubles[{BAT_HANDLE, prop::BAT_MIN_P}]    = -100.0;
        backend.doubles[{BAT_HANDLE, prop::BAT_MAX_P}]    = 100.0;
        backend.related[{BAT_HANDLE, prop::REL_TERMINAL}] = TERM_HANDLE;
        backend.bools  [{TERM_HANDLE, prop::TERMINAL_CONNECTED}] = true;
    }
};

TEST_F(BatteryTest, GetId) {
    Battery bat(BAT_HANDLE, &backend);
    EXPECT_EQ(bat.getId(), "BAT1");
}

TEST_F(BatteryTest, GetName) {
    Battery bat(BAT_HANDLE, &backend);
    EXPECT_EQ(bat.getName(), "Battery 1");
}

TEST_F(BatteryTest, TargetPGetSet) {
    Battery bat(BAT_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(bat.getTargetP(), 50.0);
    bat.setTargetP(60.0);
    double tp = backend.doubles[{BAT_HANDLE, prop::BAT_TARGET_P}];
    EXPECT_DOUBLE_EQ(tp, 60.0);
}

TEST_F(BatteryTest, TargetQGetSet) {
    Battery bat(BAT_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(bat.getTargetQ(), 10.0);
    bat.setTargetQ(15.0);
    double tq = backend.doubles[{BAT_HANDLE, prop::BAT_TARGET_Q}];
    EXPECT_DOUBLE_EQ(tq, 15.0);
}

TEST_F(BatteryTest, MinMaxP) {
    Battery bat(BAT_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(bat.getMinP(), -100.0);
    EXPECT_DOUBLE_EQ(bat.getMaxP(),  100.0);
}

TEST_F(BatteryTest, IsConnected) {
    Battery bat(BAT_HANDLE, &backend);
    EXPECT_TRUE(bat.isConnected());
}

TEST_F(BatteryTest, IsValid) {
    Battery valid(BAT_HANDLE, &backend);
    Battery invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(BatteryTest, Equality) {
    Battery a(BAT_HANDLE, &backend);
    Battery b(BAT_HANDLE, &backend);
    Battery c(99, &backend);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}
