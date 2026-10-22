#include <gtest/gtest.h>
#include <iidm/BoundaryLine.h>
#include <iidm/TieLine.h>
#include <iidm/LoadingLimits.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/VoltageLevel.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class BoundaryLineTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle BL_HANDLE = 10;
    static constexpr ObjectHandle CL_HANDLE = 11;

    void SetUp() override {
        backend.strings[{BL_HANDLE, prop::ID}]    = "DL1";
        backend.strings[{BL_HANDLE, prop::NAME}]   = "Boundary Line 1";
        backend.doubles[{BL_HANDLE, prop::BL_P0}]  = 50.0;
        backend.doubles[{BL_HANDLE, prop::BL_Q0}]  = 10.0;
        backend.doubles[{BL_HANDLE, prop::BL_R}]   = 1.0;
    }
};

// ── getCurrentLimits ──────────────────────────────────────────────────────────

TEST_F(BoundaryLineTest, GetCurrentLimitsAbsent) {
    BoundaryLine dl(BL_HANDLE, &backend);
    EXPECT_FALSE(dl.getCurrentLimits());
}

TEST_F(BoundaryLineTest, GetCurrentLimitsPresent) {
    backend.related[{BL_HANDLE, prop::CURRENT_LIMITS}]     = CL_HANDLE;
    backend.doubles[{CL_HANDLE, prop::LL_PERMANENT_LIMIT}] = 300.0;
    BoundaryLine dl(BL_HANDLE, &backend);
    auto cl = dl.getCurrentLimits();
    ASSERT_TRUE(cl);
    EXPECT_DOUBLE_EQ(cl->getPermanentLimit(), 300.0);
}

// Its limits are those of a flows limits holder: one set of groups
TEST_F(BoundaryLineTest, OperationalLimitsGroups) {
    static constexpr ObjectHandle OLG_HANDLE = 12;
    backend.children[{BL_HANDLE, prop::OPERATIONAL_LIMITS_GROUP}] = {OLG_HANDLE};
    backend.related [{BL_HANDLE, prop::REL_SELECTED_OLG}]         = OLG_HANDLE;
    backend.strings [{OLG_HANDLE, prop::OLG_ID}]                  = "DEFAULT";
    BoundaryLine dl(BL_HANDLE, &backend);
    auto groups = dl.getOperationalLimitsGroups();
    ASSERT_EQ(groups.size(), 1u);
    EXPECT_EQ(groups[0].getId(), "DEFAULT");
    auto selected = dl.getSelectedOperationalLimitsGroup();
    ASSERT_TRUE(selected);
    EXPECT_EQ(*selected, groups[0]);
}

TEST_F(BoundaryLineTest, GetTerminal) {
    static constexpr ObjectHandle TERM_H = 50;
    backend.related[{BL_HANDLE, prop::REL_TERMINAL}] = TERM_H;
    backend.doubles[{TERM_H, prop::TERMINAL_P}]      = 50.0;
    BoundaryLine dl(BL_HANDLE, &backend);
    EXPECT_TRUE(dl.getTerminal().isValid());
}

TEST_F(BoundaryLineTest, Generation) {
    static constexpr ObjectHandle GEN_H = 20;
    static constexpr ObjectHandle RL_H = 21;
    BoundaryLine dl(BL_HANDLE, &backend);
    EXPECT_FALSE(dl.getGeneration());
    backend.related[{BL_HANDLE, prop::BOUNDARY_LINE_GENERATION}] = GEN_H;
    backend.doubles[{GEN_H, prop::GENERATION_TARGET_P}] = 20.0;
    backend.doubles[{GEN_H, prop::GENERATION_MIN_P}] = 0.0;
    backend.doubles[{GEN_H, prop::GENERATION_MAX_P}] = 100.0;
    backend.doubles[{GEN_H, prop::GENERATION_TARGET_Q}] = 5.0;
    backend.doubles[{GEN_H, prop::GENERATION_TARGET_V}] = 401.0;
    backend.bools  [{GEN_H, prop::GENERATION_VOLTAGE_REGULATION_ON}] = true;
    backend.related[{GEN_H, prop::MIN_MAX_REACTIVE_LIMITS}] = RL_H;
    backend.doubles[{RL_H, prop::RL_MAX_Q}] = 60.0;
    BoundaryLine::Generation generation = *dl.getGeneration();
    EXPECT_DOUBLE_EQ(generation.getTargetP(), 20.0);
    EXPECT_DOUBLE_EQ(generation.getMinP(), 0.0);
    EXPECT_DOUBLE_EQ(generation.getMaxP(), 100.0);
    EXPECT_DOUBLE_EQ(generation.getTargetQ(), 5.0);
    EXPECT_DOUBLE_EQ(generation.getTargetV(), 401.0);
    EXPECT_TRUE(generation.isVoltageRegulationOn());
    EXPECT_DOUBLE_EQ(generation.getReactiveLimits<MinMaxReactiveLimits>().getMaxQ(), 60.0);
    generation.setTargetP(25.0).setMinP(-10.0).setMaxP(90.0).setTargetQ(4.0).setTargetV(399.0)
              .setVoltageRegulationOn(false);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_H, prop::GENERATION_TARGET_P}]), 25.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_H, prop::GENERATION_MIN_P}]), -10.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_H, prop::GENERATION_MAX_P}]), 90.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_H, prop::GENERATION_TARGET_Q}]), 4.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_H, prop::GENERATION_TARGET_V}]), 399.0);
    EXPECT_FALSE((backend.bools[{GEN_H, prop::GENERATION_VOLTAGE_REGULATION_ON}]));
}

TEST_F(BoundaryLineTest, Boundary) {
    static constexpr ObjectHandle BOUNDARY_H = 30;
    static constexpr ObjectHandle VL_H = 31;
    backend.related[{BL_HANDLE, prop::BOUNDARY}] = BOUNDARY_H;
    backend.related[{BOUNDARY_H, prop::BOUNDARY_LINE}] = BL_HANDLE;
    backend.related[{BOUNDARY_H, prop::REL_VOLTAGE_LEVEL}] = VL_H;
    backend.strings[{VL_H, prop::ID}] = "VL1";
    backend.doubles[{BOUNDARY_H, prop::BOUNDARY_V}] = 401.0;
    backend.doubles[{BOUNDARY_H, prop::BOUNDARY_ANGLE}] = -2.0;
    backend.doubles[{BOUNDARY_H, prop::BOUNDARY_P}] = 12.0;
    backend.doubles[{BOUNDARY_H, prop::BOUNDARY_Q}] = 6.0;
    backend.doubles[{BOUNDARY_H, prop::BOUNDARY_I}] = 19.3;
    BoundaryLine dl(BL_HANDLE, &backend);
    Boundary boundary = dl.getBoundary();
    EXPECT_DOUBLE_EQ(boundary.getV(), 401.0);
    EXPECT_DOUBLE_EQ(boundary.getAngle(), -2.0);
    EXPECT_DOUBLE_EQ(boundary.getP(), 12.0);
    EXPECT_DOUBLE_EQ(boundary.getQ(), 6.0);
    EXPECT_DOUBLE_EQ(boundary.getI(), 19.3);
    EXPECT_EQ(boundary.getBoundaryLine(), dl);
    EXPECT_EQ(boundary.getNetworkSideVoltageLevel().getId(), "VL1");
}

TEST_F(BoundaryLineTest, SolvedValues) {
    BoundaryLine dl(BL_HANDLE, &backend);
    dl.applySolvedValues();
    dl.setGenerationTargetPToP();
    dl.setGenerationTargetQToQ();
    dl.setGenerationTargetVToV();
    ASSERT_EQ(backend.invocations.size(), 4u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(BL_HANDLE), prop::ACT_APPLY_SOLVED_VALUES));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(BL_HANDLE), prop::ACT_SET_TARGET_P_TO_P));
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(BL_HANDLE), prop::ACT_SET_TARGET_Q_TO_Q));
    EXPECT_EQ(backend.invocations[3], std::make_pair(ObjectHandle(BL_HANDLE), prop::ACT_SET_TARGET_V_TO_V));
}

TEST_F(BoundaryLineTest, TieLine) {
    static constexpr ObjectHandle TIE = 30, OTHER = 31;
    backend.related[{BL_HANDLE, prop::TIE_LINE}] = TIE;
    backend.relatedByIndex[{TIE, prop::BOUNDARY_LINE, 1}] = OTHER;
    backend.strings[{OTHER, prop::ID}] = "DL2";
    backend.relatedByString[std::make_tuple(ObjectHandle(TIE), prop::BOUNDARY_LINE, std::string("VL1"))] = BL_HANDLE;
    backend.optStrings[{TIE, prop::BL_PAIRING_KEY}] = "XNODE";
    backend.doubles[{TIE, prop::LINE_X}] = 20.0;
    BoundaryLine dl(BL_HANDLE, &backend);
    iidm::optional<TieLine> tie = dl.getTieLine();
    ASSERT_TRUE(tie);
    EXPECT_EQ(tie->getPairingKey(), iidm::optional<std::string>("XNODE"));
    EXPECT_DOUBLE_EQ(tie->getX(), 20.0);
    EXPECT_EQ(tie->getBoundaryLine2().getId(), "DL2");
    EXPECT_EQ(tie->getBoundaryLine("VL1")->getId(), "DL1");
    EXPECT_FALSE(tie->getBoundaryLine("VL9"));

    tie->remove(true);
    EXPECT_EQ(backend.invocations.back(), std::make_pair(TIE, prop::ACT_REMOVE_UPDATING_BOUNDARY_LINES));
    tie->remove();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(TIE, prop::ACT_REMOVE));
    EXPECT_TRUE(tie->disconnectBoundaryLines());
    EXPECT_EQ(backend.invocations.back(), std::make_pair(TIE, prop::ACT_DISCONNECT));
    EXPECT_FALSE(BoundaryLine(CL_HANDLE, &backend).getTieLine());
}
