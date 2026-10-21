#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <cmath>
#include <set>
#include <sstream>
#include <string>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Networks, substations, voltage levels and switches on the real backends (see
// RealBackends.h): the same scenario on GraalVM and on JNI, with the network
// test/data/topology_test.xiidm (VL1 bus-breaker, VL2 node-breaker, in S1; VL3 in S2).

using namespace iidm;
using iidm::test::envPath;

namespace {

void checkNetwork(BackendProvider& b) {
    Identifiable root(b.getNetworkHandle(), &b);
    VoltageLevel vl1(b.findById(prop::VOLTAGE_LEVEL, "VL1"), &b);

    // The network of an object: a view of it
    Network network = vl1.getNetwork();
    EXPECT_EQ(network.getId(), "topology_test");
    EXPECT_EQ(network, root);
    EXPECT_EQ(vl1.getParentNetwork(), root);
    EXPECT_EQ(network.getContainerType(), ContainerType::NETWORK);
    EXPECT_EQ(network.getValidationLevel(), ValidationLevel::STEADY_STATE_HYPOTHESIS);
    EXPECT_EQ(network.runValidationChecks(), ValidationLevel::STEADY_STATE_HYPOTHESIS);
    EXPECT_EQ(network.getCountries(), (std::set<Country>{Country::BE, Country::FR}));
    EXPECT_EQ(network.getCountryCount(), 2);

    EXPECT_EQ(network.getSubstationCount(), 2);
    EXPECT_EQ(network.getVoltageLevelCount(), 3);
    EXPECT_EQ(network.getLoadCount(), 3);
    EXPECT_EQ(network.getGeneratorCount(), 1);
    EXPECT_EQ(network.getLineCount(), 1);
    EXPECT_EQ(network.getTwoWindingsTransformerCount(), 1);
    EXPECT_EQ(network.getBranchCount(), 2);
    EXPECT_EQ(network.getSwitchCount(), 8);
    EXPECT_EQ(network.getSwitches().size(), 8u);
    EXPECT_EQ(network.getBusbarSectionCount(), 2);
    EXPECT_EQ(network.getBusbarSections().size(), 2u);
    EXPECT_EQ(network.getBatteryCount(), 0);

    // Its views
    EXPECT_EQ(network.getBusBreakerView().getBus("B1b")->getId(), "B1b");
    EXPECT_FALSE(network.getBusBreakerView().getBus("NONE"));
    EXPECT_EQ(network.getBusBreakerView().getSwitchCount(), 1); // the bus-breaker switch of VL1
    EXPECT_GE(network.getBusBreakerView().getBusCount(), 3);
    std::vector<Bus> buses = network.getBusView().getBuses();
    EXPECT_EQ(buses.size(), 3u); // VL1 (B1 and B1b merged), VL2, VL3
    EXPECT_EQ(*network.getBusView().getBus(buses[0].getId()), buses[0]);

    // Filters
    EXPECT_EQ(network.getSubstations(Country::FR, {}).size(), 1u);
    EXPECT_EQ(network.getSubstations({}, std::string("RTE"), {"coast"})[0].getId(), "S1");
    EXPECT_TRUE(network.getSubstations(Country::BE, std::string("RTE")).empty());

    // Validation level
    network.setMinimumAcceptableValidationLevel(ValidationLevel::EQUIPMENT);
    Load l3(b.findById(prop::LOAD, "L3"), &b);
    l3.setP0(NAN);
    EXPECT_EQ(network.runValidationChecks(false), ValidationLevel::EQUIPMENT);
    EXPECT_THROW(network.runValidationChecks(true), ValidationException);
    l3.setP0(5);
    EXPECT_EQ(network.runValidationChecks(), ValidationLevel::STEADY_STATE_HYPOTHESIS);
    network.setMinimumAcceptableValidationLevel(ValidationLevel::STEADY_STATE_HYPOTHESIS);
}

void checkSubstationsAndVoltageLevels(BackendProvider& b) {
    Substation s1(b.findById(prop::SUBSTATION, "S1"), &b);
    EXPECT_EQ(s1.getContainerType(), ContainerType::SUBSTATION);
    EXPECT_EQ(s1.getGeographicalTags(), (std::vector<std::string>{"coast", "north"}));
    s1.addGeographicalTag("east").setCountry(Country::DE);
    EXPECT_EQ(s1.getGeographicalTags().size(), 3u);
    EXPECT_EQ(s1.getCountry(), iidm::optional<Country>(Country::DE));
    s1.setCountry(Country::FR);
    ASSERT_EQ(s1.getTwoWindingsTransformers().size(), 1u);
    EXPECT_EQ(s1.getTwoWindingsTransformers()[0].getId(), "T2");
    EXPECT_EQ(s1.getTwoWindingsTransformerCount(), 1);
    EXPECT_EQ(s1.getThreeWindingsTransformerCount(), 0);
    EXPECT_TRUE(s1.getThreeWindingsTransformers().empty());

    VoltageLevel vl1(b.findById(prop::VOLTAGE_LEVEL, "VL1"), &b);
    EXPECT_EQ(vl1.getContainerType(), ContainerType::VOLTAGE_LEVEL);
    EXPECT_EQ(*vl1.getSubstation(), s1);
    vl1.setNominalV(410).setLowVoltageLimit(390).setHighVoltageLimit(430);
    EXPECT_DOUBLE_EQ(vl1.getNominalV(), 410.0);
    EXPECT_EQ(vl1.getLowVoltageLimit(), iidm::optional<double>(390.0));
    EXPECT_THROW(vl1.setHighVoltageLimit(300), ValidationException); // below the low limit
    EXPECT_EQ(vl1.getConnectable("G1")->getId(), "G1");
    EXPECT_FALSE(vl1.getConnectable("L3")); // not in it
    EXPECT_EQ(vl1.getConnectableCount(), 4);
    EXPECT_EQ(vl1.getLines().size(), 1u);
    EXPECT_EQ(vl1.getLineCount(), 1);
    EXPECT_EQ(vl1.getTwoWindingsTransformers().size(), 1u);
    EXPECT_EQ(vl1.getThreeWindingsTransformerCount(), 0);
    EXPECT_EQ(vl1.getGeneratorCount(), 1);
    EXPECT_EQ(vl1.getLoadCount(), 1);
    EXPECT_EQ(vl1.getSwitchCount(), 1);

    // Bus view: the closed switch merges B1 and B1b
    ASSERT_EQ(vl1.getBusView().getBuses().size(), 1u);
    Bus merged = vl1.getBusView().getBuses()[0];
    EXPECT_EQ(*vl1.getBusView().getBus(merged.getId()), merged);
    VoltageLevel vl2(b.findById(prop::VOLTAGE_LEVEL, "VL2"), &b);
    ASSERT_TRUE(vl2.getBusView().getMergedBus("BBS1"));
    EXPECT_EQ(*vl2.getBusView().getMergedBus("BBS1"), *vl2.getBusView().getMergedBus("BBS2")); // coupled
    EXPECT_EQ(*vl1.getBusView().getMergedBus("B1b"), merged); // a configured bus
    EXPECT_THROW(vl1.getBusView().getMergedBus("BBS1"), PropertyNotFoundException); // unknown here

    Switch sw(b.findById(prop::SWITCH, "BR_C"), &b);
    EXPECT_EQ(sw.getVoltageLevel(), vl2);

    // Its topology as powsybl prints it
    std::ostringstream text;
    vl2.printTopology(text);
    EXPECT_NE(text.str().find("Topology of VL2"), std::string::npos);
    EXPECT_NE(text.str().find("BR_C"), std::string::npos);

    // Node-breaker to bus-breaker (not the other way)
    EXPECT_THROW(vl1.convertToTopology(TopologyKind::NODE_BREAKER), ValidationException);
    vl2.convertToTopology(TopologyKind::BUS_BREAKER);
    EXPECT_EQ(vl2.getTopologyKind(), TopologyKind::BUS_BREAKER);

    // Renamed
    Load l1(b.findById(prop::LOAD, "L1"), &b);
    l1.setId("L1_RENAMED");
    EXPECT_EQ(l1.getId(), "L1_RENAMED");
    EXPECT_NE(b.findById(prop::LOAD, "L1_RENAMED"), INVALID_HANDLE);
    EXPECT_THROW(l1.setId("G1"), ValidationException); // taken
}

// Network views and counts leave no Java reference behind
void checkHandlesAreReleased(BackendProvider& b) {
    VoltageLevel vl1(b.findById(prop::VOLTAGE_LEVEL, "VL1"), &b);
    const std::size_t baseline = b.liveHandleCount();
    int sum = 0;
    for (int i = 0; i < 300; ++i) {
        sum += vl1.getNetwork().getLoadCount() + vl1.getLineCount();
        sum += static_cast<int>(vl1.getBusView().getBuses().size());
        sum += vl1.getSubstation()->getTwoWindingsTransformerCount();
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    EXPECT_NE(sum, 0);
}

void checkContainers(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/topology_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/topology_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    checkNetwork(b);
    checkHandlesAreReleased(b);
    checkSubstationsAndVoltageLevels(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(ContainersNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkContainers(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(ContainersNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("containers_test");
    checkContainers(backend);
}
#endif
