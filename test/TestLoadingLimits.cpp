#include <gtest/gtest.h>
#include <iidm/LoadingLimits.h>
#include <iidm/LoadingLimitsAdder.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/PropertyCodes.h>
#include <cmath>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class LoadingLimitsTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle CL_H  = 1;
    static constexpr ObjectHandle TL1_H = 10;
    static constexpr ObjectHandle TL2_H = 11;
    static constexpr ObjectHandle OLG_H = 20;

    void SetUp() override {
        backend.doubles[{CL_H, prop::LL_PERMANENT_LIMIT}] = 1000.0;
        backend.strings[{CL_H, prop::LL_PERMANENT_LIMIT_NAME}] = "PATL";
        backend.ints[{CL_H, prop::LL_LIMIT_TYPE}] = static_cast<int>(LimitType::CURRENT);
        backend.ints[{CL_H, prop::LL_DETECTION_KIND}] = static_cast<int>(DetectionKind::HIGH);
        backend.children[{CL_H, prop::TEMPORARY_LIMIT}] = {TL1_H, TL2_H};
        backend.strings[{TL1_H, prop::TL_NAME}]                = "TL1";
        backend.doubles[{TL1_H, prop::TL_VALUE}]               = 1200.0;
        backend.ints   [{TL1_H, prop::TL_ACCEPTABLE_DURATION}] = 600;
        backend.bools  [{TL1_H, prop::TL_FICTITIOUS}]          = false;
        backend.strings[{TL2_H, prop::TL_NAME}]                = "TL2";
        backend.doubles[{TL2_H, prop::TL_VALUE}]               = 1500.0;
        backend.ints   [{TL2_H, prop::TL_ACCEPTABLE_DURATION}] = 60;
        backend.bools  [{TL2_H, prop::TL_FICTITIOUS}]          = true;
    }
};

TEST_F(LoadingLimitsTest, PermanentLimit) {
    CurrentLimits cl(CL_H, &backend);
    EXPECT_DOUBLE_EQ(cl.getPermanentLimit(), 1000.0);
    EXPECT_EQ(cl.getPermanentLimitName(), "PATL");
    EXPECT_EQ(cl.getLimitType(), LimitType::CURRENT);
    EXPECT_EQ(cl.getDetectionKind(), DetectionKind::HIGH);
    cl.setPermanentLimit(900.0).setPermanentLimitName("IST");
    EXPECT_DOUBLE_EQ((backend.doubles[{CL_H, prop::LL_PERMANENT_LIMIT}]), 900.0);
    EXPECT_EQ((backend.strings[{CL_H, prop::LL_PERMANENT_LIMIT_NAME}]), "IST");
}

TEST_F(LoadingLimitsTest, TemporaryLimits) {
    CurrentLimits cl(CL_H, &backend);
    std::vector<TemporaryLimit> tls = cl.getTemporaryLimits();
    ASSERT_EQ(tls.size(), 2u);
    EXPECT_EQ(tls[0].getName(), "TL1");
    EXPECT_DOUBLE_EQ(tls[0].getValue(), 1200.0);
    EXPECT_EQ(tls[0].getAcceptableDuration(), 600);
    EXPECT_FALSE(tls[0].isFictitious());
    EXPECT_EQ(tls[1].getName(), "TL2");
    EXPECT_TRUE(tls[1].isFictitious());
    EXPECT_TRUE(CurrentLimits(CL_H + 99, &backend).getTemporaryLimits().empty());
}

TEST_F(LoadingLimitsTest, TemporaryLimitByAcceptableDuration) {
    backend.relatedByIndex[std::make_tuple(ObjectHandle(CL_H), prop::TEMPORARY_LIMIT, 60)] = TL2_H;
    backend.evaluations[std::make_tuple(ObjectHandle(CL_H), prop::LL_TEMPORARY_LIMIT_VALUE, std::vector<double>{60.0})] = 1500.0;
    CurrentLimits cl(CL_H, &backend);
    ASSERT_TRUE(cl.getTemporaryLimit(60));
    EXPECT_EQ(cl.getTemporaryLimit(60)->getName(), "TL2");
    EXPECT_FALSE(cl.getTemporaryLimit(61));
    EXPECT_DOUBLE_EQ(cl.getTemporaryLimitValue(60), 1500.0);
    EXPECT_TRUE(std::isnan(cl.getTemporaryLimitValue(61)));
}

// Two values at once: a change, applied by ACT_ADD
TEST_F(LoadingLimitsTest, SetTemporaryLimitValue) {
    CurrentLimits cl(CL_H, &backend);
    EXPECT_EQ(&cl.setTemporaryLimitValue(60, 1400.0), &cl);
    const ObjectHandle change = backend.lastAdder;
    ASSERT_EQ(backend.invocations.size(), 2u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(CL_H), prop::LL_TEMPORARY_LIMIT_VALUE));
    EXPECT_EQ((backend.ints[{change, prop::TL_ACCEPTABLE_DURATION}]), 60);
    EXPECT_DOUBLE_EQ((backend.doubles[{change, prop::TL_VALUE}]), 1400.0);
    EXPECT_EQ(backend.invocations[1], std::make_pair(change, prop::ACT_ADD));
}

TEST_F(LoadingLimitsTest, PropertiesAndRemove) {
    CurrentLimits cl(CL_H, &backend);
    cl.setProperty("source", "rte");
    EXPECT_EQ(cl.getProperty("source"), iidm::optional<std::string>("rte"));
    TemporaryLimit(TL1_H, &backend).setProperty("note", "short");
    EXPECT_EQ(TemporaryLimit(TL1_H, &backend).getProperty("note"), iidm::optional<std::string>("short"));
    cl.remove();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(CL_H), prop::ACT_REMOVE));
}

TEST_F(LoadingLimitsTest, Groups) {
    static constexpr ObjectHandle AP_H = 2;
    backend.strings[{OLG_H, prop::OLG_ID}] = "DEFAULT";
    backend.related[{OLG_H, prop::CURRENT_LIMITS}] = CL_H;
    backend.relatedByIndex[std::make_tuple(ObjectHandle(OLG_H), prop::LOADING_LIMITS, static_cast<int>(LimitType::APPARENT_POWER))] = AP_H;
    OperationalLimitsGroup group(OLG_H, &backend);
    EXPECT_EQ(group.getId(), "DEFAULT");
    EXPECT_EQ(*group.getCurrentLimits(), CurrentLimits(CL_H, &backend));
    EXPECT_FALSE(group.getActivePowerLimits());
    EXPECT_FALSE(group.getApparentPowerLimits());
    EXPECT_EQ(*group.getLoadingLimits(LimitType::APPARENT_POWER), LoadingLimits(AP_H, &backend));
    group.setProperty("season", "winter");
    EXPECT_EQ(group.getProperty("season"), iidm::optional<std::string>("winter"));

    group.removeCurrentLimits();
    group.removeApparentPowerLimits();
    EXPECT_EQ((backend.related[{OLG_H, prop::CURRENT_LIMITS}]), INVALID_HANDLE);
    EXPECT_EQ((backend.related[{OLG_H, prop::APPARENT_POWER_LIMITS}]), INVALID_HANDLE);
}

TEST_F(LoadingLimitsTest, Adders) {
    static constexpr ObjectHandle NEW_H = 3;
    OperationalLimitsGroup group(OLG_H, &backend);
    CurrentLimitsAdder adder = group.newCurrentLimits();
    const ObjectHandle adderHandle = backend.lastAdder;
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(OLG_H), prop::CURRENT_LIMITS));
    backend.addResults[adderHandle] = NEW_H;
    CurrentLimits limits = adder.setPermanentLimit(950.0).setPermanentLimitName("PATL")
        .setDetectionKind(DetectionKind::HIGH).setProperty("source", "c++")
        .beginTemporaryLimit().setName("TL").setValue(1100.0).setAcceptableDuration(300).setFictitious(true)
            .ensureNameUnicity().addProperty("rank", "1").endTemporaryLimit()
        .add();
    EXPECT_EQ(limits, CurrentLimits(NEW_H, &backend));
    EXPECT_DOUBLE_EQ((backend.doubles[{adderHandle, prop::LL_PERMANENT_LIMIT}]), 950.0);
    EXPECT_EQ((backend.strings[{adderHandle, prop::LL_PERMANENT_LIMIT_NAME}]), "PATL");
    EXPECT_EQ((backend.ints[{adderHandle, prop::LL_DETECTION_KIND}]), static_cast<int>(DetectionKind::HIGH));
    EXPECT_EQ((backend.keyedStrings[std::make_tuple(adderHandle, prop::PROPERTY, std::string("source"))]), "c++");

    // The nested temporary limit adder, started by the limits adder
    const ObjectHandle tlAdder = adderHandle + 1;
    EXPECT_EQ(backend.invocations[1], std::make_pair(adderHandle, prop::TEMPORARY_LIMIT));
    EXPECT_EQ((backend.strings[{tlAdder, prop::TL_NAME}]), "TL");
    EXPECT_DOUBLE_EQ((backend.doubles[{tlAdder, prop::TL_VALUE}]), 1100.0);
    EXPECT_EQ((backend.ints[{tlAdder, prop::TL_ACCEPTABLE_DURATION}]), 300);
    EXPECT_TRUE((backend.bools[{tlAdder, prop::TL_FICTITIOUS}]));
    EXPECT_TRUE((backend.bools[{tlAdder, prop::ENSURE_ID_UNICITY}]));
    EXPECT_EQ((backend.keyedStrings[std::make_tuple(tlAdder, prop::PROPERTY, std::string("rank"))]), "1");
    EXPECT_EQ(backend.invocations[2], std::make_pair(tlAdder, prop::ACT_ADD));
    EXPECT_EQ(backend.invocations[3], std::make_pair(adderHandle, prop::ACT_ADD));

    group.newActivePowerLimits();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(OLG_H), prop::ACTIVE_POWER_LIMITS));
    group.newApparentPowerLimits();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(OLG_H), prop::APPARENT_POWER_LIMITS));
}
