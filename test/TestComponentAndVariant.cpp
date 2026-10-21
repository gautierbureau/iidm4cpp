#include <gtest/gtest.h>
#include <iidm/Bus.h>
#include <iidm/Component.h>
#include <iidm/Network.h>
#include <iidm/PropertyCodes.h>
#include <iidm/VariantManager.h>
#include "MockBackend.h"
#include <string>
#include <tuple>
#include <vector>

using namespace iidm;
using namespace iidm::test;

// The components of the bus view and the variant manager, as the backend sees them: components
// related to a bus or listed by the network; variants changed through the manager, a clone being
// started on it, its fields set, then run by ACT_ADD.

static constexpr ObjectHandle NETWORK_HANDLE = 1;
static constexpr ObjectHandle BUS1_HANDLE = 10;
static constexpr ObjectHandle BUS2_HANDLE = 11;
static constexpr ObjectHandle MAIN_HANDLE = 20;
static constexpr ObjectHandle OTHER_HANDLE = 21;
static constexpr ObjectHandle MANAGER_HANDLE = 30;
static constexpr ObjectHandle CLONE_HANDLE = 500; // the next "adder" of the mock

class ComponentAndVariantTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.nextAdder = CLONE_HANDLE;
        backend.related[{BUS1_HANDLE, prop::NETWORK}] = NETWORK_HANDLE;
        backend.strings[{BUS1_HANDLE, prop::ID}] = "VL1_0";
        backend.strings[{BUS2_HANDLE, prop::ID}] = "VL2_0";
        backend.ints[{MAIN_HANDLE, prop::COMPONENT_NUM}] = 0;
        backend.ints[{MAIN_HANDLE, prop::COMPONENT_SIZE}] = 2;
        backend.children[{MAIN_HANDLE, prop::BUS}] = {BUS1_HANDLE, BUS2_HANDLE};
        backend.ints[{OTHER_HANDLE, prop::COMPONENT_NUM}] = 1;
        backend.related[{NETWORK_HANDLE, prop::VARIANT_MANAGER}] = MANAGER_HANDLE;
    }
};

TEST_F(ComponentAndVariantTest, Components) {
    backend.related[{BUS1_HANDLE, prop::REL_CONNECTED_COMPONENT}] = MAIN_HANDLE;
    backend.related[{BUS1_HANDLE, prop::REL_SYNCHRONOUS_COMPONENT}] = MAIN_HANDLE;
    Bus bus(BUS1_HANDLE, &backend);
    iidm::optional<Component> component = bus.getConnectedComponent();
    ASSERT_TRUE(component);
    EXPECT_EQ(component->getNum(), 0);
    EXPECT_EQ(component->getSize(), 2);
    std::vector<Bus> buses = component->getBuses();
    ASSERT_EQ(buses.size(), 2u);
    EXPECT_EQ(buses[1].getId(), "VL2_0");
    EXPECT_EQ(*bus.getSynchronousComponent(), *component);

    // In none
    EXPECT_FALSE(Bus(BUS2_HANDLE, &backend).getConnectedComponent());
    EXPECT_FALSE(Bus(BUS2_HANDLE, &backend).getSynchronousComponent());

    // Those of the network, the main one first
    backend.children[{NETWORK_HANDLE, prop::REL_CONNECTED_COMPONENT}] = {MAIN_HANDLE, OTHER_HANDLE};
    backend.children[{NETWORK_HANDLE, prop::REL_SYNCHRONOUS_COMPONENT}] = {MAIN_HANDLE};
    Network::BusView view = bus.getNetwork().getBusView();
    std::vector<Component> components = view.getConnectedComponents();
    ASSERT_EQ(components.size(), 2u);
    EXPECT_EQ(components[1].getNum(), 1);
    EXPECT_EQ(view.getSynchronousComponents().size(), 1u);
}

TEST_F(ComponentAndVariantTest, Variants) {
    backend.stringLists[{MANAGER_HANDLE, prop::VARIANT_IDS}] = {"InitialState", "V1"};
    backend.strings[{MANAGER_HANDLE, prop::WORKING_VARIANT_ID}] = "InitialState";
    VariantManager manager = Bus(BUS1_HANDLE, &backend).getNetwork().getVariantManager();
    EXPECT_EQ(manager.getVariantIds(), (std::vector<std::string>{"InitialState", "V1"}));
    EXPECT_EQ(manager.getWorkingVariantId(), VariantManager::INITIAL_VARIANT_ID);

    EXPECT_EQ(manager.setWorkingVariant("V1").getWorkingVariantId(), "V1");
    manager.removeVariant("V1");
    EXPECT_EQ((backend.strings[{MANAGER_HANDLE, prop::REMOVE_VARIANT}]), "V1");
    EXPECT_TRUE(manager.allowVariantMultiThreadAccess(true).isVariantMultiThreadAccessAllowed());

    // A clone: started on the manager, its source, its targets in order, then run
    manager.cloneVariant(VariantManager::INITIAL_VARIANT_ID, {"V2", "V3"}, true);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(MANAGER_HANDLE), prop::VARIANT_CLONE));
    typedef std::tuple<ObjectHandle, int, std::string> Set;
    std::vector<Set> cloneSets(backend.stringSets.end() - 3, backend.stringSets.end());
    EXPECT_EQ(cloneSets, (std::vector<Set>{Set(CLONE_HANDLE, prop::VARIANT_SOURCE_ID, "InitialState"),
                                           Set(CLONE_HANDLE, prop::VARIANT_TARGET_ID, "V2"),
                                           Set(CLONE_HANDLE, prop::VARIANT_TARGET_ID, "V3")}));
    EXPECT_TRUE((backend.bools[{CLONE_HANDLE, prop::VARIANT_MAY_OVERWRITE}]));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(CLONE_HANDLE), prop::ACT_ADD));

    // One target, no overwriting
    manager.cloneVariant("V2", "V4");
    EXPECT_EQ((backend.strings[{CLONE_HANDLE + 1, prop::VARIANT_TARGET_ID}]), "V4");
    EXPECT_FALSE((backend.bools[{CLONE_HANDLE + 1, prop::VARIANT_MAY_OVERWRITE}]));
}
