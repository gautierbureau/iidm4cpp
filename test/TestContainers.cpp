#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include <iidm/NetworkFactory.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"
#include <memory>
#include <sstream>

// Networks, substations, voltage levels and switches on the mock backend: counts, validation,
// countries, geographical tags, the network of an object, the bus view, topology.

using namespace iidm;
using namespace iidm::test;

namespace {

const ObjectHandle NET = 1;        // the mock's network handle
const ObjectHandle S1 = 10, S2 = 11;
const ObjectHandle VL = 20;
const ObjectHandle LOAD = 30;
const ObjectHandle SW = 40;
const ObjectHandle BUS = 50;
const ObjectHandle VIEW = 60;      // a view of the network, from one of its objects

std::vector<double> typeArg(int type) { return {static_cast<double>(type)}; }

} // namespace

TEST(ContainersTest, NetworkCountsAndLists) {
    auto b = std::make_unique<MockBackend>();
    MockBackend& mock = *b;
    mock.evaluations[std::make_tuple(NET, prop::CHILD_COUNT, typeArg(prop::GENERATOR))] = 3;
    mock.evaluations[std::make_tuple(NET, prop::CHILD_COUNT, typeArg(prop::BUSBAR_SECTION))] = 2;
    mock.evaluations[std::make_tuple(NET, prop::CHILD_COUNT, typeArg(prop::BUS))] = 7;
    mock.evaluations[std::make_tuple(NET, prop::CHILD_COUNT, typeArg(prop::LINE))] = 0;
    mock.children[{NET, prop::SWITCH}] = {SW};
    mock.children[{NET, prop::REL_BV_BUS}] = {BUS};
    mock.relatedByString[std::make_tuple(NET, prop::REL_BBV_BUS, std::string("B1"))] = BUS;
    Network net = NetworkFactory::createFromBackend(std::move(b));
    EXPECT_EQ(net.getGeneratorCount(), 3);
    EXPECT_EQ(net.getBusbarSectionCount(), 2);
    EXPECT_EQ(net.getLineCount(), 0);
    EXPECT_EQ(net.getBusBreakerView().getBusCount(), 7);
    ASSERT_EQ(net.getSwitches().size(), 1u);
    EXPECT_EQ(net.getSwitches()[0], Switch(SW, &mock));
    EXPECT_EQ(net.getBusView().getBuses().size(), 1u);
    EXPECT_EQ(*net.getBusBreakerView().getBus("B1"), Bus(BUS, &mock));
    EXPECT_FALSE(net.getBusBreakerView().getBus("B2"));
    EXPECT_FALSE(net.getBusView().getBus("B1"));
}

TEST(ContainersTest, NetworkValidationAndCountries) {
    auto b = std::make_unique<MockBackend>();
    MockBackend& mock = *b;
    mock.ints[{NET, prop::NETWORK_VALIDATION_LEVEL}] = static_cast<int>(ValidationLevel::STEADY_STATE_HYPOTHESIS);
    mock.evaluations[std::make_tuple(NET, prop::NETWORK_VALIDATION_LEVEL, std::vector<double>{0.0})] =
        static_cast<int>(ValidationLevel::EQUIPMENT);
    mock.ints[{NET, prop::CONTAINER_TYPE}] = static_cast<int>(ContainerType::NETWORK);
    mock.stringLists[{NET, prop::NETWORK_COUNTRIES}] = {"BE", "FR"};
    Network net = NetworkFactory::createFromBackend(std::move(b));
    EXPECT_EQ(net.getValidationLevel(), ValidationLevel::STEADY_STATE_HYPOTHESIS);
    EXPECT_EQ(net.runValidationChecks(false), ValidationLevel::EQUIPMENT);
    EXPECT_EQ(&net.setMinimumAcceptableValidationLevel(ValidationLevel::EQUIPMENT), &net);
    EXPECT_EQ((mock.ints[{NET, prop::NETWORK_MIN_VALIDATION_LEVEL}]), static_cast<int>(ValidationLevel::EQUIPMENT));
    EXPECT_EQ(net.getContainerType(), ContainerType::NETWORK);
    EXPECT_EQ(net.getCountries(), (std::set<Country>{Country::BE, Country::FR}));
    EXPECT_EQ(net.getCountryCount(), 2);
}

// Filtered as powsybl does: a country, a TSO, all the tags
TEST(ContainersTest, SubstationsFilter) {
    auto b = std::make_unique<MockBackend>();
    MockBackend& mock = *b;
    mock.children[{NET, prop::SUBSTATION}] = {S1, S2};
    mock.optStrings[{S1, prop::COUNTRY}] = "FR";
    mock.optStrings[{S1, prop::SUBSTATION_TSO}] = "RTE";
    mock.stringLists[{S1, prop::SUBSTATION_GEOGRAPHICAL_TAGS}] = {"coast", "north"};
    mock.optStrings[{S2, prop::COUNTRY}] = "BE";
    Network net = NetworkFactory::createFromBackend(std::move(b));
    EXPECT_EQ(net.getSubstations(Country::FR, {}).size(), 1u);
    EXPECT_EQ(net.getSubstations({}, {}).size(), 2u);
    EXPECT_EQ(net.getSubstations({}, std::string("RTE")).size(), 1u);
    EXPECT_EQ(net.getSubstations({}, {}, {"north", "coast"}).size(), 1u);
    EXPECT_TRUE(net.getSubstations({}, {}, {"north", "south"}).empty());
    EXPECT_TRUE(net.getSubstations(Country::DE, {}).empty());
}

// The network of an object: a view, which owns nothing
TEST(ContainersTest, NetworkOfAnObject) {
    auto b = std::make_unique<MockBackend>();
    MockBackend& mock = *b;
    mock.related[{LOAD, prop::NETWORK}] = VIEW;
    mock.related[{LOAD, prop::REL_PARENT_NETWORK}] = VIEW;
    mock.strings[{VIEW, prop::ID}] = "N";
    mock.ints[{VIEW, prop::NETWORK_VALIDATION_LEVEL}] = static_cast<int>(ValidationLevel::EQUIPMENT);
    Network net = NetworkFactory::createFromBackend(std::move(b));
    Load load(LOAD, &mock);
    {
        Network view = load.getNetwork();
        EXPECT_EQ(view.getId(), "N");
        EXPECT_EQ(view.getValidationLevel(), ValidationLevel::EQUIPMENT);
        EXPECT_EQ(load.getParentNetwork().getId(), "N");
    }
    EXPECT_FALSE(mock.closed);
    EXPECT_EQ(&load.setId("L_NEW"), &load);
    EXPECT_EQ((mock.strings[{LOAD, prop::ID}]), "L_NEW");
}

TEST(ContainersTest, Substation) {
    MockBackend mock;
    mock.stringLists[{S1, prop::SUBSTATION_GEOGRAPHICAL_TAGS}] = {"north"};
    mock.children[{S1, prop::TWO_WINDINGS_TRANSFORMER}] = {70, 71};
    mock.evaluations[std::make_tuple(S1, prop::CHILD_COUNT, typeArg(prop::THREE_WINDINGS_TRANSFORMER))] = 1;
    mock.ints[{S1, prop::CONTAINER_TYPE}] = static_cast<int>(ContainerType::SUBSTATION);
    Substation s(S1, &mock);
    EXPECT_EQ(s.getGeographicalTags(), (std::vector<std::string>{"north"}));
    EXPECT_EQ(&s.addGeographicalTag("coast").setCountry(Country::DE), &s);
    EXPECT_EQ((mock.strings[{S1, prop::ADD_GEOGRAPHICAL_TAG}]), "coast");
    EXPECT_EQ((mock.strings[{S1, prop::COUNTRY}]), "DE");
    EXPECT_EQ(s.getTwoWindingsTransformers().size(), 2u);
    EXPECT_EQ(s.getThreeWindingsTransformerCount(), 1);
    EXPECT_EQ(s.getContainerType(), ContainerType::SUBSTATION);
}

TEST(ContainersTest, VoltageLevel) {
    MockBackend mock;
    mock.related[{VL, prop::REL_SUBSTATION}] = S1;
    mock.strings[{S1, prop::ID}] = "S1";
    mock.relatedByString[std::make_tuple(VL, prop::CONNECTABLE, std::string("L"))] = LOAD;
    mock.children[{VL, prop::LINE}] = {80};
    mock.evaluations[std::make_tuple(VL, prop::CHILD_COUNT, typeArg(prop::LOAD))] = 4;
    mock.strings[{VL, prop::VL_TOPOLOGY_TEXT}] = "Topology of VL\n";
    mock.children[{VL, prop::REL_BV_BUS}] = {BUS};
    mock.relatedByString[std::make_tuple(VL, prop::REL_MERGED_BUS, std::string("BBS"))] = BUS;
    VoltageLevel vl(VL, &mock);
    EXPECT_EQ(vl.getSubstation()->getId(), "S1");
    EXPECT_EQ(&vl.setNominalV(380).setLowVoltageLimit(360).setHighVoltageLimit(420), &vl);
    EXPECT_DOUBLE_EQ(vl.getNominalV(), 380.0);
    EXPECT_DOUBLE_EQ(*vl.getLowVoltageLimit(), 360.0);
    EXPECT_DOUBLE_EQ(*vl.getHighVoltageLimit(), 420.0);
    EXPECT_EQ(*vl.getConnectable("L"), Connectable(LOAD, &mock));
    EXPECT_FALSE(vl.getConnectable("X"));
    EXPECT_EQ(vl.getLines().size(), 1u);
    EXPECT_EQ(vl.getLoadCount(), 4);
    vl.convertToTopology(TopologyKind::BUS_BREAKER);
    EXPECT_EQ(vl.getTopologyKind(), TopologyKind::BUS_BREAKER);
    std::ostringstream out;
    vl.printTopology(out);
    EXPECT_EQ(out.str(), "Topology of VL\n");
    EXPECT_EQ(vl.getBusView().getBuses().size(), 1u);
    EXPECT_EQ(*vl.getBusView().getMergedBus("BBS"), Bus(BUS, &mock));
    EXPECT_FALSE(vl.getBusView().getBus("B"));
}

TEST(ContainersTest, SwitchVoltageLevel) {
    MockBackend mock;
    mock.related[{SW, prop::REL_VOLTAGE_LEVEL}] = VL;
    EXPECT_EQ(Switch(SW, &mock).getVoltageLevel(), VoltageLevel(VL, &mock));
}
