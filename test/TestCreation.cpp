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
    EXPECT_EQ(g.getTerminal().getBusBreakerView().getConnectableBus().getId(), "B");
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
    ActivePowerControl apc = g.newActivePowerControl().withDroop(4).withParticipate(true).withMaxTargetP(150).add();
    EXPECT_TRUE(g.hasActivePowerControl());
    EXPECT_DOUBLE_EQ(g.getActivePowerControl().getDroop(), 4);
    EXPECT_DOUBLE_EQ(*apc.getMaxTargetP(), 150);
    g.removeActivePowerControl();
    EXPECT_FALSE(g.hasActivePowerControl());
    EXPECT_THROW(g.getActivePowerControl(), ExtensionNotFoundException);

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
}
#endif
