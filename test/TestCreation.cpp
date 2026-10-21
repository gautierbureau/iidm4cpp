#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <cmath>
#include <limits>
#include <string>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif

// Creating and removing network objects on the real backends (see RealBackends.h): the same
// scenario runs on GraalVM and on JNI, from NetworkFactory::create.

using namespace iidm;
using iidm::test::envPath;

namespace {

const double NaN = std::numeric_limits<double>::quiet_NaN();

void checkCreation(Network& network) {
    EXPECT_EQ(network.getId(), "created");
    EXPECT_EQ(network.getSourceFormat(), "iidm4cpp");
    network.setCaseDate("2026-01-31T12:00Z").setForecastDistance(60);
    EXPECT_EQ(network.getCaseDate(), "2026-01-31T12:00Z");
    EXPECT_EQ(network.getForecastDistance(), 60);

    Substation s = network.newSubstation().setId("S").setCountry(Country::FR).setTso("RTE").add();
    VoltageLevel vl = s.newVoltageLevel().setId("VL").setNominalV(400).setLowVoltageLimit(380)
                       .setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl.getBusBreakerView().newBus().setId("B").add();
    // Local voltage regulation: the target is the generator's local target V
    Generator g = vl.newGenerator().setId("G").setBus("B").setConnectableBus("B")
                    .setEnergySource(EnergySource::NUCLEAR).setMinP(0).setMaxP(200).setTargetP(100)
                    .setRatedS(250).setLocalTargetV(405).setLocalTargetQ(0)
                    .newVoltageRegulation().withMode(RegulationMode::VOLTAGE).withRegulating(true).add()
                    .add();
    Load load = vl.newLoad().setId("L").setName("load").setBus("B").setP0(80).setQ0(10)
                  .setLoadType(LoadType::AUXILIARY).add();

    EXPECT_EQ(*s.getCountry(), Country::FR);
    EXPECT_EQ(*s.getTso(), "RTE");
    EXPECT_DOUBLE_EQ(vl.getNominalV(), 400);
    EXPECT_DOUBLE_EQ(*vl.getLowVoltageLimit(), 380);
    EXPECT_FALSE(vl.getHighVoltageLimit());
    EXPECT_EQ(g.getEnergySource(), EnergySource::NUCLEAR);
    EXPECT_DOUBLE_EQ(g.getTargetP(), 100);
    EXPECT_DOUBLE_EQ(g.getRatedS(), 250);
    EXPECT_FALSE(g.isCondenser());
    EXPECT_EQ(g.getTerminal().getBusBreakerView().getConnectableBus()->getId(), "B");
    EXPECT_TRUE(g.isRegulating());
    EXPECT_DOUBLE_EQ(g.getRegulatingTargetV(), 405);
    EXPECT_EQ(load.getNameOrId(), "load");
    EXPECT_EQ(load.getLoadType(), LoadType::AUXILIARY);
    ASSERT_TRUE(network.getGenerator("G"));
    EXPECT_EQ(network.getLoads().size(), 1u);

    // Voltage regulation: local → remote → local, terminal and target value together
    auto regulation = g.getVoltageRegulation();
    ASSERT_TRUE(regulation);
    EXPECT_EQ(*regulation->getMode(), RegulationMode::VOLTAGE);
    EXPECT_TRUE(std::isnan(regulation->getTargetValue()));
    EXPECT_FALSE(regulation->getTerminal());
    EXPECT_THROW(regulation->setTargetValue(398), ValidationException); // NaN without a terminal
    regulation->setTerminal(load.getTerminal(), 398);
    EXPECT_TRUE(regulation->isWithTerminal());
    EXPECT_EQ(regulation->getTerminal()->getConnectable().getId(), "L");
    EXPECT_DOUBLE_EQ(g.getRegulatingTargetV(), 398);
    EXPECT_EQ(g.getRegulatingTerminal().getConnectable().getId(), "L");
    regulation->setTerminal(Terminal(), NaN);
    EXPECT_FALSE(regulation->isWithTerminal());
    EXPECT_DOUBLE_EQ(g.getRegulatingTargetV(), 405);

    // A new regulation replaces the current one (a generator regulates Q remotely only); removal
    VoltageRegulation reactive = g.newVoltageRegulation().withMode(RegulationMode::REACTIVE_POWER)
                                  .withTerminal(load.getTerminal()).withTargetValue(-10)
                                  .withRegulating(true).build();
    EXPECT_EQ(*g.getVoltageRegulation()->getMode(), RegulationMode::REACTIVE_POWER);
    EXPECT_TRUE(reactive.isRegulating());
    EXPECT_DOUBLE_EQ(reactive.getTargetValue(), -10);
    g.removeVoltageRegulation();
    EXPECT_FALSE(g.getVoltageRegulation());
    EXPECT_FALSE(g.isRegulating());

    // Extension
    ActivePowerControl apc = g.newExtension<ActivePowerControlAdder>().withDroop(4).withParticipate(true).withMaxTargetP(150).add();
    EXPECT_TRUE(g.hasExtension<ActivePowerControl>());
    EXPECT_DOUBLE_EQ(g.getExtension<ActivePowerControl>().getDroop(), 4);
    EXPECT_DOUBLE_EQ(*apc.getMaxTargetP(), 150);
    g.removeExtension<ActivePowerControl>();
    EXPECT_FALSE(g.hasExtension<ActivePowerControl>());
    EXPECT_THROW(g.getExtension<ActivePowerControl>(), ExtensionNotFoundException);

    // Base classes: names, properties, aliases, types, identity, connection
    EXPECT_EQ(g.getNameOrId(), "G");
    EXPECT_FALSE(g.getOptionalName());
    EXPECT_EQ(*load.getOptionalName(), "load");
    g.setProperty("owner", "EDF").setProperty("\xc3\xa9t\xc3\xa9", "\xf0\x9d\x84\x9e"); // non-ASCII, a 4-byte character
    EXPECT_EQ(*g.getProperty("owner"), "EDF");
    EXPECT_EQ(*g.getProperty("\xc3\xa9t\xc3\xa9"), "\xf0\x9d\x84\x9e");
    EXPECT_EQ(g.getPropertyNames(), (std::vector<std::string>{"owner", "\xc3\xa9t\xc3\xa9"}));
    EXPECT_TRUE(g.removeProperty("owner"));
    EXPECT_FALSE(g.hasProperty("owner"));
    g.addAlias("G-alias").addAlias("G-code", "code");
    EXPECT_EQ(g.getAliases(), (std::vector<std::string>{"G-alias", "G-code"}));
    EXPECT_EQ(*g.getAliasType("G-code"), "code");
    EXPECT_FALSE(g.getAliasType("G-alias"));
    EXPECT_EQ(*g.getAliasFromType("code"), "G-code");
    EXPECT_EQ(*network.getGenerator("G-code"), g);          // found by alias; the same object
    load.addAlias("G-alias", "", true);                      // taken: made unique
    EXPECT_NE(load.getAliases()[0], "G-alias");
    EXPECT_THROW(load.addAlias("G-alias"), ValidationException);
    g.removeAlias("G-alias");
    EXPECT_FALSE(network.getIdentifiable("G-alias"));

    EXPECT_EQ(g.getType(), IdentifiableType::GENERATOR);
    Connectable owner = load.getTerminal().getConnectable();
    EXPECT_TRUE(owner.is<Load>());
    EXPECT_TRUE(owner.is<Injection>());
    EXPECT_FALSE(owner.is<Generator>());
    EXPECT_EQ(owner.as<Load>(), load);
    EXPECT_THROW(owner.as<Generator>(), PropertyNotFoundException);
    EXPECT_NE(owner, g);
    EXPECT_EQ(vl.getConnectables().size(), 2u);
    EXPECT_EQ(network.getConnectables().size(), 2u);
    EXPECT_EQ(network.getIdentifiables().size(), 6u); // network, S, VL, B, G, L
    ASSERT_TRUE(network.getIdentifiable("S"));
    EXPECT_TRUE(network.getIdentifiable("S")->is<Substation>());

    EXPECT_TRUE(load.disconnect());
    EXPECT_FALSE(load.disconnect()); // already
    EXPECT_FALSE(load.getTerminal().isConnected());
    EXPECT_TRUE(load.getTerminal().connect());
    EXPECT_TRUE(load.getTerminal().isConnected());

    // powsybl validates at add(): nothing is created
    EXPECT_THROW(vl.newGenerator().setId("G").add(), ValidationException);                        // duplicate id
    EXPECT_THROW(vl.newLoad().setP0(1).setQ0(1).add(), ValidationException);                      // no id
    EXPECT_THROW(vl.newLoad().setId("L2").setBus("NOPE").setP0(1).setQ0(1).add(), ValidationException); // unknown bus
    EXPECT_FALSE(network.getLoad("L2"));

    // Removal
    load.remove();
    EXPECT_FALSE(network.getLoad("L"));
    EXPECT_THROW(load.setP0(1), ValidationException); // powsybl rejects changes to removed objects
    g.remove();
    EXPECT_FALSE(network.getGenerator("G"));
    s.remove();
    EXPECT_TRUE(network.getSubstations().empty());
    EXPECT_TRUE(network.getVoltageLevels().empty());

    // Created objects and adders do not leak Java references
    Substation s2 = network.newSubstation().setId("S2").add();
    VoltageLevel vl2 = s2.newVoltageLevel().setId("VL2").setNominalV(225)
                         .setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl2.getBusBreakerView().newBus().setId("B2").add();
    const std::size_t baseline = network.getBackend().liveHandleCount();
    for (int i = 0; i < 2000; ++i) {
        vl2.newLoad().setId("L" + std::to_string(i)).setBus("B2").setP0(1).setQ0(0).add().remove();
    }
    EXPECT_EQ(network.getBackend().liveHandleCount(), baseline);
    EXPECT_TRUE(network.getLoads().empty());
}

// Every kind of injection, with its nested adders
void checkInjections(Network& network) {
    VoltageLevel vl = network.newSubstation().setId("S3").add()
        .newVoltageLevel().setId("VL3").setNominalV(400).setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl.getBusBreakerView().newBus().setId("B3").add();

    Battery battery = vl.newBattery().setId("BAT").setBus("B3").setMinP(-50).setMaxP(50).setTargetP(10)
        .setLocalTargetQ(0).setEquivalent(true).add();
    EXPECT_DOUBLE_EQ(battery.getTargetP(), 10);
    EXPECT_TRUE(battery.isEquivalent());

    ShuntCompensator linear = vl.newShuntCompensator().setId("SC").setBus("B3").setSectionCount(1)
        .newLinearModel().setBPerSection(1e-5).setGPerSection(0).setMaximumSectionCount(2).add()
        .add();
    EXPECT_EQ(linear.getModelType(), ShuntCompensatorModelType::LINEAR);
    EXPECT_EQ(linear.getMaximumSectionCount(), 2);
    ShuntCompensator nonLinear = vl.newShuntCompensator().setId("SC2").setBus("B3").setSectionCount(2)
        .setSolvedSectionCount(1)
        .newNonLinearModel()
            .beginSection().setB(1e-5).setG(0).endSection()
            .beginSection().setB(3e-5).setG(0).endSection()
            .add()
        .add();
    EXPECT_DOUBLE_EQ(nonLinear.getB(), 3e-5);
    EXPECT_EQ(nonLinear.getModel<ShuntCompensatorNonLinearModel>().getAllSections().size(), 2u);
    EXPECT_EQ(nonLinear.getSolvedSectionCount(), iidm::optional<int>(1));
    EXPECT_THROW(vl.newShuntCompensator().setId("SC3").setBus("B3").setSectionCount(0).add(), ValidationException); // no model

    StaticVarCompensator svc = vl.newStaticVarCompensator().setId("SVC").setBus("B3").setBmin(-1e-3).setBmax(1e-3)
        .setLocalTargetV(400)
        .newVoltageRegulation().withMode(RegulationMode::VOLTAGE).withRegulating(true).add()
        .add();
    EXPECT_DOUBLE_EQ(svc.getBmax(), 1e-3);
    EXPECT_TRUE(svc.isRegulating());

    VscConverterStation vsc = vl.newVscConverterStation().setId("VSC").setBus("B3").setLossFactor(1.1).setLocalTargetQ(0).add();
    EXPECT_NEAR(vsc.getLossFactor(), 1.1, 1e-6);
    LccConverterStation lcc = vl.newLccConverterStation().setId("LCC").setBus("B3").setLossFactor(1.2).setPowerFactor(0.8).add();
    EXPECT_NEAR(lcc.getPowerFactor(), 0.8, 1e-6);

    BoundaryLine bl = vl.newBoundaryLine().setId("BL").setBus("B3").setP0(10).setQ0(5).setR(1).setX(10).setG(0).setB(0)
        .setPairingKey("XNODE")
        .newGeneration().setMinP(0).setMaxP(100).setTargetP(50).setTargetQ(10).setTargetV(400)
            .setVoltageRegulationOn(false).add()
        .add();
    EXPECT_EQ(*bl.getPairingKey(), "XNODE");
    BoundaryLine::Generation generation = *bl.getGeneration();
    EXPECT_DOUBLE_EQ(generation.getTargetP(), 50);
    generation.newMinMaxReactiveLimits().setMinQ(-20).setMaxQ(20).add();
    EXPECT_DOUBLE_EQ(generation.getReactiveLimits<MinMaxReactiveLimits>().getMaxQ(), 20);

    EXPECT_EQ(vl.getConnectables().size(), 7u);
    for (Connectable connectable : vl.getConnectables()) connectable.remove();
    EXPECT_TRUE(vl.getConnectables().empty());
}

// Lines, transformers and HVDC lines
void checkBranches(Network& network) {
    Substation s = network.newSubstation().setId("S4").add();
    const char* const ids[] = {"VL41", "VL42", "VL43"};
    const double nominalV[] = {400, 225, 63};
    for (int i = 0; i < 3; ++i) {
        s.newVoltageLevel().setId(ids[i]).setNominalV(nominalV[i]).setTopologyKind(TopologyKind::BUS_BREAKER).add()
            .getBusBreakerView().newBus().setId(std::string(ids[i]) + "_B").add();
    }
    Line line = network.newLine().setId("L41").setVoltageLevel1("VL41").setBus1("VL41_B").setConnectableBus1("VL41_B")
        .setVoltageLevel2("VL42").setBus2("VL42_B").setR(1).setX(10).setG1(0).setB1(1e-6).setG2(0).setB2(2e-6).add();
    EXPECT_EQ(line.getTerminal2().getVoltageLevel().getId(), "VL42");
    EXPECT_DOUBLE_EQ(line.getB2(), 2e-6);
    EXPECT_THROW(network.newLine().setId("L42").setVoltageLevel1("VL41").setBus1("VL41_B").setR(1).setX(1).add(),
                 ValidationException); // no side 2

    TwoWindingsTransformer t2 = s.newTwoWindingsTransformer().setId("T41").setVoltageLevel1("VL41").setBus1("VL41_B")
        .setVoltageLevel2("VL42").setBus2("VL42_B").setR(0.5).setX(20).setG(0).setB(0).setRatedU1(400).setRatedU2(225)
        .setRatedS(300).add();
    EXPECT_EQ(t2.getRatedS(), iidm::optional<double>(300.0));
    EXPECT_EQ(*t2.getSubstation(), s);

    ThreeWindingsTransformer t3 = s.newThreeWindingsTransformer().setId("T43").setRatedU0(400)
        .newLeg1().setR(0.1).setX(5).setG(0).setB(0).setRatedU(400).setRatedS(200).setVoltageLevel("VL41").setBus("VL41_B").add()
        .newLeg2().setR(0.2).setX(6).setG(0).setB(0).setRatedU(225).setVoltageLevel("VL42").setBus("VL42_B").add()
        .newLeg3().setR(0.3).setX(7).setG(0).setB(0).setRatedU(63).setVoltageLevel("VL43").setConnectableBus("VL43_B").add()
        .add();
    EXPECT_DOUBLE_EQ(t3.getLeg3().getR(), 0.3);
    EXPECT_EQ(t3.getLeg1().getRatedS(), iidm::optional<double>(200.0));
    EXPECT_FALSE(t3.getLeg3().getTerminal().isConnected());
    EXPECT_THROW(s.newThreeWindingsTransformer().setId("T44").newLeg1().setR(1).add().add(), ValidationException);

    network.getVoltageLevel("VL41")->newVscConverterStation().setId("VSC41").setBus("VL41_B").setLossFactor(1)
        .setLocalTargetQ(0).add();
    network.getVoltageLevel("VL42")->newVscConverterStation().setId("VSC42").setBus("VL42_B").setLossFactor(1)
        .setLocalTargetQ(0).add();
    HvdcLine hvdc = network.newHvdcLine().setId("HVDC4").setR(1).setNominalV(400).setActivePowerSetpoint(100).setMaxP(200)
        .setConvertersMode(HvdcConverterStationMode::RECTIFIER).setConverterStationId1("VSC41").setConverterStationId2("VSC42")
        .add();
    EXPECT_EQ(hvdc.getConverterStation2().getId(), "VSC42");
    EXPECT_EQ(hvdc.getConvertersMode(), HvdcConverterStationMode::RECTIFIER);
    EXPECT_THROW(network.newHvdcLine().setId("HVDC5").setR(1).setNominalV(400).setActivePowerSetpoint(0).setMaxP(1)
                     .setConvertersMode(HvdcConverterStationMode::RECTIFIER).setConverterStationId1("VSC41")
                     .setConverterStationId2("NOPE").add(),
                 ValidationException);

    hvdc.remove();
    t3.remove();
    t2.remove();
    line.remove();
    s.remove();
    EXPECT_FALSE(network.getSubstation("S4"));
}

// Busbar sections, switches and internal connections: a node-breaker voltage level built from C++
void checkTopology(Network& network) {
    VoltageLevel vl = network.newVoltageLevel().setId("NB").setNominalV(225).setTopologyKind(TopologyKind::NODE_BREAKER).add();
    VoltageLevel::NodeBreakerView view = vl.getNodeBreakerView();
    BusbarSection bbs = view.newBusbarSection().setId("BBS").setNode(0).add();
    EXPECT_EQ(bbs.getTerminal().getNodeBreakerView().getNode(), 0);
    Switch breaker = view.newBreaker().setId("BR").setNode1(0).setNode2(1).setOpen(false).add();
    EXPECT_EQ(breaker.getKind(), SwitchKind::BREAKER);
    Switch disconnector = view.newDisconnector().setId("DIS").setNode1(1).setNode2(2).setRetained(false).add();
    EXPECT_EQ(disconnector.getKind(), SwitchKind::DISCONNECTOR);
    view.newSwitch().setId("LBS").setKind(SwitchKind::LOAD_BREAK_SWITCH).setNode1(0).setNode2(3).setOpen(true).add();
    view.newInternalConnection().setNode1(2).setNode2(4).add();
    Load load = vl.newLoad().setId("NBL").setNode(4).setP0(10).setQ0(1).add();
    EXPECT_EQ(view.getSwitchCount(), 3);
    EXPECT_EQ(view.getInternalConnectionCount(), 1);
    EXPECT_EQ(view.getNodesInternalConnectedTo(4), (std::vector<int>{2}));
    // The load is on the busbar section's bus, through the closed switches and the internal connection
    EXPECT_EQ(load.getTerminal().getBusView().getBus()->getId(), bbs.getTerminal().getBusView().getBus()->getId());
    EXPECT_THROW(view.newSwitch().setId("NOKIND").setNode1(5).setNode2(6).add(), ValidationException);
    EXPECT_THROW(view.newBusbarSection().setId("BBS").setNode(7).add(), ValidationException); // duplicate id

    VoltageLevel bb = network.newVoltageLevel().setId("BB").setNominalV(225).setTopologyKind(TopologyKind::BUS_BREAKER).add();
    bb.getBusBreakerView().newBus().setId("BB1").add();
    bb.getBusBreakerView().newBus().setId("BB2").add();
    Switch coupler = bb.getBusBreakerView().newSwitch().setId("CPL").setBus1("BB1").setBus2("BB2").setOpen(true).add();
    EXPECT_TRUE(coupler.isOpen());
    EXPECT_EQ(bb.getBusBreakerView().getBus2("CPL").getId(), "BB2");
    EXPECT_THROW(bb.getNodeBreakerView().newBreaker(), ValidationException); // not node-breaker

    load.remove();
    vl.remove();
    bb.remove();
}

// Copies of lines, transformers, tap changers, reactive capability curves and limits
void checkCopies(Network& network) {
    Substation s = network.newSubstation().setId("S5").add();
    s.newVoltageLevel().setId("VL51").setNominalV(400).setTopologyKind(TopologyKind::BUS_BREAKER).add()
        .getBusBreakerView().newBus().setId("VL51_B").add();
    s.newVoltageLevel().setId("VL52").setNominalV(225).setTopologyKind(TopologyKind::BUS_BREAKER).add()
        .getBusBreakerView().newBus().setId("VL52_B").add();
    Line line = network.newLine().setId("L51").setVoltageLevel1("VL51").setBus1("VL51_B").setVoltageLevel2("VL52")
        .setBus2("VL52_B").setR(1).setX(10).setG1(0).setB1(1e-6).setG2(0).setB2(1e-6).add();
    line.newOperationalLimitsGroup1("DEFAULT").newCurrentLimits().setPermanentLimit(1000).add();

    // A copy gets the operational limits of the copied line on add(); a fill, its values only
    Line copy = network.newLine(line).setId("L51_COPY").setBus1("VL51_B").setBus2("VL52_B").add();
    EXPECT_DOUBLE_EQ(copy.getX(), 10);
    EXPECT_EQ(copy.getTerminal2().getVoltageLevel().getId(), "VL52");
    EXPECT_DOUBLE_EQ(copy.getOperationalLimitsGroup1("DEFAULT")->getCurrentLimits()->getPermanentLimit(), 1000);
    LineAdder fill = network.newLine();
    Line filled = LineAdder::fillLineAdder(fill, line).setId("L51_FILL").setBus1("VL51_B").setBus2("VL52_B").add();
    EXPECT_DOUBLE_EQ(filled.getB2(), 1e-6);
    EXPECT_TRUE(filled.getOperationalLimitsGroups1().empty());

    TwoWindingsTransformer t2 = s.newTwoWindingsTransformer().setId("T51").setVoltageLevel1("VL51").setBus1("VL51_B")
        .setVoltageLevel2("VL52").setBus2("VL52_B").setR(0.5).setX(20).setG(0).setB(0).setRatedU1(400).setRatedU2(225).add();
    t2.newRatioTapChanger().setLowTapPosition(0).setTapPosition(1)
        .beginStep().setRho(0.9).setR(0).setX(0).setG(0).setB(0).endStep()
        .beginStep().setRho(1.1).setR(0).setX(0).setG(0).setB(0).endStep()
        .add();
    t2.newPhaseTapChanger().setLowTapPosition(0).setTapPosition(0)
        .setRegulationMode(PhaseTapChangerRegulationMode::CURRENT_LIMITER).setRegulationValue(500).setRegulating(false)
        .beginStep().setAlpha(5).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
        .add();
    TwoWindingsTransformer t2b = s.newTwoWindingsTransformer(t2).setId("T51_COPY").setBus1("VL51_B").setBus2("VL52_B").add();
    EXPECT_DOUBLE_EQ(t2b.getRatedU2(), 225);
    EXPECT_FALSE(t2b.hasRatioTapChanger()); // the transformer's values, not its tap changers
    t2b.newRatioTapChanger(*t2.getRatioTapChanger()).add();
    EXPECT_DOUBLE_EQ(t2b.getRatioTapChanger()->getStep(1).getRho(), 1.1);
    t2b.newPhaseTapChanger(*t2.getPhaseTapChanger()).setRegulationValue(600).add(); // a copy, then changed
    EXPECT_DOUBLE_EQ(t2b.getPhaseTapChanger()->getStep(0).getAlpha(), 5);
    EXPECT_DOUBLE_EQ(t2b.getPhaseTapChanger()->getRegulationValue(), 600);
    TwoWindingsTransformerAdder fillT = s.newTwoWindingsTransformer();
    TwoWindingsTransformerAdder::fillTwoWindingsTransformerAdder(fillT, t2).setId("T51_FILL").setBus1("VL51_B").setBus2("VL52_B");
    EXPECT_DOUBLE_EQ(fillT.add().getX(), 20);

    OperationalLimitsGroup winter = t2b.newOperationalLimitsGroup2("WINTER");
    winter.newCurrentLimits(*line.getOperationalLimitsGroup1("DEFAULT")->getCurrentLimits()).add();
    EXPECT_DOUBLE_EQ(winter.getCurrentLimits()->getPermanentLimit(), 1000);

    VoltageLevel vl = *network.getVoltageLevel("VL51");
    Generator g = vl.newGenerator().setId("G51").setBus("VL51_B").setMinP(0).setMaxP(100).setTargetP(0).setLocalTargetQ(0).add();
    ReactiveCapabilityCurve curve = g.newReactiveCapabilityCurve()
        .beginPoint().setP(0).setMinQ(-10).setMaxQ(10).endPoint()
        .beginPoint().setP(100).setMinQ(-20).setMaxQ(20).endPoint()
        .add();
    Battery battery = vl.newBattery().setId("BAT51").setBus("VL51_B").setMinP(-10).setMaxP(10).setTargetP(0).setLocalTargetQ(0).add();
    battery.newReactiveCapabilityCurve(curve).add();
    EXPECT_DOUBLE_EQ(battery.getReactiveLimits<ReactiveCapabilityCurve>().getMaxQ(100), 20);

    s.remove();
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(CreationTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    NetworkLoadOptions options;
    options.mode = BackendMode::GRAALVM;
    options.graalvmLibPath = lib;
    Network network = NetworkFactory::create("created", options);
    checkCreation(network);
    checkInjections(network);
    checkBranches(network);
    checkTopology(network);
    checkCopies(network);
    EXPECT_LT(static_cast<GraalVMBackend&>(network.getBackend()).javaHandleCount(), 100);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(CreationTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    NetworkLoadOptions options;
    options.mode = BackendMode::JNI;
    options.jniNetworkId = "creation_test";
    Network network = NetworkFactory::create("created", options);
    checkCreation(network);
    checkInjections(network);
    checkBranches(network);
    checkTopology(network);
    checkCopies(network);
}
#endif
