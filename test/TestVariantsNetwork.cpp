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

// Components and variants on the real backends (see RealBackends.h): the same scenario on GraalVM
// and on JNI, with the network test/data/areas_test.xiidm (VL1 - L12 - VL2 - L23 - VL3, the load L3
// on VL3).

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

void checkComponents(BackendProvider& b) {
    Line l23(b.findById(prop::LINE, "L23"), &b);
    Network network = l23.getNetwork();
    std::vector<Component> components = network.getBusView().getConnectedComponents();
    ASSERT_EQ(components.size(), 1u);
    EXPECT_EQ(components[0].getNum(), 0);
    EXPECT_EQ(components[0].getSize(), 3);
    EXPECT_EQ(idsOf(components[0].getBuses()), (Ids{"VL1_0", "VL2_0", "VL3_0"}));
    Bus b1 = *network.getBusBreakerView().getBus("B1");
    EXPECT_EQ(*b1.getConnectedComponent(), components[0]);
    EXPECT_TRUE(b1.isInMainConnectedComponent());

    // L23 disconnected: VL3 alone, the smaller component
    ASSERT_TRUE(l23.disconnect());
    components = network.getBusView().getConnectedComponents();
    ASSERT_EQ(components.size(), 2u);
    EXPECT_EQ(idsOf(components[0].getBuses()), (Ids{"VL1_0", "VL2_0"}));
    Bus vl3Bus = *network.getBusView().getBus("VL3_0");
    iidm::optional<Component> vl3Component = vl3Bus.getSynchronousComponent();
    ASSERT_TRUE(vl3Component);
    EXPECT_EQ(vl3Component->getNum(), 1);
    EXPECT_EQ(vl3Component->getSize(), 1);
    EXPECT_FALSE(vl3Bus.isInMainSynchronousComponent());
    EXPECT_EQ(network.getBusView().getSynchronousComponents().size(), 2u);
    ASSERT_TRUE(l23.connect());
    EXPECT_EQ(network.getBusView().getSynchronousComponents().size(), 1u);
}

void checkVariants(BackendProvider& b) {
    Load l3(b.findById(prop::LOAD, "L3"), &b);
    VariantManager manager = l3.getNetwork().getVariantManager();
    EXPECT_EQ(manager.getVariantIds(), (Ids{"InitialState"}));
    EXPECT_EQ(manager.getWorkingVariantId(), "InitialState");

    manager.cloneVariant(VariantManager::INITIAL_VARIANT_ID, Ids{"V2", "V1"});
    EXPECT_EQ(manager.getVariantIds(), (Ids{"InitialState", "V1", "V2"}));

    // Changed in one variant, not in the others
    manager.setWorkingVariant("V1");
    l3.setP0(99);
    EXPECT_DOUBLE_EQ(l3.getP0(), 99.0);
    manager.setWorkingVariant(VariantManager::INITIAL_VARIANT_ID);
    EXPECT_DOUBLE_EQ(l3.getP0(), 150.0);

    // Overwritten only when allowed
    EXPECT_THROW(manager.cloneVariant("InitialState", "V1"), ValidationException);
    manager.cloneVariant("InitialState", "V1", true);
    EXPECT_EQ(manager.setWorkingVariant("V1").getWorkingVariantId(), "V1");
    EXPECT_DOUBLE_EQ(l3.getP0(), 150.0);
    EXPECT_THROW(manager.setWorkingVariant("NONE"), ValidationException);
    EXPECT_THROW(manager.cloneVariant("NONE", "V3"), ValidationException);

    manager.removeVariant("V2");
    EXPECT_EQ(manager.getVariantIds(), (Ids{"InitialState", "V1"}));
    EXPECT_THROW(manager.removeVariant(VariantManager::INITIAL_VARIANT_ID), ValidationException);

    // Multi-thread access: this thread keeps its working variant
    EXPECT_FALSE(manager.isVariantMultiThreadAccessAllowed());
    EXPECT_TRUE(manager.allowVariantMultiThreadAccess(true).isVariantMultiThreadAccessAllowed());
    EXPECT_EQ(manager.getWorkingVariantId(), "V1");
    manager.allowVariantMultiThreadAccess(false);

    // The working variant removed: none until one is set
    manager.removeVariant("V1");
    EXPECT_THROW(l3.getP0(), ValidationException);
    manager.setWorkingVariant(VariantManager::INITIAL_VARIANT_ID);
    EXPECT_DOUBLE_EQ(l3.getP0(), 150.0);
}

void checkNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/areas_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/areas_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    checkComponents(b);
    checkVariants(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(VariantsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(VariantsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("variants_test");
    checkNetwork(backend);
}
#endif
