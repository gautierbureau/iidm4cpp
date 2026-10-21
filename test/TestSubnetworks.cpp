#include <gtest/gtest.h>
#include <iidm/Load.h>
#include <iidm/Network.h>
#include <iidm/NetworkFactory.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"
#include <memory>
#include <string>
#include <vector>

using namespace iidm;
using namespace iidm::test;

// Subnetworks and boundary elements, as the backend sees them: subnetworks listed and found on the
// network, created by a pending change, detached and flattened by actions; lookups by id and saves
// on the network they are called on; a boundary element check run by ACT_ADD.

static constexpr ObjectHandle ROOT_HANDLE = 1; // the mock's network
static constexpr ObjectHandle SUB1_HANDLE = 50;
static constexpr ObjectHandle SUB2_HANDLE = 51;
static constexpr ObjectHandle LOAD_HANDLE = 60;
static constexpr ObjectHandle LINE_HANDLE = 61;
static constexpr ObjectHandle PENDING_HANDLE = 500; // the next "adder" of the mock

class SubnetworksTest : public ::testing::Test {
protected:
    MockBackend* backend = nullptr;
    Network network = makeNetwork();

    Network makeNetwork() {
        std::unique_ptr<MockBackend> mock(new MockBackend());
        backend = mock.get();
        backend->nextAdder = PENDING_HANDLE;
        backend->strings[{SUB1_HANDLE, prop::ID}] = "SUB1";
        backend->strings[{SUB2_HANDLE, prop::ID}] = "SUB2";
        backend->strings[{LINE_HANDLE, prop::ID}] = "L12";
        backend->children[{ROOT_HANDLE, prop::NETWORK}] = {SUB1_HANDLE, SUB2_HANDLE};
        backend->relatedByString[std::make_tuple(ObjectHandle(ROOT_HANDLE), prop::NETWORK, std::string("SUB2"))] = SUB2_HANDLE;
        return NetworkFactory::createFromBackend(std::move(mock));
    }
};

TEST_F(SubnetworksTest, Subnetworks) {
    std::vector<Network> subnetworks = network.getSubnetworks();
    ASSERT_EQ(subnetworks.size(), 2u);
    EXPECT_EQ(subnetworks[0].getId(), "SUB1");
    iidm::optional<Network> sub2 = network.getSubnetwork("SUB2");
    ASSERT_TRUE(sub2);
    EXPECT_EQ(sub2->getId(), "SUB2");
    EXPECT_FALSE(network.getSubnetwork("NONE"));

    // Created: started on the network, its fields set, then added
    backend->addResults[PENDING_HANDLE] = SUB2_HANDLE;
    Network created = network.createSubnetwork("SUB3", "third", "fmt");
    EXPECT_EQ(backend->invocations[0], std::make_pair(ObjectHandle(ROOT_HANDLE), prop::NETWORK));
    EXPECT_EQ((backend->strings[{PENDING_HANDLE, prop::ID}]), "SUB3");
    EXPECT_EQ((backend->strings[{PENDING_HANDLE, prop::NAME}]), "third");
    EXPECT_EQ((backend->strings[{PENDING_HANDLE, prop::NETWORK_SOURCE_FORMAT}]), "fmt");
    EXPECT_EQ(backend->invocations[1], std::make_pair(ObjectHandle(PENDING_HANDLE), prop::ACT_ADD));
    EXPECT_EQ(created.getId(), "SUB2");

    // Detached, flattened
    backend->bools[{SUB1_HANDLE, prop::NETWORK_DETACHABLE}] = true;
    EXPECT_TRUE(subnetworks[0].isDetachable());
    Network detached = subnetworks[0].detach();
    EXPECT_EQ(backend->invocations[2], std::make_pair(ObjectHandle(SUB1_HANDLE), prop::ACT_DETACH));
    EXPECT_EQ(detail::Access::handle(detached), ObjectHandle(PENDING_HANDLE + 1));
    network.flatten();
    EXPECT_EQ(backend->invocations[3], std::make_pair(ObjectHandle(ROOT_HANDLE), prop::ACT_FLATTEN));
}

TEST_F(SubnetworksTest, LookupsAndSaves) {
    // On the network: found by id; on a subnetwork: among its own objects
    backend->byId[{prop::LOAD, "LD"}] = LOAD_HANDLE;
    EXPECT_TRUE(network.getLoad("LD"));
    std::vector<Network> subnetworks = network.getSubnetworks();
    Network& sub1 = subnetworks[0];
    EXPECT_FALSE(sub1.getLoad("LD"));
    backend->relatedByString[std::make_tuple(ObjectHandle(SUB1_HANDLE), prop::LOAD, std::string("LD"))] = LOAD_HANDLE;
    EXPECT_TRUE(sub1.getLoad("LD"));

    backend->bytesToReturnOnSave = "<sub1/>";
    EXPECT_EQ(sub1.saveToBytes(), "<sub1/>");
    EXPECT_EQ(backend->lastSavedNetwork, ObjectHandle(SUB1_HANDLE));
    network.save("network.xiidm");
    EXPECT_EQ(backend->lastSavedNetwork, ObjectHandle(ROOT_HANDLE));
}

TEST_F(SubnetworksTest, BoundaryElements) {
    backend->children[{SUB1_HANDLE, prop::BOUNDARY_ELEMENT}] = {LINE_HANDLE};
    std::vector<Network> subnetworks = network.getSubnetworks();
    Network& sub1 = subnetworks[0];
    std::vector<Identifiable> elements = sub1.getBoundaryElements();
    ASSERT_EQ(elements.size(), 1u);
    EXPECT_EQ(elements[0].getId(), "L12");

    // The check: started on the network, given the identifiable, then run
    backend->addResults[PENDING_HANDLE] = LINE_HANDLE;
    EXPECT_TRUE(sub1.isBoundaryElement(elements[0]));
    EXPECT_EQ(backend->invocations[0], std::make_pair(ObjectHandle(SUB1_HANDLE), prop::BOUNDARY_ELEMENT));
    EXPECT_EQ((backend->related[{PENDING_HANDLE, prop::IDENTIFIABLE}]), ObjectHandle(LINE_HANDLE));
    EXPECT_EQ(backend->invocations[1], std::make_pair(ObjectHandle(PENDING_HANDLE), prop::ACT_ADD));
    EXPECT_FALSE(network.isBoundaryElement(elements[0])); // INVALID_HANDLE from the next check
}
