#include <gtest/gtest.h>
#include <iidm/Network.h>
#include <iidm/NetworkFactory.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"
#include <memory>

using namespace iidm;
using namespace iidm::test;

// Build a Network wrapping a MockBackend for testing via the public factory method
static Network makeTestNetwork() {
    auto backend = std::make_unique<MockBackend>();
    // Network handle = 1 (the default in MockBackend)
    backend->strings[{1, prop::ID}]   = "test-network";
    backend->strings[{1, prop::NAME}]  = "Test Network";

    // Add two generators
    backend->children[{1, prop::GENERATOR}] = {10, 11};
    backend->strings[{10, prop::ID}]  = "GEN1";
    backend->strings[{11, prop::ID}]  = "GEN2";
    backend->doubles[{10, prop::GEN_TARGET_P}] = 100.0;
    backend->doubles[{11, prop::GEN_TARGET_P}] = 200.0;

    // Add a load
    backend->children[{1, prop::LOAD}] = {20};
    backend->strings[{20, prop::ID}]   = "LOAD1";
    backend->doubles[{20, prop::LOAD_P0}] = 80.0;

    // Lookup by id
    backend->byId[{prop::GENERATOR, "GEN1"}] = 10;
    backend->byId[{prop::GENERATOR, "GEN2"}] = 11;
    backend->byId[{prop::LOAD,      "LOAD1"}] = 20;

    return NetworkFactory::createFromBackend(std::move(backend));
}

TEST(NetworkTest, GetIdName) {
    auto net = makeTestNetwork();
    EXPECT_EQ(net.getId(),   "test-network");
    EXPECT_EQ(net.getName(), "Test Network");
}

TEST(NetworkTest, GetGenerators) {
    auto net = makeTestNetwork();
    auto gens = net.getGenerators();
    ASSERT_EQ(gens.size(), 2u);
    EXPECT_EQ(gens[0].getId(), "GEN1");
    EXPECT_EQ(gens[1].getId(), "GEN2");
}

TEST(NetworkTest, GetGeneratorById) {
    auto net = makeTestNetwork();
    auto gen = net.getGenerator("GEN1");
    ASSERT_TRUE(gen.has_value());
    EXPECT_DOUBLE_EQ(gen->getTargetP(), 100.0);

    auto missing = net.getGenerator("NONEXISTENT");
    EXPECT_FALSE(missing.has_value());
}

TEST(NetworkTest, GetLoads) {
    auto net = makeTestNetwork();
    auto loads = net.getLoads();
    ASSERT_EQ(loads.size(), 1u);
    EXPECT_EQ(loads[0].getId(), "LOAD1");
}

TEST(NetworkTest, GetLoadById) {
    auto net = makeTestNetwork();
    auto load = net.getLoad("LOAD1");
    ASSERT_TRUE(load.has_value());
    EXPECT_DOUBLE_EQ(load->getP0(), 80.0);
}

TEST(NetworkTest, MoveSemantics) {
    auto net1 = makeTestNetwork();
    EXPECT_EQ(net1.getId(), "test-network");
    Network net2 = std::move(net1);
    EXPECT_EQ(net2.getId(), "test-network");
}

TEST(NetworkTest, SaveDelegatesToBackend) {
    auto backend = std::make_unique<iidm::test::MockBackend>();
    backend->strings[{1, prop::ID}] = "test-network";
    auto* raw = backend.get();
    Network net = NetworkFactory::createFromBackend(std::move(backend));

    net.save("/tmp/out.xiidm");
    EXPECT_EQ(raw->lastSavedPath, "/tmp/out.xiidm");
}

TEST(NetworkTest, FactorySaveDelegatesToBackend) {
    auto backend = std::make_unique<iidm::test::MockBackend>();
    backend->strings[{1, prop::ID}] = "test-network";
    auto* raw = backend.get();
    Network net = NetworkFactory::createFromBackend(std::move(backend));

    NetworkFactory::save(net, "/tmp/factory-out.xiidm");
    EXPECT_EQ(raw->lastSavedPath, "/tmp/factory-out.xiidm");
}
