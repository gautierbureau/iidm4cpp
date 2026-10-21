#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <cmath>
#include <set>
#include <string>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Tap changers, steps, three windings transformer legs and flows limits holders on the real
// backends (see RealBackends.h): the same scenario on GraalVM and on JNI, with the network
// test/data/transformers_test.xiidm.

using namespace iidm;
using iidm::test::envPath;

namespace {

std::set<std::string> groupIds(const std::vector<OperationalLimitsGroup>& groups) {
    std::set<std::string> ids;
    for (const OperationalLimitsGroup& group : groups) ids.insert(group.getId());
    return ids;
}

void checkTwoWindingsTransformer(BackendProvider& b) {
    TwoWindingsTransformer t2(b.findById(prop::TWO_WINDINGS_TRANSFORMER, "T2"), &b);
    ASSERT_TRUE(t2.hasRatioTapChanger());
    ASSERT_TRUE(t2.hasPhaseTapChanger());

    RatioTapChanger rtc = *t2.getRatioTapChanger();
    EXPECT_EQ(rtc.getLowTapPosition(), -1);
    EXPECT_EQ(rtc.getHighTapPosition(), 1);
    EXPECT_EQ(rtc.getStepCount(), 3);
    EXPECT_EQ(rtc.getTapPosition(), 0);
    EXPECT_EQ(*rtc.findTapPosition(), 0);
    EXPECT_EQ(*rtc.getSolvedTapPosition(), 1);
    EXPECT_EQ(*rtc.getNeutralPosition(), 0);
    EXPECT_TRUE(rtc.hasLoadTapChangingCapabilities());
    EXPECT_EQ(*rtc.getProperty("maker"), "ACME");

    std::map<int, RatioTapChangerStep> steps = rtc.getAllSteps();
    ASSERT_EQ(steps.size(), 3u);
    EXPECT_DOUBLE_EQ(steps.at(-1).getRho(), 0.95);
    EXPECT_DOUBLE_EQ(steps.at(-1).getX(), 2.0);
    EXPECT_DOUBLE_EQ(steps.at(1).getR(), -1.0);
    EXPECT_EQ(rtc.getStep(-1), steps.at(-1));
    EXPECT_EQ(*rtc.getCurrentStep(), steps.at(0));
    EXPECT_EQ(*rtc.getSolvedCurrentStep(), steps.at(1));
    EXPECT_EQ(*rtc.getNeutralStep(), steps.at(0));
    EXPECT_THROW(rtc.getStep(2), ValidationException);

    // It regulates through its voltage regulation
    EXPECT_TRUE(rtc.isRegulating());
    EXPECT_TRUE(rtc.isRegulatingWithMode(RegulationMode::VOLTAGE));
    VoltageRegulation regulation = *rtc.getVoltageRegulation();
    EXPECT_DOUBLE_EQ(regulation.getTargetValue(), 226.0);
    EXPECT_DOUBLE_EQ(regulation.getTargetDeadband(), 2.0);
    EXPECT_EQ(*regulation.getTerminal(), t2.getTerminal2());
    EXPECT_EQ(rtc.getRegulatingTerminal(), t2.getTerminal2());

    // Tap positions
    rtc.setTapPosition(1);
    EXPECT_EQ(*rtc.getCurrentStep(), steps.at(1));
    rtc.setTapPosition(-1).applySolvedValues();
    EXPECT_EQ(rtc.getTapPosition(), 1);
    rtc.unsetSolvedTapPosition();
    EXPECT_FALSE(rtc.getSolvedTapPosition().has_value());
    EXPECT_FALSE(rtc.getSolvedCurrentStep().has_value());
    EXPECT_THROW(rtc.unsetTapPosition(), ValidationException); // required at this validation level
    steps.at(0).setRho(1.01).setProperty("note", "changed");
    EXPECT_DOUBLE_EQ(rtc.getStep(0).getRho(), 1.01);

    PhaseTapChanger ptc = *t2.getPhaseTapChanger();
    EXPECT_EQ(ptc.getRegulationMode(), PhaseTapChangerRegulationMode::CURRENT_LIMITER);
    EXPECT_DOUBLE_EQ(ptc.getRegulationValue(), 500.0);
    EXPECT_DOUBLE_EQ(ptc.getTargetDeadband(), 10.0);
    EXPECT_FALSE(ptc.isRegulating());
    EXPECT_EQ(*ptc.getRegulationTerminal(), t2.getTerminal1());
    EXPECT_EQ(*ptc.getNeutralPosition(), 1);
    std::map<int, PhaseTapChangerStep> phaseSteps = ptc.getAllSteps();
    ASSERT_EQ(phaseSteps.size(), 3u);
    EXPECT_DOUBLE_EQ(phaseSteps.at(0).getAlpha(), -5.0);
    EXPECT_DOUBLE_EQ(phaseSteps.at(2).getAlpha(), 5.0);

    // One regulating tap changer at most
    EXPECT_THROW(ptc.setRegulating(true), ValidationException);
    regulation.setRegulating(false);
    ptc.setRegulationMode(PhaseTapChangerRegulationMode::ACTIVE_POWER_CONTROL).setRegulationValue(60)
       .setRegulationTerminal(t2.getTerminal2()).setRegulating(true);
    EXPECT_TRUE(ptc.isRegulating());
    EXPECT_FALSE(rtc.isRegulating());
    EXPECT_EQ(*ptc.getRegulationTerminal(), t2.getTerminal2());

    ptc.remove();
    EXPECT_FALSE(t2.hasPhaseTapChanger());
}

void checkThreeWindingsTransformer(BackendProvider& b) {
    ThreeWindingsTransformer t3(b.findById(prop::THREE_WINDINGS_TRANSFORMER, "T3"), &b);
    std::vector<ThreeWindingsTransformer::Leg> legs = t3.getLegs();
    ASSERT_EQ(legs.size(), 3u);
    const ThreeSides sides[] = {ThreeSides::ONE, ThreeSides::TWO, ThreeSides::THREE};
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(legs[i].getSide(), sides[i]);
        EXPECT_EQ(t3.getLeg(sides[i]), legs[i]);
        EXPECT_EQ(legs[i].getTransformer(), t3);
        EXPECT_EQ(t3.getTerminal(sides[i]), legs[i].getTerminal());
    }
    EXPECT_EQ(t3.getLeg1(), legs[0]);
    EXPECT_EQ(t3.getTerminal("VL3"), legs[2].getTerminal());
    EXPECT_EQ(*legs[2].getTerminal().getSide(), ThreeSides::THREE);
    EXPECT_DOUBLE_EQ(t3.getRatedU0(), 400.0);

    ThreeWindingsTransformer::Leg leg1 = legs[0];
    EXPECT_DOUBLE_EQ(leg1.getR(), 0.1);
    EXPECT_DOUBLE_EQ(leg1.getX(), 5.0);
    EXPECT_DOUBLE_EQ(leg1.getRatedU(), 400.0);
    EXPECT_DOUBLE_EQ(*leg1.getRatedS(), 200.0);
    EXPECT_FALSE(legs[1].getRatedS().has_value());
    legs[1].setRatedS(150).setX(6.5);
    EXPECT_DOUBLE_EQ(*legs[1].getRatedS(), 150.0);
    EXPECT_DOUBLE_EQ(legs[1].getX(), 6.5);

    // Tap changers of the legs
    EXPECT_FALSE(leg1.hasRatioTapChanger());
    PhaseTapChanger ptc = *leg1.getPhaseTapChanger();
    EXPECT_EQ(ptc.getLowTapPosition(), -2);
    EXPECT_EQ(ptc.getRegulationMode(), PhaseTapChangerRegulationMode::ACTIVE_POWER_CONTROL);
    EXPECT_TRUE(ptc.isRegulating());
    EXPECT_EQ(*ptc.getRegulationTerminal(), leg1.getTerminal());
    EXPECT_DOUBLE_EQ(ptc.getStep(-2).getAlpha(), -10.0);
    RatioTapChanger rtc2 = *legs[1].getRatioTapChanger();
    EXPECT_EQ(rtc2.getTapPosition(), 2);
    EXPECT_FALSE(rtc2.isRegulating());
    EXPECT_FALSE(rtc2.getVoltageRegulation().has_value());
    RatioTapChanger rtc3 = *legs[2].getRatioTapChanger();
    EXPECT_FALSE(rtc3.isRegulating());
    EXPECT_DOUBLE_EQ(rtc3.getVoltageRegulation()->getTargetValue(), 64.0);

    // Limits of the legs
    EXPECT_EQ(groupIds(leg1.getOperationalLimitsGroups()), (std::set<std::string>{"SUMMER", "WINTER"}));
    EXPECT_EQ(leg1.getSelectedOperationalLimitsGroup()->getId(), "WINTER");
    CurrentLimits limits = *leg1.getCurrentLimits();
    EXPECT_DOUBLE_EQ(limits.getPermanentLimit(), 400.0);
    ASSERT_EQ(limits.getTemporaryLimits().size(), 1u);
    EXPECT_EQ(limits.getTemporaryLimits()[0].getName(), "TL20");
    EXPECT_TRUE(legs[1].getOperationalLimitsGroups().empty());
    EXPECT_EQ(groupIds(legs[2].getOperationalLimitsGroups()), (std::set<std::string>{"ONLY"}));
    EXPECT_FALSE(legs[2].getSelectedOperationalLimitsGroup().has_value());
    EXPECT_FALSE(legs[2].getCurrentLimits().has_value());
}

void checkBoundaryLine(BackendProvider& b) {
    BoundaryLine dl(b.findById(prop::BOUNDARY_LINE, "DL"), &b);
    EXPECT_EQ(groupIds(dl.getOperationalLimitsGroups()), (std::set<std::string>{"DEFAULT"}));
    EXPECT_EQ(dl.getSelectedOperationalLimitsGroup()->getId(), "DEFAULT");
    EXPECT_DOUBLE_EQ(dl.getCurrentLimits()->getPermanentLimit(), 1000.0);
}

// Values, solved tap positions and steps changed (after checkTwoWindingsTransformer: the ratio tap
// changer of T2 is at position 1, its phase tap changer removed)
void checkTransformerChanges(BackendProvider& b) {
    TwoWindingsTransformer t2(b.findById(prop::TWO_WINDINGS_TRANSFORMER, "T2"), &b);
    EXPECT_EQ(t2.getSubstation()->getId(), "S");
    t2.setR(0.6).setX(21).setG(1e-6).setB(1e-5).setRatedU1(410).setRatedU2(230).setRatedS(350);
    EXPECT_DOUBLE_EQ(t2.getR(), 0.6);
    EXPECT_DOUBLE_EQ(t2.getX(), 21.0);
    EXPECT_DOUBLE_EQ(t2.getG(), 1e-6);
    EXPECT_DOUBLE_EQ(t2.getB(), 1e-5);
    EXPECT_DOUBLE_EQ(t2.getRatedU1(), 410.0);
    EXPECT_DOUBLE_EQ(t2.getRatedU2(), 230.0);
    EXPECT_EQ(t2.getRatedS(), iidm::optional<double>(350.0));
    t2.setRatedS(NAN);
    EXPECT_FALSE(t2.getRatedS());
    EXPECT_THROW(t2.setRatedU1(NAN), ValidationException);

    RatioTapChanger rtc = *t2.getRatioTapChanger();
    rtc.setSolvedTapPosition(-1);
    t2.setPhaseTapPositionToSolvedTapPosition(); // it has none: no change
    EXPECT_EQ(rtc.getTapPosition(), 1);
    t2.setRatioTapPositionToSolvedTapPosition();
    EXPECT_EQ(rtc.getTapPosition(), -1);
    rtc.setSolvedTapPosition(0);
    t2.applySolvedValues();
    EXPECT_EQ(rtc.getTapPosition(), 0);
    EXPECT_EQ(rtc.findSolvedTapPosition(), iidm::optional<int>(0));
    t2.unsetSolvedValues();
    EXPECT_FALSE(rtc.findSolvedTapPosition());

    // Two steps instead of three, from the low tap position (-1); the tap position (0) stays in
    rtc.stepsReplacer()
        .beginStep().setRho(0.9).setR(0).setX(1).setG(0).setB(0).setProperty("origin", "c++").endStep()
        .beginStep().setRho(1.0).setR(0).setX(1).setG(0).setB(0).endStep()
        .replaceSteps();
    EXPECT_EQ(rtc.getStepCount(), 2);
    EXPECT_EQ(rtc.getHighTapPosition(), 0);
    EXPECT_DOUBLE_EQ(rtc.getStep(-1).getRho(), 0.9);
    EXPECT_EQ(rtc.getStep(-1).getProperty("origin"), iidm::optional<std::string>("c++"));
    // One step only: the tap position would be out of them
    EXPECT_THROW(rtc.stepsReplacer().beginStep().setRho(1).setR(0).setX(0).setG(0).setB(0).endStep().replaceSteps(),
                 ValidationException);
    EXPECT_EQ(rtc.getStepCount(), 2);

    ThreeWindingsTransformer t3(b.findById(prop::THREE_WINDINGS_TRANSFORMER, "T3"), &b);
    EXPECT_EQ(t3.getSubstation()->getId(), "S");
    t3.setRatedU0(390);
    EXPECT_DOUBLE_EQ(t3.getRatedU0(), 390.0);
    EXPECT_EQ(t3.getSide(t3.getLeg2().getTerminal()), ThreeSides::TWO);
    EXPECT_THROW(t3.getSide(t2.getTerminal1()), ValidationException);

    // Leg 1: a phase tap changer from -2 to 0, at 0
    PhaseTapChanger ptc1 = *t3.getLeg1().getPhaseTapChanger();
    ptc1.setSolvedTapPosition(-1);
    t3.setRatioTapPositionToSolvedTapPosition(); // the ratio tap changers only
    EXPECT_EQ(ptc1.getTapPosition(), 0);
    t3.setPhaseTapPositionToSolvedTapPosition();
    EXPECT_EQ(ptc1.getTapPosition(), -1);
    ptc1.setSolvedTapPosition(-2);
    t3.applySolvedValues();
    EXPECT_EQ(ptc1.getTapPosition(), -2);
    ptc1.stepsReplacer()
        .beginStep().setAlpha(-4).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .beginStep().setAlpha(4).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .replaceSteps();
    EXPECT_EQ(ptc1.getStepCount(), 2);
    EXPECT_DOUBLE_EQ(ptc1.getStep(-1).getAlpha(), 4.0);
}

// New tap changers, which replace the current ones (after checkTransformerChanges)
void checkNewTapChangers(BackendProvider& b) {
    TwoWindingsTransformer t2(b.findById(prop::TWO_WINDINGS_TRANSFORMER, "T2"), &b);
    RatioTapChanger rtc = t2.newRatioTapChanger().setLowTapPosition(0).setTapPosition(1).setLoadTapChangingCapabilities(true)
        .setProperty("origin", "c++")
        .beginStep().setRho(0.95).setR(0).setX(0).setG(0).setB(0).endStep()
        .beginStep().setRho(1.05).setR(0).setX(0).setG(0).setB(0).setProperty("step", "high").endStep()
        .newVoltageRegulation().withMode(RegulationMode::VOLTAGE).withTargetValue(226).withTargetDeadband(2)
            .withTerminal(t2.getTerminal2()).withRegulating(true).add()
        .add();
    EXPECT_EQ(rtc, *t2.getRatioTapChanger());
    EXPECT_EQ(rtc.getStepCount(), 2);
    EXPECT_EQ(rtc.getTapPosition(), 1);
    EXPECT_DOUBLE_EQ(rtc.getStep(1).getRho(), 1.05);
    EXPECT_EQ(rtc.getStep(1).getProperty("step"), iidm::optional<std::string>("high"));
    EXPECT_EQ(rtc.getProperty("origin"), iidm::optional<std::string>("c++"));
    EXPECT_DOUBLE_EQ(rtc.getVoltageRegulation()->getTargetValue(), 226.0);
    // The tap position out of the steps: rejected, the tap changer kept
    EXPECT_THROW(t2.newRatioTapChanger().setLowTapPosition(0).setTapPosition(5)
                     .beginStep().setRho(1).setR(0).setX(0).setG(0).setB(0).endStep().add(),
                 ValidationException);
    EXPECT_EQ(t2.getRatioTapChanger()->getStepCount(), 2);

    PhaseTapChanger ptc = t2.newPhaseTapChanger().setLowTapPosition(-1).setTapPosition(0).setSolvedTapPosition(-1)
        .setRegulationMode(PhaseTapChangerRegulationMode::ACTIVE_POWER_CONTROL).setRegulationValue(80)
        .setTargetDeadband(5).setRegulating(false).setRegulationTerminal(t2.getTerminal1())
        .beginStep().setAlpha(-6).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .beginStep().setAlpha(6).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .add();
    EXPECT_TRUE(t2.hasPhaseTapChanger());
    EXPECT_DOUBLE_EQ(ptc.getStep(0).getAlpha(), 6.0);
    EXPECT_EQ(ptc.getRegulationMode(), PhaseTapChangerRegulationMode::ACTIVE_POWER_CONTROL);
    EXPECT_EQ(*ptc.getRegulationTerminal(), t2.getTerminal1());
    EXPECT_EQ(ptc.findSolvedTapPosition(), iidm::optional<int>(-1));

    // On a leg of a three windings transformer
    ThreeWindingsTransformer t3(b.findById(prop::THREE_WINDINGS_TRANSFORMER, "T3"), &b);
    ThreeWindingsTransformer::Leg leg3 = t3.getLeg3();
    EXPECT_NE(leg3.getRatioTapChanger()->getLowTapPosition(), 1);
    leg3.newRatioTapChanger().setLowTapPosition(1).setTapPosition(1)
        .beginStep().setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .add();
    EXPECT_EQ(t3.getLeg3().getRatioTapChanger()->getLowTapPosition(), 1);
    EXPECT_EQ(t3.getLeg3().getRatioTapChanger()->getStepCount(), 1);
}

// Reading tap changers, steps and legs leaves no Java reference behind
void checkHandlesAreReleased(BackendProvider& b) {
    ThreeWindingsTransformer t3(b.findById(prop::THREE_WINDINGS_TRANSFORMER, "T3"), &b);
    const std::size_t baseline = b.liveHandleCount();
    double sum = 0;
    for (int i = 0; i < 500; ++i) {
        for (const ThreeWindingsTransformer::Leg& leg : t3.getLegs()) {
            iidm::optional<RatioTapChanger> rtc = leg.getRatioTapChanger();
            if (rtc) {
                for (const auto& step : rtc->getAllSteps()) sum += step.second.getRho();
            }
            if (leg.hasPhaseTapChanger()) sum += leg.getPhaseTapChanger()->getCurrentStep()->getAlpha();
        }
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    EXPECT_NE(sum, 0.0);
}

void checkTransformers(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/transformers_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/transformers_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    checkTwoWindingsTransformer(b);
    checkThreeWindingsTransformer(b);
    checkBoundaryLine(b);
    checkHandlesAreReleased(b);
    checkTransformerChanges(b);
    checkNewTapChangers(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(TransformersTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkTransformers(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(TransformersTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("transformers_test");
    checkTransformers(backend);
}
#endif
