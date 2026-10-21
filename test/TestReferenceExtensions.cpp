#include <gtest/gtest.h>
#include <iidm/Generator.h>
#include <iidm/Line.h>
#include <iidm/Network.h>
#include <iidm/NetworkFactory.h>
#include <iidm/ObservabilityAreaAdder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ReferencePriorities.h>
#include <iidm/ReferenceTerminals.h>
#include <iidm/VoltageLevel.h>
#include "MockBackend.h"
#include <memory>
#include <string>
#include <tuple>
#include <vector>

using namespace iidm;
using namespace iidm::test;

// Reference priorities, reference terminals and the observability area adder as the backend sees
// them: the priority helpers evaluated (no argument for an injection, the side otherwise) and set by
// a pending change; terminals added by relation; areas as nested adders.

static constexpr ObjectHandle NETWORK_HANDLE = 1; // the mock's network
static constexpr ObjectHandle HOST_HANDLE = 10;
static constexpr ObjectHandle TERMINAL_HANDLE = 11;
static constexpr ObjectHandle EXT_HANDLE = 20;
static constexpr ObjectHandle PRIORITY_HANDLE = 21;
static constexpr ObjectHandle ADDER_HANDLE = 500; // the next "adder" of the mock

class ReferenceExtensionsTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.nextAdder = ADDER_HANDLE;
    }
};

TEST_F(ReferenceExtensionsTest, ReferencePriorities) {
    backend.related[{HOST_HANDLE, prop::REFERENCE_PRIORITIES}] = EXT_HANDLE;
    backend.children[{EXT_HANDLE, prop::REFERENCE_PRIORITY}] = {PRIORITY_HANDLE};
    backend.related[{PRIORITY_HANDLE, prop::REL_TERMINAL}] = TERMINAL_HANDLE;
    backend.ints[{PRIORITY_HANDLE, prop::EXT_RP_PRIORITY}] = 2;
    Generator g(HOST_HANDLE, &backend);
    std::vector<ReferencePriority> priorities = g.getExtension<ReferencePriorities>().getReferencePriorities();
    ASSERT_EQ(priorities.size(), 1u);
    EXPECT_EQ(priorities[0].getPriority(), 2);
    EXPECT_EQ(detail::Access::handle(priorities[0].getTerminal()), ObjectHandle(TERMINAL_HANDLE));

    // A new priority: its terminal and value
    Terminal terminal(TERMINAL_HANDLE, &backend);
    g.getExtension<ReferencePriorities>().newReferencePriority().setTerminal(terminal).setPriority(7).add();
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(EXT_HANDLE), prop::REFERENCE_PRIORITY));
    EXPECT_EQ((backend.related[{ADDER_HANDLE, prop::REL_TERMINAL}]), ObjectHandle(TERMINAL_HANDLE));
    EXPECT_EQ((backend.ints[{ADDER_HANDLE, prop::EXT_RP_PRIORITY}]), 7);
    g.getExtension<ReferencePriorities>().deleteReferencePriorities();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(EXT_HANDLE), prop::ACT_DELETE_REFERENCE_PRIORITIES));
}

TEST_F(ReferenceExtensionsTest, PriorityHelpers) {
    backend.evaluations[std::make_tuple(ObjectHandle(HOST_HANDLE), prop::EXT_RP_PRIORITY, std::vector<double>())] = 2;
    backend.evaluations[std::make_tuple(ObjectHandle(HOST_HANDLE), prop::EXT_RP_PRIORITY, std::vector<double>{1})] = 3;
    EXPECT_EQ(ReferencePriority::get(Generator(HOST_HANDLE, &backend)), 2);
    EXPECT_EQ(ReferencePriority::get(Line(HOST_HANDLE, &backend), TwoSides::TWO), 3);

    ReferencePriority::set(Line(HOST_HANDLE, &backend), TwoSides::ONE, 4);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(HOST_HANDLE), prop::EXT_RP_PRIORITY));
    EXPECT_EQ((backend.ints[{ADDER_HANDLE, prop::TERMINAL_SIDE}]), 0);
    EXPECT_EQ((backend.ints[{ADDER_HANDLE, prop::EXT_RP_PRIORITY}]), 4);
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::ACT_ADD));
    ReferencePriority::set(Generator(HOST_HANDLE, &backend), 5);
    EXPECT_EQ(backend.ints.count({ADDER_HANDLE + 1, prop::TERMINAL_SIDE}), 0u); // no side for an injection
    EXPECT_EQ((backend.ints[{ADDER_HANDLE + 1, prop::EXT_RP_PRIORITY}]), 5);
}

TEST_F(ReferenceExtensionsTest, ReferenceTerminals) {
    std::unique_ptr<MockBackend> mock(new MockBackend());
    MockBackend& b = *mock;
    b.nextAdder = ADDER_HANDLE;
    b.related[{NETWORK_HANDLE, prop::REFERENCE_TERMINALS}] = EXT_HANDLE;
    b.children[{EXT_HANDLE, prop::TERMINAL}] = {TERMINAL_HANDLE};
    Network network = NetworkFactory::createFromBackend(std::move(mock));
    ReferenceTerminals terminals = network.getExtension<ReferenceTerminals>();
    EXPECT_EQ(terminals.getReferenceTerminals().size(), 1u);
    EXPECT_EQ(ReferenceTerminals::getTerminals(network).size(), 1u);

    Terminal terminal(TERMINAL_HANDLE, &b);
    terminals.addReferenceTerminal(terminal).reset();
    EXPECT_EQ((b.related[{EXT_HANDLE, prop::ADD_REFERENCE_TERMINAL}]), ObjectHandle(TERMINAL_HANDLE));
    EXPECT_EQ(b.invocations[0], std::make_pair(ObjectHandle(EXT_HANDLE), prop::ACT_RESET_REFERENCE_TERMINALS));
    b.nextAdder = ADDER_HANDLE; // reset() counted as an adder by the mock
    terminals.setReferenceTerminals({terminal});
    EXPECT_EQ(b.invocations[1], std::make_pair(ObjectHandle(EXT_HANDLE), prop::TERMINAL));
    EXPECT_EQ((b.related[{ADDER_HANDLE, prop::ADD_REFERENCE_TERMINAL}]), ObjectHandle(TERMINAL_HANDLE));
    EXPECT_EQ(b.invocations[2], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::ACT_ADD));

    ReferenceTerminals::reset(network);
    EXPECT_EQ(b.invocations[3], std::make_pair(ObjectHandle(NETWORK_HANDLE), prop::ACT_RESET_REFERENCE_TERMINALS));
    ReferenceTerminals::addTerminal(terminal);
    EXPECT_EQ(b.invocations[4], std::make_pair(ObjectHandle(TERMINAL_HANDLE), prop::ACT_ADD_REFERENCE_TERMINAL));
}

TEST_F(ReferenceExtensionsTest, ObservabilityAreaAdder) {
    VoltageLevel vl(HOST_HANDLE, &backend);
    vl.newExtension<ObservabilityAreaAdder>()
        .withObservabilityAreaByNodes({0, 1}, 1, ObservabilityStatus::BORDER)
        .withObservabilityAreaByBusViewBus("VL_0", 2, ObservabilityStatus::OBSERVABLE);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(HOST_HANDLE), prop::OBSERVABILITY_AREA));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::AREA_CHARACTERISTICS));
    EXPECT_EQ((backend.ints[{ADDER_HANDLE + 1, prop::OBS_AREA_NUMBER}]), 1);
    EXPECT_EQ((backend.ints[{ADDER_HANDLE + 1, prop::OBS_AREA_STATUS}]), static_cast<int>(ObservabilityStatus::BORDER));
    EXPECT_EQ((backend.ints[{ADDER_HANDLE + 1, prop::OBS_AREA_NODE}]), 1); // the last node set
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(ADDER_HANDLE + 1), prop::ACT_ADD));
    EXPECT_EQ((backend.strings[{ADDER_HANDLE + 2, prop::OBS_AREA_BUS_ID}]), "VL_0");
}
