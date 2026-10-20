#include <gtest/gtest.h>
#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/CoordinatedReactiveControlAdder.h>
#include <iidm/Generator.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class CoordinatedReactiveControlTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle GEN_HANDLE = 10;
    static constexpr ObjectHandle CRC_HANDLE = 11; // the extension object

    void SetUp() override {
        backend.related[{GEN_HANDLE, prop::COORDINATED_REACTIVE_CONTROL}] = CRC_HANDLE;
        backend.doubles[{CRC_HANDLE, prop::EXT_CRC_Q_PERCENT}] = 50.0;
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
    EXPECT_DOUBLE_EQ((backend.doubles[{CRC_HANDLE, prop::EXT_CRC_Q_PERCENT}]), 75.0);
}

TEST_F(CoordinatedReactiveControlTest, MethodChaining) {
    Generator gen(GEN_HANDLE, &backend);
    CoordinatedReactiveControl crc = gen.getCoordinatedReactiveControl();
    EXPECT_EQ(&crc.setQPercent(30.0), &crc);
    EXPECT_DOUBLE_EQ((backend.doubles[{CRC_HANDLE, prop::EXT_CRC_Q_PERCENT}]), 30.0);
}

TEST_F(CoordinatedReactiveControlTest, IsValid) {
    CoordinatedReactiveControl valid(CRC_HANDLE, &backend);
    CoordinatedReactiveControl invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(CoordinatedReactiveControlTest, Equality) {
    CoordinatedReactiveControl crc1(CRC_HANDLE, &backend);
    CoordinatedReactiveControl crc2(CRC_HANDLE, &backend);
    CoordinatedReactiveControl crc3(99, &backend);
    EXPECT_EQ(crc1, crc2);
    EXPECT_NE(crc1, crc3);
}

TEST_F(CoordinatedReactiveControlTest, AbsentThrows) {
    MockBackend empty;
    EXPECT_THROW(Generator(GEN_HANDLE, &empty).getCoordinatedReactiveControl(), ExtensionNotFoundException);
}

TEST_F(CoordinatedReactiveControlTest, NewAndRemove) {
    MockBackend b;
    static constexpr ObjectHandle NEW_CRC = 12;
    Generator gen(GEN_HANDLE, &b);
    CoordinatedReactiveControlAdder adder = gen.newCoordinatedReactiveControl();
    EXPECT_EQ(b.invocations.back(), std::make_pair(static_cast<ObjectHandle>(GEN_HANDLE), prop::COORDINATED_REACTIVE_CONTROL));
    const ObjectHandle adderHandle = b.lastAdder;
    b.addResults[adderHandle] = NEW_CRC;
    CoordinatedReactiveControl crc = adder.withQPercent(40.0).add();
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::EXT_CRC_Q_PERCENT}]), 40.0);
    EXPECT_EQ(b.invocations.back(), std::make_pair(adderHandle, prop::ACT_ADD));
    EXPECT_EQ(crc, CoordinatedReactiveControl(NEW_CRC, &b));

    gen.removeCoordinatedReactiveControl();
    EXPECT_EQ((b.related[{GEN_HANDLE, prop::COORDINATED_REACTIVE_CONTROL}]), INVALID_HANDLE);
}
