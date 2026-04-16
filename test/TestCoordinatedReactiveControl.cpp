#include <gtest/gtest.h>
#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/Generator.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class CoordinatedReactiveControlTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle GEN_HANDLE = 10;

    void SetUp() override {
        backend.bools  [{GEN_HANDLE, prop::EXT_CRC_EXISTS}]    = true;
        backend.doubles[{GEN_HANDLE, prop::EXT_CRC_Q_PERCENT}] = 50.0;
    }
};

TEST_F(CoordinatedReactiveControlTest, HasCoordinatedReactiveControl) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_TRUE(gen.hasCoordinatedReactiveControl());
}

TEST_F(CoordinatedReactiveControlTest, HasCoordinatedReactiveControlAbsent) {
    MockBackend empty;
    Generator gen(GEN_HANDLE, &empty);
    EXPECT_FALSE(gen.hasCoordinatedReactiveControl());
}

TEST_F(CoordinatedReactiveControlTest, GetQPercent) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(gen.getCoordinatedReactiveControl().getQPercent(), 50.0);
}

TEST_F(CoordinatedReactiveControlTest, SetQPercent) {
    Generator gen(GEN_HANDLE, &backend);
    gen.getCoordinatedReactiveControl().setQPercent(75.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::EXT_CRC_Q_PERCENT}]), 75.0);
}

TEST_F(CoordinatedReactiveControlTest, MethodChaining) {
    Generator gen(GEN_HANDLE, &backend);
    auto& crc = gen.getCoordinatedReactiveControl().setQPercent(30.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::EXT_CRC_Q_PERCENT}]), 30.0);
    (void)crc;
}

TEST_F(CoordinatedReactiveControlTest, IsValid) {
    CoordinatedReactiveControl valid(GEN_HANDLE, &backend);
    CoordinatedReactiveControl invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(CoordinatedReactiveControlTest, Equality) {
    CoordinatedReactiveControl crc1(GEN_HANDLE, &backend);
    CoordinatedReactiveControl crc2(GEN_HANDLE, &backend);
    CoordinatedReactiveControl crc3(99, &backend);
    EXPECT_EQ(crc1, crc2);
    EXPECT_NE(crc1, crc3);
}
