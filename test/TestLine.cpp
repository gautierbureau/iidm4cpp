#include <gtest/gtest.h>
#include <iidm/Line.h>
#include <iidm/CurrentLimits.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class LineTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle LINE_HANDLE  = 50;
    static constexpr ObjectHandle TERM1_HANDLE = 51;
    static constexpr ObjectHandle TERM2_HANDLE = 52;

    void SetUp() override {
        backend.strings[{LINE_HANDLE, prop::ID}]   = "LINE1";
        backend.strings[{LINE_HANDLE, prop::NAME}]  = "Line 1";
        backend.doubles[{LINE_HANDLE, prop::LINE_R}]  = 0.01;
        backend.doubles[{LINE_HANDLE, prop::LINE_X}]  = 0.1;
        backend.doubles[{LINE_HANDLE, prop::LINE_G1}] = 0.0;
        backend.doubles[{LINE_HANDLE, prop::LINE_B1}] = 1e-4;
        backend.doubles[{LINE_HANDLE, prop::LINE_G2}] = 0.0;
        backend.doubles[{LINE_HANDLE, prop::LINE_B2}] = 1e-4;
        backend.related[{LINE_HANDLE, prop::REL_TERMINAL1}] = TERM1_HANDLE;
        backend.related[{LINE_HANDLE, prop::REL_TERMINAL2}] = TERM2_HANDLE;
    }
};

TEST_F(LineTest, GetId) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_EQ(line.getId(), "LINE1");
}

TEST_F(LineTest, GetRX) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(line.getR(), 0.01);
    EXPECT_DOUBLE_EQ(line.getX(), 0.1);
}

TEST_F(LineTest, GetG1B1G2B2) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(line.getG1(), 0.0);
    EXPECT_DOUBLE_EQ(line.getB1(), 1e-4);
    EXPECT_DOUBLE_EQ(line.getG2(), 0.0);
    EXPECT_DOUBLE_EQ(line.getB2(), 1e-4);
}

TEST_F(LineTest, GetTerminals) {
    Line line(LINE_HANDLE, &backend);
    Terminal t1 = line.getTerminal1();
    Terminal t2 = line.getTerminal2();
    EXPECT_TRUE(t1.isValid());
    EXPECT_TRUE(t2.isValid());
    EXPECT_NE(t1, t2);
}

// ── getCurrentLimits ──────────────────────────────────────────────────────────

static constexpr ObjectHandle CL1_HANDLE = 60;
static constexpr ObjectHandle CL2_HANDLE = 61;
static constexpr ObjectHandle TL1_HANDLE = 70;

TEST_F(LineTest, GetCurrentLimitsAbsent) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_FALSE(line.getCurrentLimits1().has_value());
    EXPECT_FALSE(line.getCurrentLimits2().has_value());
}

TEST_F(LineTest, GetCurrentLimits1PermanentLimit) {
    backend.related[{LINE_HANDLE, prop::REL_CURRENT_LIMITS1}] = CL1_HANDLE;
    backend.doubles[{CL1_HANDLE, prop::CL_PERMANENT_LIMIT}]   = 1000.0;
    Line line(LINE_HANDLE, &backend);
    auto cl = line.getCurrentLimits1();
    ASSERT_TRUE(cl.has_value());
    EXPECT_TRUE(cl->isValid());
    EXPECT_DOUBLE_EQ(cl->getPermanentLimit(), 1000.0);
}

TEST_F(LineTest, GetCurrentLimits2PermanentLimit) {
    backend.related[{LINE_HANDLE, prop::REL_CURRENT_LIMITS2}] = CL2_HANDLE;
    backend.doubles[{CL2_HANDLE, prop::CL_PERMANENT_LIMIT}]   = 800.0;
    Line line(LINE_HANDLE, &backend);
    auto cl = line.getCurrentLimits2();
    ASSERT_TRUE(cl.has_value());
    EXPECT_DOUBLE_EQ(cl->getPermanentLimit(), 800.0);
}

TEST_F(LineTest, GetCurrentLimitsTemporaryLimits) {
    backend.related[{LINE_HANDLE, prop::REL_CURRENT_LIMITS1}] = CL1_HANDLE;
    backend.doubles[{CL1_HANDLE, prop::CL_PERMANENT_LIMIT}]   = 1000.0;
    backend.children[{CL1_HANDLE, prop::TEMPORARY_LIMIT}]     = {TL1_HANDLE};
    backend.strings[{TL1_HANDLE, prop::TL_NAME}]              = "10min";
    backend.doubles[{TL1_HANDLE, prop::TL_VALUE}]             = 1200.0;
    backend.ints   [{TL1_HANDLE, prop::TL_ACCEPTABLE_DURATION}] = 600;
    backend.bools  [{TL1_HANDLE, prop::TL_FICTITIOUS}]         = false;

    Line line(LINE_HANDLE, &backend);
    auto cl = line.getCurrentLimits1();
    ASSERT_TRUE(cl.has_value());
    auto tls = cl->getTemporaryLimits();
    ASSERT_EQ(tls.size(), 1u);
    EXPECT_EQ(tls[0].name,               "10min");
    EXPECT_DOUBLE_EQ(tls[0].value,        1200.0);
    EXPECT_EQ(tls[0].acceptableDuration,  600);
    EXPECT_FALSE(tls[0].fictitious);
}

TEST_F(LineTest, GetCurrentLimitsNoTemporaryLimits) {
    backend.related[{LINE_HANDLE, prop::REL_CURRENT_LIMITS1}] = CL1_HANDLE;
    backend.doubles[{CL1_HANDLE, prop::CL_PERMANENT_LIMIT}]   = 500.0;
    Line line(LINE_HANDLE, &backend);
    auto cl = line.getCurrentLimits1();
    ASSERT_TRUE(cl.has_value());
    EXPECT_EQ(cl->getTemporaryLimits().size(), 0u);
}
