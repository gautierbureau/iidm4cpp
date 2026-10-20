#include <gtest/gtest.h>
#include <iidm/HvdcOperatorActivePowerRange.h>
#include <iidm/HvdcOperatorActivePowerRangeAdder.h>
#include <iidm/HvdcLine.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class HvdcOperatorActivePowerRangeTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle HVDC_HANDLE = 30;
    static constexpr ObjectHandle EXT_HANDLE = 31; // the extension object

    void SetUp() override {
        backend.related[{HVDC_HANDLE, prop::HVDC_OPERATOR_ACTIVE_POWER_RANGE}] = EXT_HANDLE;
        backend.doubles[{EXT_HANDLE, prop::EXT_HOAR_OPR_CS1_TO_CS2}] = 400.0;
        backend.doubles[{EXT_HANDLE, prop::EXT_HOAR_OPR_CS2_TO_CS1}] = 350.0;
    }
};

TEST_F(HvdcOperatorActivePowerRangeTest, HasExtension) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_TRUE(hvdc.hasHvdcOperatorActivePowerRange());
}

TEST_F(HvdcOperatorActivePowerRangeTest, HasExtensionAbsent) {
    MockBackend empty;
    HvdcLine hvdc(HVDC_HANDLE, &empty);
    EXPECT_FALSE(hvdc.hasHvdcOperatorActivePowerRange());
}

TEST_F(HvdcOperatorActivePowerRangeTest, GetOprFromCS1toCS2) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(hvdc.getHvdcOperatorActivePowerRange().getOprFromCS1toCS2(), 400.0);
}

TEST_F(HvdcOperatorActivePowerRangeTest, SetOprFromCS1toCS2) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getHvdcOperatorActivePowerRange().setOprFromCS1toCS2(500.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_HOAR_OPR_CS1_TO_CS2}]), 500.0);
}

TEST_F(HvdcOperatorActivePowerRangeTest, GetOprFromCS2toCS1) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(hvdc.getHvdcOperatorActivePowerRange().getOprFromCS2toCS1(), 350.0);
}

TEST_F(HvdcOperatorActivePowerRangeTest, SetOprFromCS2toCS1) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getHvdcOperatorActivePowerRange().setOprFromCS2toCS1(300.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_HOAR_OPR_CS2_TO_CS1}]), 300.0);
}

TEST_F(HvdcOperatorActivePowerRangeTest, MethodChaining) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getHvdcOperatorActivePowerRange().setOprFromCS1toCS2(600.0).setOprFromCS2toCS1(550.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_HOAR_OPR_CS1_TO_CS2}]), 600.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_HOAR_OPR_CS2_TO_CS1}]), 550.0);
}

TEST_F(HvdcOperatorActivePowerRangeTest, IsValid) {
    HvdcOperatorActivePowerRange valid(EXT_HANDLE, &backend);
    HvdcOperatorActivePowerRange invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(HvdcOperatorActivePowerRangeTest, Equality) {
    HvdcOperatorActivePowerRange ext1(EXT_HANDLE, &backend);
    HvdcOperatorActivePowerRange ext2(EXT_HANDLE, &backend);
    HvdcOperatorActivePowerRange ext3(99, &backend);
    EXPECT_EQ(ext1, ext2);
    EXPECT_NE(ext1, ext3);
}

TEST_F(HvdcOperatorActivePowerRangeTest, AbsentThrows) {
    MockBackend empty;
    EXPECT_THROW(HvdcLine(HVDC_HANDLE, &empty).getHvdcOperatorActivePowerRange(), ExtensionNotFoundException);
}

TEST_F(HvdcOperatorActivePowerRangeTest, NewAndRemove) {
    MockBackend b;
    static constexpr ObjectHandle NEW_EXT = 32;
    HvdcLine hvdc(HVDC_HANDLE, &b);
    HvdcOperatorActivePowerRangeAdder adder = hvdc.newHvdcOperatorActivePowerRange();
    EXPECT_EQ(b.invocations.back(), std::make_pair(static_cast<ObjectHandle>(HVDC_HANDLE), prop::HVDC_OPERATOR_ACTIVE_POWER_RANGE));
    const ObjectHandle adderHandle = b.lastAdder;
    b.addResults[adderHandle] = NEW_EXT;
    HvdcOperatorActivePowerRange ext = adder.withOprFromCS1toCS2(300.0).withOprFromCS2toCS1(250.0).add();
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::EXT_HOAR_OPR_CS1_TO_CS2}]), 300.0);
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::EXT_HOAR_OPR_CS2_TO_CS1}]), 250.0);
    EXPECT_EQ(ext, HvdcOperatorActivePowerRange(NEW_EXT, &b));

    hvdc.removeHvdcOperatorActivePowerRange();
    EXPECT_EQ((b.related[{HVDC_HANDLE, prop::HVDC_OPERATOR_ACTIVE_POWER_RANGE}]), INVALID_HANDLE);
}
