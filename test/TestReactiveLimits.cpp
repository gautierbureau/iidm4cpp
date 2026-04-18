#include <gtest/gtest.h>
#include <iidm/Generator.h>
#include <iidm/VscConverterStation.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class ReactiveLimitsTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle GEN_HANDLE  = 40;
    static constexpr ObjectHandle VSC_HANDLE  = 41;
    static constexpr ObjectHandle PT0_HANDLE  = 50;
    static constexpr ObjectHandle PT1_HANDLE  = 51;
    static constexpr ObjectHandle PT2_HANDLE  = 52;

    void SetUp() override {
        // Generator with MinMaxReactiveLimits
        backend.strings[{GEN_HANDLE, prop::ID}]   = "GEN1";
        backend.strings[{GEN_HANDLE, prop::NAME}]  = "Generator 1";
        backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_P}]          = 100.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_Q}]          = 0.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_V}]          = 225.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_MIN_P}]             = 0.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_MAX_P}]             = 200.0;
        backend.bools  [{GEN_HANDLE, prop::GEN_VOLTAGE_REGULATOR_ON}] = true;
        backend.ints   [{GEN_HANDLE, prop::GEN_ENERGY_SOURCE}]     = 0;
        backend.ints   [{GEN_HANDLE, prop::GEN_REACTIVE_LIMITS_KIND}] = 1; // MIN_MAX
        backend.doubles[{GEN_HANDLE, prop::GEN_MIN_Q}]             = -150.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_MAX_Q}]             = 150.0;

        // VSC with ReactiveCapabilityCurve
        backend.strings[{VSC_HANDLE, prop::ID}]   = "VSC1";
        backend.ints   [{VSC_HANDLE, prop::VSC_REACTIVE_LIMITS_KIND}] = 2; // CURVE
        backend.children[{VSC_HANDLE, prop::REACTIVE_CURVE_POINT}] =
            {PT0_HANDLE, PT1_HANDLE, PT2_HANDLE};
        backend.doubles[{PT0_HANDLE, prop::POINT_P}]     = 0.0;
        backend.doubles[{PT0_HANDLE, prop::POINT_MIN_Q}] = -50.0;
        backend.doubles[{PT0_HANDLE, prop::POINT_MAX_Q}] =  50.0;
        backend.doubles[{PT1_HANDLE, prop::POINT_P}]     = 100.0;
        backend.doubles[{PT1_HANDLE, prop::POINT_MIN_Q}] = -80.0;
        backend.doubles[{PT1_HANDLE, prop::POINT_MAX_Q}] =  80.0;
        backend.doubles[{PT2_HANDLE, prop::POINT_P}]     = 200.0;
        backend.doubles[{PT2_HANDLE, prop::POINT_MIN_Q}] = -30.0;
        backend.doubles[{PT2_HANDLE, prop::POINT_MAX_Q}] =  30.0;
    }
};

// ── MinMaxReactiveLimits on Generator ─────────────────────────────────────────

TEST_F(ReactiveLimitsTest, GenHasMinMaxReactiveLimits) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_TRUE(gen.hasMinMaxReactiveLimits());
    EXPECT_FALSE(gen.hasReactiveCapabilityCurve());
}

TEST_F(ReactiveLimitsTest, GenMinMaxValues) {
    Generator gen(GEN_HANDLE, &backend);
    MinMaxReactiveLimits lim = gen.getMinMaxReactiveLimits();
    EXPECT_TRUE(lim.isValid());
    EXPECT_DOUBLE_EQ(lim.getMinQ(), -150.0);
    EXPECT_DOUBLE_EQ(lim.getMaxQ(),  150.0);
}

TEST_F(ReactiveLimitsTest, GenNoReactiveLimits) {
    MockBackend b2;
    b2.strings[{GEN_HANDLE, prop::ID}] = "GEN2";
    b2.ints[{GEN_HANDLE, prop::GEN_REACTIVE_LIMITS_KIND}] = 0; // NONE
    Generator gen(GEN_HANDLE, &b2);
    EXPECT_FALSE(gen.hasMinMaxReactiveLimits());
    EXPECT_FALSE(gen.hasReactiveCapabilityCurve());
}

// ── ReactiveCapabilityCurve on VscConverterStation ───────────────────────────

TEST_F(ReactiveLimitsTest, VscHasReactiveCapabilityCurve) {
    VscConverterStation vsc(VSC_HANDLE, &backend);
    EXPECT_FALSE(vsc.hasMinMaxReactiveLimits());
    EXPECT_TRUE(vsc.hasReactiveCapabilityCurve());
}

TEST_F(ReactiveLimitsTest, VscCurvePoints) {
    VscConverterStation vsc(VSC_HANDLE, &backend);
    ReactiveCapabilityCurve curve = vsc.getReactiveCapabilityCurve();
    EXPECT_TRUE(curve.isValid());

    auto pts = curve.getPoints();
    ASSERT_EQ(pts.size(), 3u);

    EXPECT_DOUBLE_EQ(pts[0].p,      0.0);
    EXPECT_DOUBLE_EQ(pts[0].minQ, -50.0);
    EXPECT_DOUBLE_EQ(pts[0].maxQ,  50.0);

    EXPECT_DOUBLE_EQ(pts[1].p,    100.0);
    EXPECT_DOUBLE_EQ(pts[1].minQ, -80.0);
    EXPECT_DOUBLE_EQ(pts[1].maxQ,  80.0);

    EXPECT_DOUBLE_EQ(pts[2].p,    200.0);
    EXPECT_DOUBLE_EQ(pts[2].minQ, -30.0);
    EXPECT_DOUBLE_EQ(pts[2].maxQ,  30.0);
}

// ── Generator with ReactiveCapabilityCurve ────────────────────────────────────

TEST_F(ReactiveLimitsTest, GenWithCurve) {
    MockBackend b2;
    b2.strings[{GEN_HANDLE, prop::ID}] = "GEN3";
    b2.ints[{GEN_HANDLE, prop::GEN_REACTIVE_LIMITS_KIND}] = 2; // CURVE
    b2.children[{GEN_HANDLE, prop::REACTIVE_CURVE_POINT}] = {PT0_HANDLE};
    b2.doubles[{PT0_HANDLE, prop::POINT_P}]     = 50.0;
    b2.doubles[{PT0_HANDLE, prop::POINT_MIN_Q}] = -60.0;
    b2.doubles[{PT0_HANDLE, prop::POINT_MAX_Q}] =  60.0;

    Generator gen(GEN_HANDLE, &b2);
    EXPECT_FALSE(gen.hasMinMaxReactiveLimits());
    EXPECT_TRUE(gen.hasReactiveCapabilityCurve());
    auto pts = gen.getReactiveCapabilityCurve().getPoints();
    ASSERT_EQ(pts.size(), 1u);
    EXPECT_DOUBLE_EQ(pts[0].p,     50.0);
    EXPECT_DOUBLE_EQ(pts[0].minQ, -60.0);
    EXPECT_DOUBLE_EQ(pts[0].maxQ,  60.0);
}
