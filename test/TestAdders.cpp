#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "MockBackend.h"
#include <memory>
#include <utility>

// Each adder starts with invoke(parent, TYPE), sends each field with the code of the property it
// sets, and adds the object with invoke(adder, ACT_ADD); remove() is invoke(object, ACT_REMOVE).

using namespace iidm;
using namespace iidm::test;

namespace {

std::pair<ObjectHandle, int> call(ObjectHandle h, int code) {
    return std::make_pair(h, code);
}

} // namespace

class AddersTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle VL = 20;
};

TEST_F(AddersTest, Generator) {
    static constexpr ObjectHandle G = 21;
    VoltageLevel vl(VL, &backend);
    GeneratorAdder adder = vl.newGenerator();
    const ObjectHandle a = backend.lastAdder;
    EXPECT_EQ(backend.invocations.back(), call(VL, prop::GENERATOR));
    backend.addResults[a] = G;

    Generator g = adder.setId("G").setName("gen").setBus("B").setConnectableBus("B").setNode(3)
                       .setEnergySource(EnergySource::NUCLEAR).setMinP(10).setMaxP(200).setTargetP(100)
                       .setRatedS(250).setCondenser(true).setLocalTargetV(405).setLocalTargetQ(5)
                       .setEnsureIdUnicity(true).setFictitious(true)
                       .add();

    EXPECT_EQ((backend.strings[{a, prop::ID}]), "G");
    EXPECT_EQ((backend.strings[{a, prop::NAME}]), "gen");
    EXPECT_EQ((backend.strings[{a, prop::TERMINAL_BUS_BREAKER_BUS_ID}]), "B");
    EXPECT_EQ((backend.strings[{a, prop::TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID}]), "B");
    EXPECT_EQ((backend.ints[{a, prop::TERMINAL_NODE}]), 3);
    EXPECT_EQ((backend.ints[{a, prop::GEN_ENERGY_SOURCE}]), static_cast<int>(EnergySource::NUCLEAR));
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::GEN_MIN_P}]), 10);
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::GEN_MAX_P}]), 200);
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::GEN_TARGET_P}]), 100);
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::GEN_RATED_S}]), 250);
    EXPECT_TRUE((backend.bools[{a, prop::GEN_CONDENSER}]));
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::LOCAL_TARGET_V}]), 405);
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::LOCAL_TARGET_Q}]), 5);
    EXPECT_TRUE((backend.bools[{a, prop::ENSURE_ID_UNICITY}]));
    EXPECT_TRUE((backend.bools[{a, prop::FICTITIOUS}]));
    EXPECT_EQ(backend.invocations.back(), call(a, prop::ACT_ADD));
    EXPECT_EQ(g, Generator(G, &backend));

    g.remove();
    EXPECT_EQ(backend.invocations.back(), call(G, prop::ACT_REMOVE));
}

TEST_F(AddersTest, NestedVoltageRegulationReturnsToTheGeneratorAdder) {
    static constexpr ObjectHandle TERMINAL = 22;
    VoltageLevel vl(VL, &backend);
    GeneratorAdder adder = vl.newGenerator();
    const ObjectHandle generatorAdder = backend.lastAdder;

    GeneratorAdder& same = adder.setId("G")
        .newVoltageRegulation()
            .withMode(RegulationMode::VOLTAGE_PER_REACTIVE_POWER).withRegulating(true).withSlope(0.1)
            .withTargetDeadband(2).withTargetValue(400).withTerminal(Terminal(TERMINAL, &backend))
            .add()
        .setMinP(0);
    const ObjectHandle regulationAdder = backend.lastAdder;

    EXPECT_EQ(&same, &adder);
    EXPECT_EQ(backend.invocations[1], call(generatorAdder, prop::VOLTAGE_REGULATION));
    EXPECT_EQ((backend.ints[{regulationAdder, prop::VR_MODE}]), static_cast<int>(RegulationMode::VOLTAGE_PER_REACTIVE_POWER));
    EXPECT_TRUE((backend.bools[{regulationAdder, prop::VR_REGULATING}]));
    EXPECT_DOUBLE_EQ((backend.doubles[{regulationAdder, prop::VR_SLOPE}]), 0.1);
    EXPECT_DOUBLE_EQ((backend.doubles[{regulationAdder, prop::VR_TARGET_DEADBAND}]), 2);
    EXPECT_DOUBLE_EQ((backend.doubles[{regulationAdder, prop::VR_TARGET_VALUE}]), 400);
    EXPECT_EQ((backend.related[{regulationAdder, prop::REL_TERMINAL}]), TERMINAL);
    EXPECT_EQ(backend.invocations.back(), call(regulationAdder, prop::ACT_ADD));
    EXPECT_DOUBLE_EQ((backend.doubles[{generatorAdder, prop::GEN_MIN_P}]), 0);
}

TEST_F(AddersTest, VoltageRegulationOfAnExistingGenerator) {
    static constexpr ObjectHandle G = 21;
    static constexpr ObjectHandle REGULATION = 23;
    static constexpr ObjectHandle TERMINAL = 24;
    Generator g(G, &backend);
    VoltageRegulationBuilder builder = g.newVoltageRegulation();
    const ObjectHandle b = backend.lastAdder;
    EXPECT_EQ(backend.invocations.back(), call(G, prop::VOLTAGE_REGULATION));
    backend.addResults[b] = REGULATION;
    VoltageRegulation regulation = builder.withMode(RegulationMode::REACTIVE_POWER).withRegulating(false).build();
    EXPECT_EQ(regulation, VoltageRegulation(REGULATION, &backend));

    // Terminal and target value change together, in one invoke
    regulation.setTerminal(Terminal(TERMINAL, &backend), 12.5);
    const ObjectHandle change = backend.lastAdder;
    EXPECT_EQ(backend.invocations[backend.invocations.size() - 2], call(REGULATION, prop::REL_TERMINAL));
    EXPECT_EQ((backend.related[{change, prop::REL_TERMINAL}]), TERMINAL);
    EXPECT_DOUBLE_EQ((backend.doubles[{change, prop::VR_TARGET_VALUE}]), 12.5);
    EXPECT_EQ(backend.invocations.back(), call(change, prop::ACT_ADD));
}

TEST_F(AddersTest, NetworkSubstationVoltageLevelBusLoad) {
    static constexpr ObjectHandle S = 30;
    static constexpr ObjectHandle V = 31;
    static constexpr ObjectHandle B = 32;
    static constexpr ObjectHandle L = 33;
    auto owned = std::unique_ptr<MockBackend>(new MockBackend());
    MockBackend& mock = *owned;
    Network network = NetworkFactory::createFromBackend(std::move(owned));

    SubstationAdder substationAdder = network.newSubstation();
    EXPECT_EQ(mock.invocations.back(), call(mock.networkHandle, prop::SUBSTATION));
    mock.addResults[mock.lastAdder] = S;
    Substation s = substationAdder.setId("S").setCountry(Country::FR).setTso("RTE").add();
    EXPECT_EQ((mock.strings[{mock.lastAdder, prop::COUNTRY}]), "FR");
    EXPECT_EQ((mock.strings[{mock.lastAdder, prop::SUBSTATION_TSO}]), "RTE");

    VoltageLevelAdder vlAdder = s.newVoltageLevel();
    EXPECT_EQ(mock.invocations.back(), call(S, prop::VOLTAGE_LEVEL));
    mock.addResults[mock.lastAdder] = V;
    VoltageLevel vl = vlAdder.setId("VL").setNominalV(400).setLowVoltageLimit(380).setHighVoltageLimit(420)
                             .setTopologyKind(TopologyKind::BUS_BREAKER).add();
    EXPECT_DOUBLE_EQ((mock.doubles[{mock.lastAdder, prop::VL_NOMINAL_V}]), 400);
    EXPECT_DOUBLE_EQ((mock.doubles[{mock.lastAdder, prop::VL_LOW_VOLTAGE_LIMIT}]), 380);
    EXPECT_DOUBLE_EQ((mock.doubles[{mock.lastAdder, prop::VL_HIGH_VOLTAGE_LIMIT}]), 420);
    EXPECT_EQ((mock.ints[{mock.lastAdder, prop::VL_TOPOLOGY_KIND}]), static_cast<int>(TopologyKind::BUS_BREAKER));

    BusAdder busAdder = vl.getBusBreakerView().newBus();
    EXPECT_EQ(mock.invocations.back(), call(V, prop::BUS));
    mock.addResults[mock.lastAdder] = B;
    busAdder.setId("B").add();

    LoadAdder loadAdder = vl.newLoad();
    EXPECT_EQ(mock.invocations.back(), call(V, prop::LOAD));
    mock.addResults[mock.lastAdder] = L;
    Load load = loadAdder.setId("L").setBus("B").setLoadType(LoadType::AUXILIARY).setP0(10).setQ0(2).add();
    EXPECT_EQ((mock.ints[{mock.lastAdder, prop::LOAD_TYPE}]), static_cast<int>(LoadType::AUXILIARY));
    EXPECT_DOUBLE_EQ((mock.doubles[{mock.lastAdder, prop::LOAD_P0}]), 10);
    EXPECT_DOUBLE_EQ((mock.doubles[{mock.lastAdder, prop::LOAD_Q0}]), 2);

    network.newVoltageLevel();
    EXPECT_EQ(mock.invocations.back(), call(mock.networkHandle, prop::VOLTAGE_LEVEL));

    load.remove();
    EXPECT_EQ(mock.invocations.back(), call(L, prop::ACT_REMOVE));
    mock.relatedByString[std::make_tuple(V, prop::REL_BBV_BUS, std::string("B"))] = B;
    vl.getBusBreakerView().removeBus("B");
    EXPECT_EQ(mock.invocations.back(), call(B, prop::ACT_REMOVE));
    EXPECT_THROW(vl.getBusBreakerView().removeBus("NOPE"), ValidationException);
    vl.remove();
    EXPECT_EQ(mock.invocations.back(), call(V, prop::ACT_REMOVE));
    s.remove();
    EXPECT_EQ(mock.invocations.back(), call(S, prop::ACT_REMOVE));
}

TEST_F(AddersTest, AddersAreReleased) {
    VoltageLevel vl(VL, &backend);
    {
        LoadAdder adder = vl.newLoad();
        EXPECT_EQ(backend.liveHandleCount(), 2u); // the voltage level and the adder
    }
    EXPECT_EQ(backend.liveHandleCount(), 1u);
}

TEST_F(AddersTest, NotBoundToANetwork) {
    EXPECT_THROW(VoltageLevel().newGenerator(), InvalidHandleException);
    EXPECT_THROW(Generator().remove(), InvalidHandleException);
    EXPECT_THROW(Generator().newActivePowerControl(), InvalidHandleException);
}

TEST_F(AddersTest, Injections) {
    VoltageLevel vl(VL, &backend);
    vl.newBattery().setId("BAT").setBus("B").setMinP(-50).setMaxP(50).setTargetP(10).setLocalTargetQ(0)
        .setEquivalent(true).add();
    ObjectHandle a = backend.lastAdder;
    EXPECT_EQ(backend.invocations.front(), call(VL, prop::BATTERY));
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::BAT_TARGET_P}]), 10);
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::BAT_MIN_P}]), -50);
    EXPECT_TRUE((backend.bools[{a, prop::EQUIVALENT}]));

    vl.newStaticVarCompensator().setId("SVC").setBmin(-1e-3).setBmax(1e-3).setLocalTargetV(400).add();
    a = backend.lastAdder;
    EXPECT_EQ(backend.invocations[backend.invocations.size() - 2], call(VL, prop::STATIC_VAR_COMPENSATOR));
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::SVC_B_MAX}]), 1e-3);
    EXPECT_DOUBLE_EQ((backend.doubles[{a, prop::LOCAL_TARGET_V}]), 400);

    vl.newVscConverterStation().setId("VSC").setLossFactor(1.1).setLocalTargetQ(0).add();
    EXPECT_DOUBLE_EQ((backend.doubles[{backend.lastAdder, prop::CONVERTER_STATION_LOSS_FACTOR}]), 1.1);
    vl.newLccConverterStation().setId("LCC").setLossFactor(1.2).setPowerFactor(0.8).add();
    EXPECT_DOUBLE_EQ((backend.doubles[{backend.lastAdder, prop::LCC_POWER_FACTOR}]), 0.8);
    EXPECT_EQ(backend.invocations[backend.invocations.size() - 2], call(VL, prop::LCC_CONVERTER_STATION));
}

TEST_F(AddersTest, ShuntCompensatorModels) {
    VoltageLevel vl(VL, &backend);
    backend.invocations.clear();
    vl.newShuntCompensator().setId("SC").setSectionCount(1).setSolvedSectionCount(0)
        .newLinearModel().setBPerSection(1e-5).setGPerSection(0).setMaximumSectionCount(2).add()
        .add();
    const ObjectHandle shunt = backend.lastAdder - 1; // the shunt, then its model
    EXPECT_EQ(backend.invocations[0], call(VL, prop::SHUNT_COMPENSATOR));
    EXPECT_EQ((backend.ints[{shunt, prop::SHUNT_SECTION_COUNT}]), 1);
    EXPECT_EQ((backend.ints[{shunt, prop::SHUNT_SOLVED_SECTION_COUNT}]), 0);
    EXPECT_EQ(backend.invocations[1], call(shunt, prop::SHUNT_COMPENSATOR_LINEAR_MODEL));
    EXPECT_DOUBLE_EQ((backend.doubles[{shunt + 1, prop::SHUNT_B_PER_SECTION}]), 1e-5);
    EXPECT_EQ((backend.ints[{shunt + 1, prop::SHUNT_MAX_SECTION_COUNT}]), 2);
    EXPECT_EQ(backend.invocations[2], call(shunt + 1, prop::ACT_ADD));
    EXPECT_EQ(backend.invocations[3], call(shunt, prop::ACT_ADD));

    backend.invocations.clear();
    vl.newShuntCompensator().setId("SC2").setSectionCount(2)
        .newNonLinearModel()
            .beginSection().setB(1e-5).setG(0).endSection()
            .beginSection().setB(3e-5).setG(1e-6).endSection()
            .add()
        .add();
    const ObjectHandle shunt2 = backend.lastAdder - 3; // the shunt, its model, two sections
    EXPECT_EQ(backend.invocations[1], call(shunt2, prop::SHUNT_COMPENSATOR_NON_LINEAR_MODEL));
    EXPECT_EQ(backend.invocations[2], call(shunt2 + 1, prop::SHUNT_SECTION));
    EXPECT_EQ(backend.invocations[3], call(shunt2 + 2, prop::ACT_ADD));
    EXPECT_DOUBLE_EQ((backend.doubles[{shunt2 + 3, prop::SHUNT_SECTION_G}]), 1e-6);
    EXPECT_EQ(backend.invocations[6], call(shunt2 + 1, prop::ACT_ADD));
    EXPECT_EQ(backend.invocations[7], call(shunt2, prop::ACT_ADD));
}

TEST_F(AddersTest, BoundaryLineGeneration) {
    VoltageLevel vl(VL, &backend);
    backend.invocations.clear();
    vl.newBoundaryLine().setId("BL").setP0(10).setQ0(5).setR(1).setX(10).setG(0).setB(1e-6).setPairingKey("XNODE")
        .newGeneration().setMinP(0).setMaxP(100).setTargetP(50).setTargetQ(10).setTargetV(400)
            .setVoltageRegulationOn(true).add()
        .add();
    const ObjectHandle line = backend.lastAdder - 1;
    EXPECT_EQ(backend.invocations[0], call(VL, prop::BOUNDARY_LINE));
    EXPECT_DOUBLE_EQ((backend.doubles[{line, prop::BL_B}]), 1e-6);
    EXPECT_EQ((backend.strings[{line, prop::BL_PAIRING_KEY}]), "XNODE");
    EXPECT_EQ(backend.invocations[1], call(line, prop::BOUNDARY_LINE_GENERATION));
    EXPECT_DOUBLE_EQ((backend.doubles[{line + 1, prop::GENERATION_TARGET_V}]), 400);
    EXPECT_TRUE((backend.bools[{line + 1, prop::GENERATION_VOLTAGE_REGULATION_ON}]));
    EXPECT_EQ(backend.invocations[2], call(line + 1, prop::ACT_ADD));
    EXPECT_EQ(backend.invocations[3], call(line, prop::ACT_ADD));
}

TEST_F(AddersTest, Branches) {
    static constexpr ObjectHandle NETWORK = 1; // the mock's network
    static constexpr ObjectHandle S = 30;
    std::unique_ptr<MockBackend> mock(new MockBackend());
    MockBackend& b = *mock;
    Network network = NetworkFactory::createFromBackend(std::move(mock));
    network.newLine().setId("L").setVoltageLevel1("VL1").setBus1("B1").setConnectableBus1("B1").setVoltageLevel2("VL2")
        .setNode2(4).setR(1).setX(10).setG1(0).setB1(1e-6).setG2(0).setB2(2e-6).setEquivalent(true).add();
    ObjectHandle a = b.lastAdder;
    EXPECT_EQ(b.invocations[0], call(NETWORK, prop::LINE));
    EXPECT_EQ((b.strings[{a, prop::BRANCH_VOLTAGE_LEVEL1}]), "VL1");
    EXPECT_EQ((b.strings[{a, prop::BRANCH_BUS1}]), "B1");
    EXPECT_EQ((b.strings[{a, prop::BRANCH_CONNECTABLE_BUS1}]), "B1");
    EXPECT_EQ((b.strings[{a, prop::BRANCH_VOLTAGE_LEVEL2}]), "VL2");
    EXPECT_EQ((b.ints[{a, prop::BRANCH_NODE2}]), 4);
    EXPECT_DOUBLE_EQ((b.doubles[{a, prop::LINE_B2}]), 2e-6);
    EXPECT_TRUE((b.bools[{a, prop::EQUIVALENT}]));

    Substation s(S, &b);
    s.newTwoWindingsTransformer().setId("T2").setRatedU1(400).setRatedU2(225).setRatedS(300).setR(0.5).add();
    a = b.lastAdder;
    EXPECT_EQ(b.invocations[2], call(S, prop::TWO_WINDINGS_TRANSFORMER));
    EXPECT_DOUBLE_EQ((b.doubles[{a, prop::TWO_WT_RATED_U2}]), 225);
    EXPECT_DOUBLE_EQ((b.doubles[{a, prop::TWO_WT_RATED_S}]), 300);

    b.invocations.clear();
    s.newThreeWindingsTransformer().setId("T3").setRatedU0(400)
        .newLeg1().setR(0.1).setX(5).setRatedU(400).setRatedS(200).setVoltageLevel("VL1").setBus("B1").add()
        .newLeg3().setNode(7).setVoltageLevel("VL3").add()
        .add();
    const ObjectHandle t3 = b.lastAdder - 2; // the transformer, then its legs
    EXPECT_EQ(b.invocations[0], call(S, prop::THREE_WINDINGS_TRANSFORMER));
    EXPECT_DOUBLE_EQ((b.doubles[{t3, prop::THREE_WT_RATED_U0}]), 400);
    EXPECT_EQ(b.invocations[1], call(t3, prop::THREE_WINDINGS_TRANSFORMER_LEG));
    EXPECT_EQ((b.ints[{t3 + 1, prop::LEG_SIDE}]), 0);
    EXPECT_DOUBLE_EQ((b.doubles[{t3 + 1, prop::LEG_RATED_S}]), 200);
    EXPECT_EQ((b.strings[{t3 + 1, prop::TERMINAL_VOLTAGE_LEVEL_ID}]), "VL1");
    EXPECT_EQ((b.strings[{t3 + 1, prop::TERMINAL_BUS_BREAKER_BUS_ID}]), "B1");
    EXPECT_EQ(b.invocations[2], call(t3 + 1, prop::ACT_ADD));
    EXPECT_EQ((b.ints[{t3 + 2, prop::LEG_SIDE}]), 2);
    EXPECT_EQ((b.ints[{t3 + 2, prop::TERMINAL_NODE}]), 7);
    EXPECT_EQ(b.invocations[5], call(t3, prop::ACT_ADD));

    network.newHvdcLine().setId("HVDC").setR(1).setNominalV(400).setActivePowerSetpoint(100).setMaxP(200)
        .setConvertersMode(HvdcConverterStationMode::INVERTER).setConverterStationId1("VSC1").setConverterStationId2("VSC2")
        .add();
    a = b.lastAdder;
    EXPECT_EQ(b.invocations[6], call(NETWORK, prop::HVDC_LINE));
    EXPECT_EQ((b.ints[{a, prop::HVDC_CONVERTERS_MODE}]), static_cast<int>(HvdcConverterStationMode::INVERTER));
    EXPECT_EQ((b.strings[{a, prop::HVDC_CONVERTER_STATION_ID2}]), "VSC2");
    EXPECT_DOUBLE_EQ((b.doubles[{a, prop::HVDC_ACTIVE_POWER_SETPOINT}]), 100);
}

TEST_F(AddersTest, Topology) {
    VoltageLevel vl(VL, &backend);
    vl.getNodeBreakerView().newBusbarSection().setId("BBS").setNode(0).add();
    EXPECT_EQ(backend.invocations[0], call(VL, prop::BUSBAR_SECTION));
    EXPECT_EQ((backend.ints[{backend.lastAdder, prop::TERMINAL_NODE}]), 0);

    vl.getNodeBreakerView().newBreaker().setId("BR").setNode1(0).setNode2(1).setOpen(true).setRetained(true).add();
    ObjectHandle a = backend.lastAdder;
    EXPECT_EQ(backend.invocations[2], call(VL, prop::REL_NBV_SWITCH));
    EXPECT_EQ((backend.ints[{a, prop::SW_KIND}]), static_cast<int>(SwitchKind::BREAKER));
    EXPECT_EQ((backend.ints[{a, prop::SW_NODE2}]), 1);
    EXPECT_TRUE((backend.bools[{a, prop::SW_OPEN}]));
    EXPECT_TRUE((backend.bools[{a, prop::SW_RETAINED}]));
    vl.getNodeBreakerView().newDisconnector().setId("DIS").add();
    EXPECT_EQ((backend.ints[{backend.lastAdder, prop::SW_KIND}]), static_cast<int>(SwitchKind::DISCONNECTOR));
    vl.getNodeBreakerView().newSwitch().setKind(SwitchKind::LOAD_BREAK_SWITCH).add();
    EXPECT_EQ((backend.ints[{backend.lastAdder, prop::SW_KIND}]), static_cast<int>(SwitchKind::LOAD_BREAK_SWITCH));

    vl.getNodeBreakerView().newInternalConnection().setNode1(1).setNode2(2).add();
    a = backend.lastAdder;
    EXPECT_EQ(backend.invocations[backend.invocations.size() - 2], call(VL, prop::INTERNAL_CONNECTION));
    EXPECT_EQ((backend.ints[{a, prop::IC_NODE1}]), 1);
    EXPECT_EQ((backend.ints[{a, prop::IC_NODE2}]), 2);
    EXPECT_EQ(backend.invocations.back(), call(a, prop::ACT_ADD));

    vl.getBusBreakerView().newSwitch().setId("CPL").setBus1("B1").setBus2("B2").setOpen(false).add();
    a = backend.lastAdder;
    EXPECT_EQ(backend.invocations[backend.invocations.size() - 2], call(VL, prop::REL_BBV_SWITCH));
    EXPECT_EQ((backend.strings[{a, prop::SW_BUS1}]), "B1");
    EXPECT_EQ((backend.strings[{a, prop::SW_BUS2}]), "B2");
    EXPECT_FALSE((backend.bools[{a, prop::SW_OPEN}]));
}
