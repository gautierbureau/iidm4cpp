#include <gtest/gtest.h>
#include <iidm/NetworkFactory.h>
#include <iidm/IidmException.h>
#include <cstdlib>

// Integration tests that load the real IEEE 14-bus network via the GraalVM
// backend.  They are skipped when the required environment variables are not
// set so that the regular mock-based CI jobs are not affected.
//
// Required env vars (set by the "integration" CI job):
//   IIDM_BRIDGE_GRAALVM_LIB   – full path to libiidm_bridge.so
//   IIDM_IEEE14_NETWORK_PATH  – full path to examples/IEEE14.xiidm

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED

using namespace iidm;

class IntegrationTest : public ::testing::Test {
protected:
    static const char* libPath() { return std::getenv("IIDM_BRIDGE_GRAALVM_LIB"); }
    static const char* netPath() { return std::getenv("IIDM_IEEE14_NETWORK_PATH"); }

    static bool available() {
        return libPath() && libPath()[0] && netPath() && netPath()[0];
    }

    Network loadNetwork() {
        NetworkLoadOptions opts;
        opts.mode           = BackendMode::GRAALVM;
        opts.graalvmLibPath = libPath();
        return NetworkFactory::load(netPath(), opts);
    }
};

TEST_F(IntegrationTest, NetworkIdAndElementCounts) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    EXPECT_EQ(net.getId(), "ieee14bus");
    EXPECT_EQ(net.getGenerators().size(), 5u);
    EXPECT_EQ(net.getLoads().size(),      11u);
    EXPECT_EQ(net.getLines().size(),      17u);
}

TEST_F(IntegrationTest, Generator1Properties) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    auto gen = net.getGenerator("_GEN____1_SM");
    ASSERT_TRUE(gen.has_value()) << "Generator _GEN____1_SM not found";
    EXPECT_NEAR(gen->getTargetP(), 232.3463, 1e-3);
    EXPECT_NEAR(gen->getMinP(),   -9999.0,   1e-1);
    EXPECT_NEAR(gen->getMaxP(),    9999.0,   1e-1);
    EXPECT_TRUE(gen->isVoltageRegulatorOn());
}

TEST_F(IntegrationTest, Load10Properties) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    auto load = net.getLoad("_LOAD__10_EC");
    ASSERT_TRUE(load.has_value()) << "Load _LOAD__10_EC not found";
    EXPECT_NEAR(load->getP0(), 9.0, 1e-6);
    EXPECT_NEAR(load->getQ0(), 5.8, 1e-6);
}

TEST_F(IntegrationTest, LineProperties) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    auto line = net.getLine("_BUS___10-BUS___11-1_AC");
    ASSERT_TRUE(line.has_value()) << "Line _BUS___10-BUS___11-1_AC not found";
    EXPECT_NEAR(line->getR(), 0.156256, 1e-6);
    EXPECT_NEAR(line->getX(), 0.365778, 1e-6);
}

TEST_F(IntegrationTest, SetGeneratorTargetP) {
    if (!available()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB / IIDM_IEEE14_NETWORK_PATH not set";

    Network net = loadNetwork();

    auto gen = net.getGenerator("_GEN____2_SM");
    ASSERT_TRUE(gen.has_value());
    double original = gen->getTargetP();
    gen->setTargetP(original * 0.9);
    EXPECT_NEAR(gen->getTargetP(), original * 0.9, 1e-6);
}

#endif // IIDM_BRIDGE_GRAALVM_ENABLED
