#include <gtest/gtest.h>
#include <iidm/CurrentLimits.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class CurrentLimitsTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle CL_H  = 1;
    static constexpr ObjectHandle TL1_H = 10;
    static constexpr ObjectHandle TL2_H = 11;

    void SetUp() override {
        backend.doubles[{CL_H, prop::CL_PERMANENT_LIMIT}] = 1000.0;
    }
};

TEST_F(CurrentLimitsTest, GetPermanentLimit) {
    CurrentLimits cl(CL_H, &backend);
    EXPECT_DOUBLE_EQ(cl.getPermanentLimit(), 1000.0);
}

TEST_F(CurrentLimitsTest, IsValid) {
    CurrentLimits valid(CL_H, &backend);
    CurrentLimits invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(CurrentLimitsTest, GetTemporaryLimitsEmpty) {
    CurrentLimits cl(CL_H, &backend);
    EXPECT_TRUE(cl.getTemporaryLimits().empty());
}

TEST_F(CurrentLimitsTest, GetTemporaryLimits) {
    backend.children[{CL_H, prop::TEMPORARY_LIMIT}] = {TL1_H, TL2_H};
    backend.strings[{TL1_H, prop::TL_NAME}]               = "TL1";
    backend.doubles[{TL1_H, prop::TL_VALUE}]               = 1200.0;
    backend.ints   [{TL1_H, prop::TL_ACCEPTABLE_DURATION}] = 60;
    backend.bools  [{TL1_H, prop::TL_FICTITIOUS}]          = false;
    backend.strings[{TL2_H, prop::TL_NAME}]               = "TL2";
    backend.doubles[{TL2_H, prop::TL_VALUE}]               = 1500.0;
    backend.ints   [{TL2_H, prop::TL_ACCEPTABLE_DURATION}] = 300;
    backend.bools  [{TL2_H, prop::TL_FICTITIOUS}]          = true;

    CurrentLimits cl(CL_H, &backend);
    auto tls = cl.getTemporaryLimits();
    ASSERT_EQ(tls.size(), 2u);
    EXPECT_EQ(tls[0].name, "TL1");
    EXPECT_DOUBLE_EQ(tls[0].value, 1200.0);
    EXPECT_EQ(tls[0].acceptableDuration, 60);
    EXPECT_FALSE(tls[0].fictitious);
    EXPECT_EQ(tls[1].name, "TL2");
    EXPECT_TRUE(tls[1].fictitious);
}
