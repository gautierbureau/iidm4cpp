#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <string>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Subnetworks on the real backends (see RealBackends.h): the same scenario on GraalVM and on JNI,
// with the network test/data/subnetworks_test.xiidm: SUB1 (S1, VL1: G1, BL1) and SUB2 (S2, VL2: LD2,
// the unpaired boundary line BLX) linked by the root's line L12, SUB1 and SUB3 (S3, VL3: LD3, BL3)
// by the root's tie line TL13 (BL1 + BL3); the root's own substation S0 (VL0: LD0), created after.

using namespace iidm;
using iidm::test::envPath;

namespace {

using Ids = std::vector<std::string>;

template <class T>
Ids idsOf(const std::vector<T>& objects) {
    Ids ids;
    for (const T& object : objects) ids.push_back(object.getId());
    return ids;
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

void checkSubnetworks(Network& network) {
    EXPECT_EQ(idsOf(network.getSubnetworks()), (Ids{"SUB1", "SUB2", "SUB3"}));
    iidm::optional<Network> found = network.getSubnetwork("SUB2");
    ASSERT_TRUE(found);
    EXPECT_EQ(found->getNameOrId(), "SUB2"); // XIIDM keeps no name for a subnetwork
    EXPECT_EQ(found->getSourceFormat(), "test");
    EXPECT_FALSE(network.getSubnetwork("NONE"));
    std::vector<Network> subnetworks = network.getSubnetworks();
    Network& sub1 = subnetworks[0];
    Network& sub3 = subnetworks[2];
    EXPECT_EQ(*sub1.getProperty("tso"), "RTE");
    EXPECT_TRUE(sub1.getSubnetworks().empty());
    EXPECT_EQ(sub1.getParentNetwork(), network);

    // Lookups and lists: a subnetwork has its own objects only (the line between two is the root's)
    Generator g1 = *network.getGenerator("G1");
    EXPECT_EQ(g1.getParentNetwork().getId(), "SUB1");
    EXPECT_EQ(g1.getNetwork(), network);
    EXPECT_EQ(network.getLoad("LD0")->getParentNetwork(), network);
    EXPECT_TRUE(sub1.getGenerator("G1"));
    EXPECT_FALSE(subnetworks[1].getGenerator("G1"));
    EXPECT_TRUE(network.getLine("L12"));
    EXPECT_FALSE(sub1.getLine("L12"));
    EXPECT_EQ(sub1.getLineCount(), 0);
    EXPECT_EQ(network.getLineCount(), 1);
    EXPECT_EQ(idsOf(sub1.getSubstations()), (Ids{"S1"}));
    EXPECT_EQ(network.getSubstationCount(), 4);
    EXPECT_TRUE(sub1.getIdentifiable("BL1"));
    EXPECT_FALSE(sub1.getIdentifiable("LD2"));

    // Boundary elements (sorted): of a subnetwork, its boundary lines (BL1, paired in a tie line to
    // another subnetwork) rather than the tie lines, which are boundary elements too
    EXPECT_EQ(idsOf(sub1.getBoundaryElements()), (Ids{"BL1", "L12"}));
    EXPECT_EQ(idsOf(subnetworks[1].getBoundaryElements()), (Ids{"BLX", "L12"}));
    EXPECT_EQ(idsOf(network.getBoundaryElements()), (Ids{"BLX"}));
    Line l12 = *network.getLine("L12");
    EXPECT_TRUE(sub1.isBoundaryElement(l12));
    EXPECT_FALSE(sub3.isBoundaryElement(l12));
    EXPECT_FALSE(sub1.isBoundaryElement(g1));
    EXPECT_TRUE(sub1.isBoundaryElement(*network.getTieLine("TL13")));
    EXPECT_TRUE(network.isBoundaryElement(*network.getBoundaryLine("BLX")));

    // Saved alone
    std::string saved = sub1.saveToBytes();
    EXPECT_TRUE(contains(saved, "id=\"G1\""));
    EXPECT_FALSE(contains(saved, "id=\"LD2\""));
    EXPECT_TRUE(contains(network.saveToBytes(), "id=\"LD2\""));

    // A new one, which gets its own objects
    Network sub4 = network.createSubnetwork("SUB4", "Fourth", "cpp");
    EXPECT_EQ(sub4.getNameOrId(), "Fourth");
    EXPECT_EQ(sub4.getSourceFormat(), "cpp");
    Substation s4 = sub4.newSubstation().setId("S4").add();
    EXPECT_EQ(s4.getParentNetwork(), sub4);
    EXPECT_TRUE(network.getSubstation("S4"));
    EXPECT_TRUE(sub4.getSubstation("S4"));
    EXPECT_FALSE(sub1.getSubstation("S4"));
    EXPECT_EQ(network.getSubnetworks().size(), 4u);
}

void checkDetachAndFlatten(Network& network) {
    std::vector<Network> subnetworks = network.getSubnetworks();
    // The line L12 cannot be split; the tie line can
    EXPECT_FALSE(subnetworks[0].isDetachable());
    EXPECT_TRUE(subnetworks[2].isDetachable());
    EXPECT_FALSE(network.isDetachable());
    EXPECT_THROW(subnetworks[0].detach(), ValidationException);
    EXPECT_THROW(network.detach(), ValidationException);

    Network detached = subnetworks[2].detach();
    EXPECT_EQ(detached.getId(), "SUB3");
    EXPECT_TRUE(detached.getLoad("LD3"));
    ASSERT_TRUE(detached.getBoundaryLine("BL3"));
    EXPECT_FALSE(detached.getBoundaryLine("BL3")->isPaired());
    EXPECT_EQ(detached.getLoad("LD3")->getNetwork(), detached);
    EXPECT_FALSE(network.getTieLine("TL13"));
    EXPECT_FALSE(network.getBoundaryLine("BL1")->isPaired());
    EXPECT_EQ(idsOf(network.getSubnetworks()), (Ids{"SUB1", "SUB2", "SUB4"}));
    std::string saved = detached.saveToBytes();
    EXPECT_TRUE(contains(saved, "id=\"SUB3\""));
    EXPECT_FALSE(contains(saved, "id=\"G1\""));

    // Flattened: the subnetworks' objects are the network's
    EXPECT_THROW(subnetworks[0].flatten(), ValidationException);
    network.flatten();
    EXPECT_TRUE(network.getSubnetworks().empty());
    EXPECT_EQ(network.getGenerator("G1")->getParentNetwork(), network);
    EXPECT_EQ(idsOf(network.getSubstations()), (Ids{"S1", "S2", "S0", "S4"})); // in creation order
}

void checkNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/subnetworks_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/subnetworks_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    Network network = Identifiable(b.getNetworkHandle(), &b).getNetwork();
    checkSubnetworks(network);
    checkDetachAndFlatten(network);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(SubnetworksNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(SubnetworksNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("subnetworks_test");
    checkNetwork(backend);
}
#endif
