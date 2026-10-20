#include <gtest/gtest.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/PropertyCodes.h>
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

TEST_F(ToBeEstimatedTest, TwoWindingsTransformerHasExtension) {
    EXPECT_TRUE(TwoWindingsTransformer(TWT_H, &backend).hasTwoWindingsTransformerToBeEstimated());
    MockBackend empty;
    EXPECT_FALSE(TwoWindingsTransformer(TWT_H, &empty).hasTwoWindingsTransformerToBeEstimated());
    EXPECT_THROW(TwoWindingsTransformer(TWT_H, &empty).getTwoWindingsTransformerToBeEstimated(), ExtensionNotFoundException);
}

TEST_F(ToBeEstimatedTest, TwoWindingsTransformerGetSet) {
    auto ext = TwoWindingsTransformer(TWT_H, &backend).getTwoWindingsTransformerToBeEstimated();
    EXPECT_EQ(ext, TwoWindingsTransformerToBeEstimated(TWT_EXT_H, &backend));
    EXPECT_TRUE(ext.shouldEstimateRatioTapChanger());
    EXPECT_FALSE(ext.shouldEstimatePhaseTapChanger());
    ext.shouldEstimateRatioTapChanger(false).shouldEstimatePhaseTapChanger(true);
    EXPECT_FALSE((backend.bools[{TWT_EXT_H, prop::EXT_TWO_WT_TBE_RTC}]));
    EXPECT_TRUE((backend.bools[{TWT_EXT_H, prop::EXT_TWO_WT_TBE_PTC}]));
}

TEST_F(ToBeEstimatedTest, TwoWindingsTransformerNewAndRemove) {
    MockBackend b;
    TwoWindingsTransformer twt(TWT_H, &b);
    TwoWindingsTransformerToBeEstimatedAdder adder = twt.newTwoWindingsTransformerToBeEstimated();
    EXPECT_EQ(b.invocations.back(), std::make_pair(static_cast<ObjectHandle>(TWT_H), prop::TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED));
    const ObjectHandle adderHandle = b.lastAdder;
    b.addResults[adderHandle] = TWT_EXT_H;
    auto ext = adder.withRatioTapChangerStatus(true).withPhaseTapChangerStatus(false).add();
    EXPECT_TRUE((b.bools[{adderHandle, prop::EXT_TWO_WT_TBE_RTC}]));
    EXPECT_FALSE((b.bools[{adderHandle, prop::EXT_TWO_WT_TBE_PTC}]));
    EXPECT_EQ(ext, TwoWindingsTransformerToBeEstimated(TWT_EXT_H, &b));

    twt.removeTwoWindingsTransformerToBeEstimated();
    EXPECT_EQ((b.related[{TWT_H, prop::TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED}]), INVALID_HANDLE);
}

TEST_F(ToBeEstimatedTest, ThreeWindingsTransformerHasExtension) {
    EXPECT_TRUE(ThreeWindingsTransformer(THREE_H, &backend).hasThreeWindingsTransformerToBeEstimated());
    MockBackend empty;
    EXPECT_FALSE(ThreeWindingsTransformer(THREE_H, &empty).hasThreeWindingsTransformerToBeEstimated());
}

TEST_F(ToBeEstimatedTest, ThreeWindingsTransformerGet) {
    auto ext = ThreeWindingsTransformer(THREE_H, &backend).getThreeWindingsTransformerToBeEstimated();
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
    auto ext = ThreeWindingsTransformer(THREE_H, &backend).getThreeWindingsTransformerToBeEstimated();
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
    ThreeWindingsTransformerToBeEstimatedAdder adder = t3.newThreeWindingsTransformerToBeEstimated();
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

    t3.removeThreeWindingsTransformerToBeEstimated();
    EXPECT_EQ((b.related[{THREE_H, prop::THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED}]), INVALID_HANDLE);
}
