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

// ── Component collection getters ──────────────────────────────────────────────

TEST(NetworkTest, GetSubstations) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::SUBSTATION}] = {10, 11};
    b->strings[{10, prop::ID}] = "SUB1";
    b->strings[{11, prop::ID}] = "SUB2";
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto subs = net.getSubstations();
    ASSERT_EQ(subs.size(), 2u);
    EXPECT_EQ(subs[0].getId(), "SUB1");
    EXPECT_EQ(subs[1].getId(), "SUB2");
}

TEST(NetworkTest, GetVoltageLevels) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::VOLTAGE_LEVEL}] = {20};
    b->strings[{20, prop::ID}] = "VL1";
    b->doubles[{20, prop::VL_NOMINAL_V}] = 400.0;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto vls = net.getVoltageLevels();
    ASSERT_EQ(vls.size(), 1u);
    EXPECT_EQ(vls[0].getId(), "VL1");
}

TEST(NetworkTest, GetLines) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::LINE}] = {30};
    b->strings[{30, prop::ID}] = "LINE1";
    b->doubles[{30, prop::LINE_R}] = 0.5;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto lines = net.getLines();
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0].getId(), "LINE1");
}

TEST(NetworkTest, GetTwoWindingsTransformers) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::TWO_WINDINGS_TRANSFORMER}] = {40};
    b->strings[{40, prop::ID}] = "TWT1";
    b->doubles[{40, prop::TWO_WT_R}] = 1.0;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto twts = net.getTwoWindingsTransformers();
    ASSERT_EQ(twts.size(), 1u);
    EXPECT_EQ(twts[0].getId(), "TWT1");
}

TEST(NetworkTest, GetThreeWindingsTransformers) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::THREE_WINDINGS_TRANSFORMER}] = {50};
    b->strings[{50, prop::ID}] = "3WT1";
    b->doubles[{50, prop::THREE_WT_RATED_U0}] = 110.0;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto twts = net.getThreeWindingsTransformers();
    ASSERT_EQ(twts.size(), 1u);
    EXPECT_EQ(twts[0].getId(), "3WT1");
}

TEST(NetworkTest, GetHvdcLines) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::HVDC_LINE}] = {60};
    b->strings[{60, prop::ID}] = "HVDC1";
    b->doubles[{60, prop::HVDC_R}] = 1.0;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto hvdcs = net.getHvdcLines();
    ASSERT_EQ(hvdcs.size(), 1u);
    EXPECT_EQ(hvdcs[0].getId(), "HVDC1");
}

TEST(NetworkTest, GetDanglingLines) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::DANGLING_LINE}] = {70};
    b->strings[{70, prop::ID}] = "DL1";
    b->doubles[{70, prop::DL_P0}] = 50.0;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto dls = net.getDanglingLines();
    ASSERT_EQ(dls.size(), 1u);
    EXPECT_EQ(dls[0].getId(), "DL1");
}

TEST(NetworkTest, GetShuntCompensators) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::SHUNT_COMPENSATOR}] = {80};
    b->strings[{80, prop::ID}] = "SC1";
    b->ints[{80, prop::SHUNT_SECTION_COUNT}] = 1;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto scs = net.getShuntCompensators();
    ASSERT_EQ(scs.size(), 1u);
    EXPECT_EQ(scs[0].getId(), "SC1");
}

TEST(NetworkTest, GetStaticVarCompensators) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::STATIC_VAR_COMPENSATOR}] = {90};
    b->strings[{90, prop::ID}] = "SVC1";
    b->doubles[{90, prop::SVC_B_MIN}] = -0.1;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto svcs = net.getStaticVarCompensators();
    ASSERT_EQ(svcs.size(), 1u);
    EXPECT_EQ(svcs[0].getId(), "SVC1");
}

TEST(NetworkTest, GetVscConverterStations) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::VSC_CONVERTER_STATION}] = {100};
    b->strings[{100, prop::ID}] = "VSC1";
    b->doubles[{100, prop::VSC_LOSS_FACTOR}] = 0.01f;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto vscs = net.getVscConverterStations();
    ASSERT_EQ(vscs.size(), 1u);
    EXPECT_EQ(vscs[0].getId(), "VSC1");
}

TEST(NetworkTest, GetLccConverterStations) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::LCC_CONVERTER_STATION}] = {110};
    b->strings[{110, prop::ID}] = "LCC1";
    b->doubles[{110, prop::LCC_LOSS_FACTOR}] = 0.02f;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto lccs = net.getLccConverterStations();
    ASSERT_EQ(lccs.size(), 1u);
    EXPECT_EQ(lccs[0].getId(), "LCC1");
}

TEST(NetworkTest, GetBatteries) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->children[{1, prop::BATTERY}] = {120};
    b->strings[{120, prop::ID}] = "BAT1";
    b->doubles[{120, prop::BAT_TARGET_P}] = 50.0;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto bats = net.getBatteries();
    ASSERT_EQ(bats.size(), 1u);
    EXPECT_EQ(bats[0].getId(), "BAT1");
}

// ── findById getters ──────────────────────────────────────────────────────────

TEST(NetworkTest, GetLineById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{30, prop::ID}] = "LINE1";
    b->doubles[{30, prop::LINE_R}] = 0.5;
    b->byId[{prop::LINE, "LINE1"}] = 30;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    auto line = net.getLine("LINE1");
    ASSERT_TRUE(line.has_value());
    EXPECT_EQ(line->getId(), "LINE1");
    EXPECT_FALSE(net.getLine("MISSING").has_value());
}

TEST(NetworkTest, GetTwoWindingsTransformerById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{40, prop::ID}] = "TWT1";
    b->byId[{prop::TWO_WINDINGS_TRANSFORMER, "TWT1"}] = 40;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getTwoWindingsTransformer("TWT1").has_value());
    EXPECT_FALSE(net.getTwoWindingsTransformer("MISSING").has_value());
}

TEST(NetworkTest, GetThreeWindingsTransformerById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{50, prop::ID}] = "3WT1";
    b->byId[{prop::THREE_WINDINGS_TRANSFORMER, "3WT1"}] = 50;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getThreeWindingsTransformer("3WT1").has_value());
    EXPECT_FALSE(net.getThreeWindingsTransformer("MISSING").has_value());
}

TEST(NetworkTest, GetHvdcLineById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{60, prop::ID}] = "HVDC1";
    b->byId[{prop::HVDC_LINE, "HVDC1"}] = 60;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getHvdcLine("HVDC1").has_value());
    EXPECT_FALSE(net.getHvdcLine("MISSING").has_value());
}

TEST(NetworkTest, GetDanglingLineById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{70, prop::ID}] = "DL1";
    b->byId[{prop::DANGLING_LINE, "DL1"}] = 70;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getDanglingLine("DL1").has_value());
    EXPECT_FALSE(net.getDanglingLine("MISSING").has_value());
}

TEST(NetworkTest, GetShuntCompensatorById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{80, prop::ID}] = "SC1";
    b->byId[{prop::SHUNT_COMPENSATOR, "SC1"}] = 80;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getShuntCompensator("SC1").has_value());
    EXPECT_FALSE(net.getShuntCompensator("MISSING").has_value());
}

TEST(NetworkTest, GetStaticVarCompensatorById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{90, prop::ID}] = "SVC1";
    b->byId[{prop::STATIC_VAR_COMPENSATOR, "SVC1"}] = 90;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getStaticVarCompensator("SVC1").has_value());
    EXPECT_FALSE(net.getStaticVarCompensator("MISSING").has_value());
}

TEST(NetworkTest, GetVscConverterStationById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{100, prop::ID}] = "VSC1";
    b->byId[{prop::VSC_CONVERTER_STATION, "VSC1"}] = 100;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getVscConverterStation("VSC1").has_value());
    EXPECT_FALSE(net.getVscConverterStation("MISSING").has_value());
}

TEST(NetworkTest, GetLccConverterStationById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{110, prop::ID}] = "LCC1";
    b->byId[{prop::LCC_CONVERTER_STATION, "LCC1"}] = 110;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getLccConverterStation("LCC1").has_value());
    EXPECT_FALSE(net.getLccConverterStation("MISSING").has_value());
}

TEST(NetworkTest, GetBatteryById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{120, prop::ID}] = "BAT1";
    b->byId[{prop::BATTERY, "BAT1"}] = 120;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getBattery("BAT1").has_value());
    EXPECT_FALSE(net.getBattery("MISSING").has_value());
}

TEST(NetworkTest, GetSwitchById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{130, prop::ID}] = "SW1";
    b->bools[{130, prop::SW_OPEN}] = false;
    b->byId[{prop::SWITCH, "SW1"}] = 130;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getSwitch("SW1").has_value());
    EXPECT_FALSE(net.getSwitch("MISSING").has_value());
}

TEST(NetworkTest, GetBusbarSectionById) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "N";
    b->strings[{140, prop::ID}] = "BBS1";
    b->doubles[{140, prop::BBS_V}] = 400.0;
    b->byId[{prop::BUSBAR_SECTION, "BBS1"}] = 140;
    auto net = NetworkFactory::createFromBackend(std::move(b));
    ASSERT_TRUE(net.getBusbarSection("BBS1").has_value());
    EXPECT_FALSE(net.getBusbarSection("MISSING").has_value());
}
