#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <string>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// The Fortescue extensions, the asymmetrical load and the phase angle clocks on the real backends
// (see RealBackends.h): the same scenario on GraalVM and on JNI, with the network
// test/data/asymmetrical_test.xiidm. G: generator Fortescue; L12: line Fortescue; LD: asymmetrical
// load; T2: two windings transformer Fortescue and phase angle clock; T3: three windings
// transformer Fortescue and phase angle clock.

using namespace iidm;
using iidm::test::envPath;

namespace {

void checkInjectionsAndLine(Network& network) {
    GeneratorFortescue gf = network.getGenerator("G")->getExtension<GeneratorFortescue>();
    EXPECT_TRUE(gf.isGrounded());
    EXPECT_DOUBLE_EQ(gf.getRz(), 0.1);
    EXPECT_DOUBLE_EQ(gf.getXz(), 2.0);
    EXPECT_DOUBLE_EQ(gf.getRn(), 0.2);
    EXPECT_DOUBLE_EQ(gf.getXn(), 3.0);
    EXPECT_DOUBLE_EQ(gf.getGroundingR(), 0.5);
    EXPECT_DOUBLE_EQ(gf.getGroundingX(), 1.5);
    gf.setGrounded(false);
    EXPECT_FALSE(network.getGenerator("G")->getExtension<GeneratorFortescue>().isGrounded());

    LineFortescue lf = network.getLine("L12")->getExtension<LineFortescue>();
    EXPECT_DOUBLE_EQ(lf.getRz(), 3.0);
    EXPECT_DOUBLE_EQ(lf.getXz(), 30.0);
    EXPECT_DOUBLE_EQ(lf.getG1z(), 0.001);
    EXPECT_DOUBLE_EQ(lf.getB2z(), 0.0002);
    EXPECT_FALSE(lf.isOpenPhaseA());
    EXPECT_TRUE(lf.isOpenPhaseB());
    lf.setOpenPhaseC(true);
    EXPECT_TRUE(lf.isOpenPhaseC());

    Load load = *network.getLoad("LD");
    LoadAsymmetrical la = load.getExtension<LoadAsymmetrical>();
    EXPECT_EQ(la.getConnectionType(), LoadConnectionType::DELTA);
    EXPECT_DOUBLE_EQ(la.getDeltaPa(), 1.0);
    EXPECT_DOUBLE_EQ(la.getDeltaQc(), 0.3);
    load.removeExtension<LoadAsymmetrical>();
    EXPECT_FALSE(load.hasExtension<LoadAsymmetrical>());
    LoadAsymmetrical added = load.newExtension<LoadAsymmetricalAdder>().withConnectionType(LoadConnectionType::Y).withDeltaPb(4).add();
    EXPECT_EQ(added.getConnectionType(), LoadConnectionType::Y);
    EXPECT_DOUBLE_EQ(added.getDeltaPb(), 4.0);
}

void checkTransformers(Network& network) {
    TwoWindingsTransformer t2 = *network.getTwoWindingsTransformer("T2");
    TwoWindingsTransformerFortescue tf = t2.getExtension<TwoWindingsTransformerFortescue>();
    EXPECT_DOUBLE_EQ(tf.getRz(), 0.6);
    EXPECT_DOUBLE_EQ(tf.getXz(), 25.0);
    EXPECT_FALSE(tf.isFreeFluxes());
    EXPECT_DOUBLE_EQ(tf.getXm(), 500.0);
    EXPECT_EQ(tf.getConnectionType1(), WindingConnectionType::Y_GROUNDED);
    EXPECT_EQ(tf.getConnectionType2(), WindingConnectionType::DELTA);
    EXPECT_DOUBLE_EQ(tf.getGroundingR1(), 0.01);
    EXPECT_DOUBLE_EQ(tf.getGroundingX2(), 0.04);
    EXPECT_EQ(t2.getExtension<TwoWindingsTransformerPhaseAngleClock>().getPhaseAngleClock(), 11);
    EXPECT_THROW(t2.getExtension<TwoWindingsTransformerPhaseAngleClock>().setPhaseAngleClock(12), ValidationException);

    ThreeWindingsTransformer t3 = *network.getThreeWindingsTransformer("T3");
    ThreeWindingsTransformerFortescue t3f = t3.getExtension<ThreeWindingsTransformerFortescue>();
    LegFortescue leg1 = t3f.getLeg1();
    EXPECT_DOUBLE_EQ(leg1.getRz(), 0.11);
    EXPECT_TRUE(leg1.isFreeFluxes());
    EXPECT_EQ(leg1.getConnectionType(), WindingConnectionType::Y_GROUNDED);
    EXPECT_DOUBLE_EQ(leg1.getGroundingX(), 0.2);
    EXPECT_DOUBLE_EQ(t3f.getLeg2().getXz(), 22.0);
    EXPECT_EQ(t3f.getLeg3().getConnectionType(), WindingConnectionType::DELTA);
    t3f.getLeg2().setRz(0.25);
    EXPECT_DOUBLE_EQ(t3.getExtension<ThreeWindingsTransformerFortescue>().getLeg2().getRz(), 0.25);
    EXPECT_EQ(t3.getExtension<ThreeWindingsTransformerPhaseAngleClock>().getPhaseAngleClockLeg2(), 5);
    EXPECT_EQ(t3.getExtension<ThreeWindingsTransformerPhaseAngleClock>().getPhaseAngleClockLeg3(), 7);

    // Added again, a leg after the other
    t3.removeExtension<ThreeWindingsTransformerFortescue>();
    EXPECT_FALSE(t3.hasExtension<ThreeWindingsTransformerFortescue>());
    EXPECT_THROW(t3.newExtension<ThreeWindingsTransformerFortescueAdder>().withRz(1).add(), ValidationException); // no leg selected
    ThreeWindingsTransformerFortescue again = t3.newExtension<ThreeWindingsTransformerFortescueAdder>()
        .leg1().withRz(1).withConnectionType(WindingConnectionType::DELTA)
        .leg3().withXz(3).withFreeFluxes(true)
        .add();
    EXPECT_DOUBLE_EQ(again.getLeg1().getRz(), 1.0);
    EXPECT_EQ(again.getLeg1().getConnectionType(), WindingConnectionType::DELTA);
    EXPECT_DOUBLE_EQ(again.getLeg3().getXz(), 3.0);
    EXPECT_TRUE(again.getLeg3().isFreeFluxes());
}

void checkNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/asymmetrical_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/asymmetrical_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    Network network = Identifiable(b.getNetworkHandle(), &b).getNetwork();
    checkInjectionsAndLine(network);
    checkTransformers(network);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(AsymmetricalExtensionsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(AsymmetricalExtensionsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("asymmetrical_test");
    checkNetwork(backend);
}
#endif
