#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <cmath>
#include <string>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// The data extensions on the real backends (see RealBackends.h): the same scenario on GraalVM and on
// JNI, with the network test/data/data_extensions_test.xiidm. G1: short circuit, ENTSO-E category,
// startup, manual frequency restoration reserve; BAT: short circuit; VL1: identifiable short
// circuit; LD: load detail; SVC: standby automaton; L12: operating status. G2 has none.

using namespace iidm;
using iidm::test::envPath;

namespace {

void checkGenerators(Network& network) {
    Generator g1 = *network.getGenerator("G1");
    GeneratorShortCircuit sc = g1.getExtension<GeneratorShortCircuit>();
    EXPECT_DOUBLE_EQ(sc.getDirectSubtransX(), 0.2);
    EXPECT_DOUBLE_EQ(sc.getDirectTransX(), 0.3);
    EXPECT_DOUBLE_EQ(sc.getStepUpTransformerX(), 0.1);
    sc.setDirectTransX(0.35);
    EXPECT_DOUBLE_EQ(g1.getExtension<GeneratorShortCircuit>().getDirectTransX(), 0.35);
    EXPECT_EQ(g1.getExtension<GeneratorEntsoeCategory>().getCode(), 4);
    GeneratorStartup startup = g1.getExtension<GeneratorStartup>();
    EXPECT_DOUBLE_EQ(startup.getPlannedActivePowerSetpoint(), 180.0);
    EXPECT_DOUBLE_EQ(startup.getStartupCost(), 1000.0);
    EXPECT_DOUBLE_EQ(startup.getMarginalCost(), 30.0);
    EXPECT_DOUBLE_EQ(startup.getPlannedOutageRate(), 0.1);
    EXPECT_DOUBLE_EQ(startup.getForcedOutageRate(), 0.05);
    EXPECT_TRUE(g1.getExtension<ManualFrequencyRestorationReserve>().isParticipate());

    // Added: XIIDM keeps no dynamic model info
    EXPECT_FALSE(g1.hasExtension<DynamicModelInfo>());
    DynamicModelInfo info = g1.newExtension<DynamicModelInfoAdder>().setModelName("GeneratorSynchronousFourWindings").add();
    EXPECT_EQ(info.getModelName(), "GeneratorSynchronousFourWindings");
    EXPECT_EQ(g1.getExtension<DynamicModelInfo>().setModelName("GeneratorFictitious").getModelName(), "GeneratorFictitious");

    Generator g2 = *network.getGenerator("G2");
    EXPECT_FALSE(g2.hasExtension<GeneratorShortCircuit>());
    EXPECT_THROW(g2.getExtension<GeneratorStartup>(), ExtensionNotFoundException);
    EXPECT_EQ(g2.newExtension<GeneratorEntsoeCategoryAdder>().withCode(7).add().getCode(), 7);
    GeneratorStartup g2Startup = g2.newExtension<GeneratorStartupAdder>().withMarginalCost(12).add();
    EXPECT_DOUBLE_EQ(g2Startup.getMarginalCost(), 12.0);
    EXPECT_TRUE(std::isnan(g2Startup.getStartupCost()));
    g2.removeExtension<GeneratorStartup>();
    EXPECT_FALSE(g2.hasExtension<GeneratorStartup>());
    g2.newExtension<ManualFrequencyRestorationReserveAdder>().withParticipate(false).add();
    EXPECT_FALSE(g2.getExtension<ManualFrequencyRestorationReserve>().isParticipate());

    Battery battery = *network.getBattery("BAT");
    EXPECT_DOUBLE_EQ(battery.getExtension<BatteryShortCircuit>().getDirectSubtransX(), 0.4);
    EXPECT_TRUE(std::isnan(battery.getExtension<BatteryShortCircuit>().getStepUpTransformerX()));
}

void checkOthers(Network& network) {
    IdentifiableShortCircuit isc = network.getVoltageLevel("VL1")->getExtension<IdentifiableShortCircuit>();
    EXPECT_DOUBLE_EQ(isc.getIpMin(), 1000.0);
    EXPECT_DOUBLE_EQ(isc.getIpMax(), 5000.0);
    EXPECT_FALSE(network.getVoltageLevel("VL2")->hasExtension<IdentifiableShortCircuit>());

    Load load = *network.getLoad("LD");
    LoadDetail detail = load.getExtension<LoadDetail>();
    EXPECT_DOUBLE_EQ(detail.getFixedActivePower(), 30.0);
    EXPECT_DOUBLE_EQ(detail.getVariableReactivePower(), 6.0);
    detail.setVariableActivePower(55);
    EXPECT_DOUBLE_EQ(load.getExtension<LoadDetail>().getVariableActivePower(), 55.0);

    StaticVarCompensator svc = *network.getStaticVarCompensator("SVC");
    StandbyAutomaton automaton = svc.getExtension<StandbyAutomaton>();
    EXPECT_TRUE(automaton.isStandby());
    EXPECT_DOUBLE_EQ(automaton.getB0(), 0.001);
    EXPECT_DOUBLE_EQ(automaton.getLowVoltageSetpoint(), 390.0);
    EXPECT_DOUBLE_EQ(automaton.getHighVoltageSetpoint(), 410.0);
    EXPECT_DOUBLE_EQ(automaton.getLowVoltageThreshold(), 380.0);
    EXPECT_DOUBLE_EQ(automaton.getHighVoltageThreshold(), 420.0);
    // In standby, the low threshold must stay below the high one
    EXPECT_THROW(automaton.setLowVoltageThreshold(430), ValidationException);
    automaton.setStandby(false).setLowVoltageThreshold(370);
    EXPECT_DOUBLE_EQ(svc.getExtension<StandbyAutomaton>().getLowVoltageThreshold(), 370.0);

    Line line = *network.getLine("L12");
    OperatingStatus status = line.getExtension<OperatingStatus>();
    EXPECT_EQ(status.getStatus(), OperatingStatus::Status::PLANNED_OUTAGE);
    status.setStatus(OperatingStatus::Status::FORCED_OUTAGE);
    EXPECT_EQ(line.getExtension<OperatingStatus>().getStatus(), OperatingStatus::Status::FORCED_OUTAGE);
    line.removeExtension<OperatingStatus>();
    EXPECT_FALSE(line.hasExtension<OperatingStatus>());
    // Not on a load
    EXPECT_THROW(load.newExtension<OperatingStatusAdder>().withStatus(OperatingStatus::Status::IN_OPERATION).add(), ValidationException);
}

void checkNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/data_extensions_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/data_extensions_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    Network network = Identifiable(b.getNetworkHandle(), &b).getNetwork();
    checkGenerators(network);
    checkOthers(network);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(DataExtensionsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(DataExtensionsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("data_extensions_test");
    checkNetwork(backend);
}
#endif
