#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "MockBackend.h"
#include <memory>
#include <string>
#include <tuple>
#include <vector>

// The base classes (Identifiable, Connectable, Injection, Branch, HvdcConverterStation) and the
// primitives behind them: optional values, string lists, keyed strings, object identity.

using namespace iidm;
using namespace iidm::test;

namespace {

constexpr ObjectHandle G = 10;
constexpr ObjectHandle L = 11;
constexpr ObjectHandle LINE = 12;
constexpr ObjectHandle TERMINAL = 13;

std::tuple<ObjectHandle, int, std::string> keyed(ObjectHandle h, int code, const std::string& key) {
    return std::make_tuple(h, code, key);
}

} // namespace

TEST(IdentifiableTest, Names) {
    MockBackend b;
    b.strings[{G, prop::ID}] = "G";
    b.strings[{G, prop::NAME}] = "G"; // getNameOrId
    Generator g(G, &b);
    EXPECT_EQ(g.getId(), "G");
    EXPECT_EQ(g.getNameOrId(), "G");
    EXPECT_FALSE(g.getOptionalName());
    b.optStrings[{G, prop::NAME}] = "gen";
    EXPECT_EQ(*g.getOptionalName(), "gen");
    g.setName("other");
    EXPECT_EQ((b.strings[{G, prop::NAME}]), "other");
}

TEST(IdentifiableTest, FictitiousAndType) {
    MockBackend b;
    b.ints[{G, prop::IDENTIFIABLE_TYPE}] = static_cast<int>(IdentifiableType::GENERATOR);
    Generator g(G, &b);
    EXPECT_FALSE(g.isFictitious());
    g.setFictitious(true);
    EXPECT_TRUE(g.isFictitious());
    EXPECT_EQ(g.getType(), IdentifiableType::GENERATOR);
}

TEST(IdentifiableTest, Properties) {
    MockBackend b;
    Load load(L, &b);
    EXPECT_FALSE(load.hasProperty());
    EXPECT_FALSE(load.hasProperty("k"));
    EXPECT_FALSE(load.getProperty("k"));
    EXPECT_EQ(load.getProperty("k", "default"), "default");

    load.setProperty("k", "v").setProperty("k2", "v2");
    EXPECT_EQ((b.keyedStrings[keyed(L, prop::PROPERTY, "k")]), "v");
    EXPECT_TRUE(load.hasProperty("k"));
    EXPECT_EQ(*load.getProperty("k"), "v");
    EXPECT_EQ(load.getProperty("k", "default"), "v");

    b.stringLists[{L, prop::PROPERTY_NAMES}] = {"k", "k2"};
    EXPECT_TRUE(load.hasProperty());
    EXPECT_EQ(load.getPropertyNames(), (std::vector<std::string>{"k", "k2"}));

    EXPECT_TRUE(load.removeProperty("k"));
    EXPECT_FALSE(load.hasProperty("k"));
    EXPECT_FALSE(load.removeProperty("k"));
}

TEST(IdentifiableTest, Aliases) {
    MockBackend b;
    Line line(LINE, &b);
    EXPECT_FALSE(line.hasAliases());

    line.addAlias("a1").addAlias("a2", "type2");
    EXPECT_EQ((b.keyedStrings[keyed(LINE, prop::ALIAS_TYPE, "a1")]), ""); // no type
    EXPECT_EQ((b.keyedStrings[keyed(LINE, prop::ALIAS_TYPE, "a2")]), "type2");
    line.addAlias("a3", "type3", true);
    EXPECT_EQ((b.keyedStrings[keyed(LINE, prop::UNIQUE_ALIAS_TYPE, "a3")]), "type3");
    EXPECT_EQ(*line.getAliasType("a2"), "type2");

    b.keyedStrings[keyed(LINE, prop::ALIAS_FROM_TYPE, "type2")] = "a2";
    EXPECT_EQ(*line.getAliasFromType("type2"), "a2");
    EXPECT_FALSE(line.getAliasFromType("nope"));

    b.stringLists[{LINE, prop::ALIASES}] = {"a1", "a2"};
    EXPECT_TRUE(line.hasAliases());
    EXPECT_EQ(line.getAliases(), (std::vector<std::string>{"a1", "a2"}));

    line.removeAlias("a2");
    EXPECT_FALSE(line.getAliasType("a2"));
}

TEST(IdentifiableTest, TypeChecksAndConversions) {
    MockBackend b;
    b.bools[{G, prop::GENERATOR}] = true;
    b.bools[{G, prop::INJECTION}] = true;
    b.strings[{G, prop::ID}] = "G";
    Identifiable identifiable(G, &b);
    EXPECT_TRUE(identifiable.is<Generator>());
    EXPECT_TRUE(identifiable.is<Injection>());
    EXPECT_FALSE(identifiable.is<Load>());
    EXPECT_FALSE(identifiable.is<Branch>());

    Generator g = identifiable.as<Generator>();
    EXPECT_EQ(g, identifiable);
    EXPECT_EQ(identifiable.as<Injection>().getId(), "G");
    EXPECT_THROW(identifiable.as<Load>(), PropertyNotFoundException);

    // A wrapper converts to its base classes without losing anything
    Connectable connectable = g;
    Identifiable back = connectable;
    EXPECT_EQ(back, g);
    EXPECT_EQ(b.liveHandleCount(), 1u); // one handle shared by every wrapper of the object

    // A line or a transformer converts to a Branch
    b.related[{LINE, prop::REL_TERMINAL1}] = TERMINAL;
    Line line(LINE, &b);
    Branch branch = line;
    EXPECT_EQ(branch.getTerminal1(), Terminal(TERMINAL, &b));
    EXPECT_EQ(branch.getTerminal(TwoSides::ONE), line.getTerminal1());
    std::vector<Branch> branches{line, TwoWindingsTransformer(14, &b)};
    EXPECT_EQ(branches.size(), 2u);
}

TEST(IdentifiableTest, Identity) {
    MockBackend b;
    // Same handle: same object, without asking the backend
    EXPECT_EQ(Generator(G, &b), Generator(G, &b));
    // Different handles: the backend tells (the mock compares handles)
    EXPECT_NE(Generator(G, &b), Generator(L, &b));
    // Objects that are not valid are all equal, and differ from valid ones
    EXPECT_TRUE(Generator(G, &b).isValid());
    EXPECT_FALSE(Generator().isValid());
    EXPECT_EQ(Generator(), Load());
    EXPECT_EQ(Terminal(INVALID_HANDLE, &b), Terminal());
    EXPECT_NE(Generator(), Generator(G, &b));
    // Objects of different networks are different
    MockBackend other;
    EXPECT_NE(Generator(G, &b), Generator(G, &other));
}

TEST(IdentifiableTest, NetworkLookups) {
    auto owned = std::unique_ptr<MockBackend>(new MockBackend());
    MockBackend& b = *owned;
    Network network = NetworkFactory::createFromBackend(std::move(owned));
    b.byId[{prop::IDENTIFIABLE, "G"}] = G;
    b.byId[{prop::CONNECTABLE, "G"}] = G;
    b.byId[{prop::BRANCH, "LINE"}] = LINE;
    b.byId[{prop::HVDC_CONVERTER_STATION, "VSC"}] = 15;
    b.children[{b.networkHandle, prop::IDENTIFIABLE}] = {G, L, LINE};
    b.children[{b.networkHandle, prop::CONNECTABLE}] = {G, L, LINE};
    b.children[{b.networkHandle, prop::BRANCH}] = {LINE};
    b.children[{b.networkHandle, prop::HVDC_CONVERTER_STATION}] = {15};
    b.strings[{G, prop::ID}] = "G";

    EXPECT_EQ(network.getIdentifiable("G")->getId(), "G");
    EXPECT_EQ(*network.getConnectable("G"), Generator(G, &b));
    EXPECT_EQ(*network.getBranch("LINE"), Line(LINE, &b));
    EXPECT_TRUE(static_cast<bool>(network.getHvdcConverterStation("VSC")));
    EXPECT_FALSE(network.getIdentifiable("NOPE"));
    EXPECT_EQ(network.getIdentifiables().size(), 3u);
    EXPECT_EQ(network.getConnectables().size(), 3u);
    EXPECT_EQ(network.getBranches().size(), 1u);
    EXPECT_EQ(network.getHvdcConverterStations().size(), 1u);

    // The network is an identifiable too
    b.strings[{b.networkHandle, prop::ID}] = "net";
    network.setProperty("k", "v");
    EXPECT_EQ((b.keyedStrings[keyed(b.networkHandle, prop::PROPERTY, "k")]), "v");
    EXPECT_EQ(network.getId(), "net");
}

TEST(IdentifiableTest, ConnectableAndInjection) {
    MockBackend b;
    b.children[{G, prop::TERMINAL}] = {TERMINAL};
    b.related[{G, prop::REL_TERMINAL}] = TERMINAL;
    Generator g(G, &b);
    ASSERT_EQ(g.getTerminals().size(), 1u);
    EXPECT_EQ(g.getTerminals()[0], g.getTerminal());
    g.setEquivalent(true);
    EXPECT_TRUE(g.isEquivalent());
    g.remove();
    EXPECT_EQ(b.invocations.back(), std::make_pair(G, prop::ACT_REMOVE));
}

TEST(IdentifiableTest, VoltageLevelConnectables) {
    MockBackend b;
    b.children[{20, prop::CONNECTABLE}] = {G, L};
    VoltageLevel vl(20, &b);
    std::vector<Connectable> connectables = vl.getConnectables();
    ASSERT_EQ(connectables.size(), 2u);
    EXPECT_EQ(connectables[1], Load(L, &b));
}

TEST(IdentifiableTest, OptionalValues) {
    MockBackend b;
    ShuntCompensator shunt(30, &b);
    EXPECT_FALSE(shunt.findSectionCount());
    EXPECT_FALSE(shunt.getSolvedSectionCount());
    b.optInts[{30, prop::SHUNT_SECTION_COUNT}] = 2;
    b.optInts[{30, prop::SHUNT_SOLVED_SECTION_COUNT}] = -1; // any int is a value
    EXPECT_EQ(*shunt.findSectionCount(), 2);
    EXPECT_EQ(*shunt.getSolvedSectionCount(), -1);
    shunt.setSolvedSectionCount(1);
    EXPECT_EQ((b.ints[{30, prop::SHUNT_SOLVED_SECTION_COUNT}]), 1);

    Substation s(31, &b);
    EXPECT_FALSE(s.getTso());
    b.optStrings[{31, prop::SUBSTATION_TSO}] = "";
    EXPECT_EQ(*s.getTso(), ""); // an empty TSO is not no TSO
}
