#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "MockBackend.h"
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
