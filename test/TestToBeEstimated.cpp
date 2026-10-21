#include <gtest/gtest.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/ThreeWindingsTransformerToBeEstimated.h>
#include <iidm/ThreeWindingsTransformerToBeEstimatedAdder.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/TwoWindingsTransformerToBeEstimated.h>
#include <iidm/TwoWindingsTransformerToBeEstimatedAdder.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class ToBeEstimatedTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle TWT_H       = 30;
    static constexpr ObjectHandle THREE_H     = 31;
    static constexpr ObjectHandle TWT_EXT_H   = 32; // the extension objects
    static constexpr ObjectHandle THREE_EXT_H = 33;

    void SetUp() override {
        backend.related[{TWT_H, prop::TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED}] = TWT_EXT_H;
        backend.bools[{TWT_EXT_H, prop::EXT_TWO_WT_TBE_RTC}] = true;
        backend.bools[{TWT_EXT_H, prop::EXT_TWO_WT_TBE_PTC}] = false;
        backend.related[{THREE_H, prop::THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED}] = THREE_EXT_H;
        backend.bools[{THREE_EXT_H, prop::EXT_THREE_WT_TBE_RTC2}] = true;
        backend.bools[{THREE_EXT_H, prop::EXT_THREE_WT_TBE_PTC3}] = true;
    }
};

TEST_F(ToBeEstimatedTest, ThreeWindingsTransformerGet) {
    auto ext = ThreeWindingsTransformer(THREE_H, &backend).getExtension<ThreeWindingsTransformerToBeEstimated>();
    EXPECT_FALSE(ext.shouldEstimateRatioTapChanger1());
    EXPECT_TRUE(ext.shouldEstimateRatioTapChanger2());
    EXPECT_FALSE(ext.shouldEstimateRatioTapChanger3());
    EXPECT_TRUE(ext.shouldEstimateRatioTapChanger(ThreeSides::TWO));
    EXPECT_FALSE(ext.shouldEstimatePhaseTapChanger1());
    EXPECT_FALSE(ext.shouldEstimatePhaseTapChanger2());
    EXPECT_TRUE(ext.shouldEstimatePhaseTapChanger3());
    EXPECT_TRUE(ext.shouldEstimatePhaseTapChanger(ThreeSides::THREE));
}

TEST_F(ToBeEstimatedTest, ThreeWindingsTransformerSetUsesSideCodes) {
    auto ext = ThreeWindingsTransformer(THREE_H, &backend).getExtension<ThreeWindingsTransformerToBeEstimated>();
    ext.shouldEstimateRatioTapChanger1(true)
       .shouldEstimateRatioTapChanger(false, ThreeSides::TWO)
       .shouldEstimateRatioTapChanger3(true)
       .shouldEstimatePhaseTapChanger1(true)
       .shouldEstimatePhaseTapChanger2(true)
       .shouldEstimatePhaseTapChanger(false, ThreeSides::THREE);
    EXPECT_TRUE((backend.bools[{THREE_EXT_H, prop::EXT_THREE_WT_TBE_RTC1}]));
    EXPECT_FALSE((backend.bools[{THREE_EXT_H, prop::EXT_THREE_WT_TBE_RTC2}]));
    EXPECT_TRUE((backend.bools[{THREE_EXT_H, prop::EXT_THREE_WT_TBE_RTC3}]));
    EXPECT_TRUE((backend.bools[{THREE_EXT_H, prop::EXT_THREE_WT_TBE_PTC1}]));
    EXPECT_TRUE((backend.bools[{THREE_EXT_H, prop::EXT_THREE_WT_TBE_PTC2}]));
    EXPECT_FALSE((backend.bools[{THREE_EXT_H, prop::EXT_THREE_WT_TBE_PTC3}]));
}

TEST_F(ToBeEstimatedTest, ThreeWindingsTransformerAdderUsesSideCodes) {
    MockBackend b;
    ThreeWindingsTransformer t3(THREE_H, &b);
    ThreeWindingsTransformerToBeEstimatedAdder adder = t3.newExtension<ThreeWindingsTransformerToBeEstimatedAdder>();
    const ObjectHandle adderHandle = b.lastAdder;
    adder.withRatioTapChanger1Status(true)
         .withRatioTapChangerStatus(ThreeSides::TWO, true)
         .withRatioTapChanger3Status(false)
         .withPhaseTapChanger1Status(false)
         .withPhaseTapChanger2Status(true)
         .withPhaseTapChangerStatus(ThreeSides::THREE, true);
    EXPECT_TRUE((b.bools[{adderHandle, prop::EXT_THREE_WT_TBE_RTC1}]));
    EXPECT_TRUE((b.bools[{adderHandle, prop::EXT_THREE_WT_TBE_RTC2}]));
    EXPECT_FALSE((b.bools[{adderHandle, prop::EXT_THREE_WT_TBE_RTC3}]));
    EXPECT_FALSE((b.bools[{adderHandle, prop::EXT_THREE_WT_TBE_PTC1}]));
    EXPECT_TRUE((b.bools[{adderHandle, prop::EXT_THREE_WT_TBE_PTC2}]));
    EXPECT_TRUE((b.bools[{adderHandle, prop::EXT_THREE_WT_TBE_PTC3}]));

    t3.removeExtension<ThreeWindingsTransformerToBeEstimated>();
    EXPECT_EQ((b.related[{THREE_H, prop::THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED}]), INVALID_HANDLE);
}
