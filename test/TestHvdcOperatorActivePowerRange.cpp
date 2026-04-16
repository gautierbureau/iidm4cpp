#include <gtest/gtest.h>
#include <iidm/HvdcOperatorActivePowerRange.h>
#include <iidm/HvdcLine.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class HvdcOperatorActivePowerRangeTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle HVDC_HANDLE = 30;

    void SetUp() override {
        backend.bools  [{HVDC_HANDLE, prop::EXT_HOAR_EXISTS}]          = true;
        backend.doubles[{HVDC_HANDLE, prop::EXT_HOAR_OPR_CS1_TO_CS2}] = 400.0;
        backend.doubles[{HVDC_HANDLE, prop::EXT_HOAR_OPR_CS2_TO_CS1}] = 350.0;
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
    EXPECT_DOUBLE_EQ((backend.doubles[{HVDC_HANDLE, prop::EXT_HOAR_OPR_CS1_TO_CS2}]), 500.0);
}

TEST_F(HvdcOperatorActivePowerRangeTest, GetOprFromCS2toCS1) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(hvdc.getHvdcOperatorActivePowerRange().getOprFromCS2toCS1(), 350.0);
}

TEST_F(HvdcOperatorActivePowerRangeTest, SetOprFromCS2toCS1) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getHvdcOperatorActivePowerRange().setOprFromCS2toCS1(300.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{HVDC_HANDLE, prop::EXT_HOAR_OPR_CS2_TO_CS1}]), 300.0);
}

TEST_F(HvdcOperatorActivePowerRangeTest, MethodChaining) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getHvdcOperatorActivePowerRange().setOprFromCS1toCS2(600.0).setOprFromCS2toCS1(550.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{HVDC_HANDLE, prop::EXT_HOAR_OPR_CS1_TO_CS2}]), 600.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{HVDC_HANDLE, prop::EXT_HOAR_OPR_CS2_TO_CS1}]), 550.0);
}

TEST_F(HvdcOperatorActivePowerRangeTest, IsValid) {
    HvdcOperatorActivePowerRange valid(HVDC_HANDLE, &backend);
    HvdcOperatorActivePowerRange invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(HvdcOperatorActivePowerRangeTest, Equality) {
    HvdcOperatorActivePowerRange ext1(HVDC_HANDLE, &backend);
    HvdcOperatorActivePowerRange ext2(HVDC_HANDLE, &backend);
    HvdcOperatorActivePowerRange ext3(99, &backend);
    EXPECT_EQ(ext1, ext2);
    EXPECT_NE(ext1, ext3);
}
