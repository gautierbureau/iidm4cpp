#include <gtest/gtest.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

// Its voltage regulation is the VoltageRegulationHolder's: see TestVoltageRegulationHolder.

using namespace iidm;
using namespace iidm::test;

class StaticVarCompensatorTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle SVC_H  = 10;
    static constexpr ObjectHandle TERM_H = 20;

    void SetUp() override {
        backend.strings[{SVC_H, prop::ID}]                    = "SVC1";
        backend.strings[{SVC_H, prop::NAME}]                   = "Static Var 1";
        backend.doubles[{SVC_H, prop::SVC_B_MIN}]              = -0.1;
        backend.doubles[{SVC_H, prop::SVC_B_MAX}]              = 0.1;
        backend.related[{SVC_H, prop::REL_TERMINAL}]           = TERM_H;
        backend.related[{SVC_H, prop::REL_REGULATING_TERMINAL}] = TERM_H;
    }
};

TEST_F(StaticVarCompensatorTest, GetId) {
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_EQ(svc.getId(), "SVC1");
}

TEST_F(StaticVarCompensatorTest, BLimits) {
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_DOUBLE_EQ(svc.getBmin(), -0.1);
    EXPECT_DOUBLE_EQ(svc.getBmax(),  0.1);
    svc.setBmin(-0.2).setBmax(0.2);
    EXPECT_DOUBLE_EQ((backend.doubles[{SVC_H, prop::SVC_B_MIN}]), -0.2);
    EXPECT_DOUBLE_EQ((backend.doubles[{SVC_H, prop::SVC_B_MAX}]), 0.2);
}

TEST_F(StaticVarCompensatorTest, GetTerminal) {
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_TRUE(svc.getTerminal().isValid());
    EXPECT_EQ(svc.getRegulatingTerminal(), svc.getTerminal());
}

TEST_F(StaticVarCompensatorTest, IsValid) {
    StaticVarCompensator valid(SVC_H, &backend);
    StaticVarCompensator invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
