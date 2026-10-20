#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include <memory>
#include <utility>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

// Using an object that is not bound to a network throws InvalidHandleException
// instead of dereferencing a null pointer.

TEST(NullBackendTest, DefaultConstructedComponents) {
    EXPECT_THROW(Generator().getTargetP(), InvalidHandleException);
    EXPECT_THROW(Generator().setTargetP(1.0), InvalidHandleException);
    EXPECT_THROW(Generator().getId(), InvalidHandleException);
    EXPECT_THROW(Load().getP0(), InvalidHandleException);
    EXPECT_THROW(Line().getTerminal1().getP(), InvalidHandleException);
    EXPECT_THROW(Terminal().getP(), InvalidHandleException);
    EXPECT_THROW(Terminal().connect(), InvalidHandleException);
    EXPECT_THROW(Bus().getV(), InvalidHandleException);
    EXPECT_THROW(VoltageLevel().getGenerators(), InvalidHandleException);
    EXPECT_THROW(Substation().getVoltageLevels(), InvalidHandleException);
    EXPECT_THROW(TwoWindingsTransformer().getRatioTapChanger(), InvalidHandleException);
    EXPECT_THROW(ThreeWindingsTransformer().getLeg1().getR(), InvalidHandleException);
    EXPECT_THROW(ThreeWindingsTransformer::Leg().getR(), InvalidHandleException);
    EXPECT_THROW(VoltageLevel::NodeBreakerView().getNodes(), InvalidHandleException);
    EXPECT_THROW(HvdcLine().getActivePowerSetpoint(), InvalidHandleException);
    EXPECT_THROW(Switch().isOpen(), InvalidHandleException);
}

TEST(NullBackendTest, DefaultConstructedExtensions) {
    EXPECT_THROW(ActivePowerControl().getDroop(), InvalidHandleException);
    EXPECT_THROW(ActivePowerControl().getMinTargetP(), InvalidHandleException);
    EXPECT_THROW(SlackTerminal().setTerminal(Terminal()), InvalidHandleException);
    EXPECT_THROW(Measurement().getValue(), InvalidHandleException);
    EXPECT_THROW(Measurements().getMeasurements(), InvalidHandleException);
    EXPECT_THROW(DiscreteMeasurement().setValue("OPEN"), InvalidHandleException);
    EXPECT_THROW(InjectionObservability().getQualityP(), InvalidHandleException);
    EXPECT_THROW(ObservabilityArea().getObservabilityAreas(), InvalidHandleException);
    EXPECT_THROW(ObservabilityArea::AreaCharacteristics().getTerminals(), InvalidHandleException);
}

TEST(NullBackendTest, ExplicitNullBackend) {
    EXPECT_THROW(Generator(42, nullptr).getTargetP(), InvalidHandleException);
    EXPECT_THROW(ActivePowerControl(42, nullptr).getDroop(), InvalidHandleException);
}

TEST(NullBackendTest, HandleChecksDoNotThrow) {
    Generator g;
    EXPECT_FALSE(g.isValid());
    EXPECT_TRUE(g == Generator());
    EXPECT_FALSE(Measurement().isValid());
}

TEST(NullBackendTest, MovedFromNetwork) {
    Network net = NetworkFactory::createFromBackend(std::unique_ptr<BackendProvider>(new MockBackend()));
    Network other = std::move(net);
    EXPECT_NO_THROW(other.getGenerators());
    EXPECT_THROW(net.getGenerators(), InvalidHandleException);  // NOLINT(bugprone-use-after-move)
    EXPECT_THROW(net.getId(), InvalidHandleException);          // NOLINT(bugprone-use-after-move)
    // destroying the moved-from network is fine
}
