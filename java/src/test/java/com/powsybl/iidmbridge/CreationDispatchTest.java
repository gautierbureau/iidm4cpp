package com.powsybl.iidmbridge;

import com.powsybl.commons.PowsyblException;
import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.ActivePowerControl;
import com.powsybl.iidm.network.regulation.RegulationMode;
import com.powsybl.iidm.network.regulation.VoltageRegulation;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Creation and removal through {@link PropertyDispatcher#invokeObject} (JNI) and
 * {@link PropertyDispatcher#invoke} (GraalVM): adders take the object's own property codes.
 */
class CreationDispatchTest {

    private Network network;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Creation.createNetwork("created");
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    private static Object add(Object adder) {
        return PropertyDispatcher.invokeObject(adder, ACT_ADD);
    }

    private VoltageLevel addVoltageLevel() {
        Object s = PropertyDispatcher.invokeObject(network, SUBSTATION);
        PropertyDispatcher.setString(s, ID, "S");
        PropertyDispatcher.setString(s, COUNTRY, "FR");
        PropertyDispatcher.setString(s, SUBSTATION_TSO, "RTE");
        Substation substation = (Substation) add(s);

        Object v = PropertyDispatcher.invokeObject(substation, VOLTAGE_LEVEL);
        PropertyDispatcher.setString(v, ID, "VL");
        PropertyDispatcher.setDouble(v, VL_NOMINAL_V, 400);
        PropertyDispatcher.setInt(v, VL_TOPOLOGY_KIND, TopologyKind.BUS_BREAKER.ordinal());
        VoltageLevel vl = (VoltageLevel) add(v);

        Object b = PropertyDispatcher.invokeObject(vl, BUS);
        PropertyDispatcher.setString(b, ID, "B");
        add(b);
        return vl;
    }

    private Generator addGenerator(VoltageLevel vl) {
        Object g = PropertyDispatcher.invokeObject(vl, GENERATOR);
        PropertyDispatcher.setString(g, ID, "G");
        PropertyDispatcher.setString(g, TERMINAL_BUS_BREAKER_BUS_ID, "B");
        PropertyDispatcher.setString(g, TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, "B");
        PropertyDispatcher.setDouble(g, GEN_MIN_P, 0);
        PropertyDispatcher.setDouble(g, GEN_MAX_P, 200);
        PropertyDispatcher.setDouble(g, GEN_TARGET_P, 100);
        PropertyDispatcher.setDouble(g, GEN_RATED_S, 250);
        PropertyDispatcher.setDouble(g, LOCAL_TARGET_Q, 5);
        PropertyDispatcher.setDouble(g, LOCAL_TARGET_V, 405);
        PropertyDispatcher.setInt(g, GEN_ENERGY_SOURCE, EnergySource.NUCLEAR.ordinal());
        // Nested adder: adding it returns null, the generator adder goes on. Local regulation: the
        // target is the generator's local target V
        Object vr = PropertyDispatcher.invokeObject(g, VOLTAGE_REGULATION);
        PropertyDispatcher.setInt(vr, VR_MODE, RegulationMode.VOLTAGE.ordinal());
        PropertyDispatcher.setBool(vr, VR_REGULATING, true);
        assertNull(add(vr));
        return (Generator) add(g);
    }

    @Test void createsANetworkFromScratch() {
        VoltageLevel vl = addVoltageLevel();
        Generator gen = addGenerator(vl);

        Object l = PropertyDispatcher.invokeObject(vl, LOAD);
        PropertyDispatcher.setString(l, ID, "L");
        PropertyDispatcher.setString(l, NAME, "load");
        PropertyDispatcher.setString(l, TERMINAL_BUS_BREAKER_BUS_ID, "B");
        PropertyDispatcher.setDouble(l, LOAD_P0, 80);
        PropertyDispatcher.setDouble(l, LOAD_Q0, 10);
        PropertyDispatcher.setInt(l, LOAD_TYPE, LoadType.AUXILIARY.ordinal());
        PropertyDispatcher.setBool(l, FICTITIOUS, true);
        Load load = (Load) add(l);

        Substation s = network.getSubstation("S");
        assertEquals(Country.FR, s.getCountry().orElseThrow());
        assertEquals("RTE", PropertyDispatcher.getString(s, SUBSTATION_TSO));
        assertSame(gen, network.getGenerator("G"));
        assertEquals(100.0, PropertyDispatcher.getDouble(gen, GEN_TARGET_P));
        assertEquals(250.0, PropertyDispatcher.getDouble(gen, GEN_RATED_S));
        assertEquals(5.0, PropertyDispatcher.getDouble(gen, LOCAL_TARGET_Q));
        assertEquals(EnergySource.NUCLEAR.ordinal(), PropertyDispatcher.getInt(gen, GEN_ENERGY_SOURCE));
        assertEquals("B", PropertyDispatcher.getString(gen.getTerminal(), TERMINAL_BUS_BREAKER_BUS_ID));
        VoltageRegulation regulation = (VoltageRegulation) PropertyDispatcher.getRelatedObject(gen, VOLTAGE_REGULATION);
        assertTrue(Double.isNaN(PropertyDispatcher.getDouble(regulation, VR_TARGET_VALUE)));
        assertEquals(RegulationMode.VOLTAGE.ordinal(), PropertyDispatcher.getInt(regulation, VR_MODE));
        assertTrue(PropertyDispatcher.getBool(gen, REGULATING));
        assertEquals(405.0, PropertyDispatcher.getDouble(gen, REGULATING_TARGET_V));

        assertSame(load, network.getLoad("L"));
        assertEquals("load", load.getNameOrId());
        assertEquals(LoadType.AUXILIARY, load.getLoadType());
        assertTrue(PropertyDispatcher.getBool(load, FICTITIOUS));
    }

    @Test void voltageRegulationOfAnExistingGenerator() {
        Generator gen = addGenerator(addVoltageLevel());
        PropertyDispatcher.setRelatedObject(gen, VOLTAGE_REGULATION, null);
        assertNull(PropertyDispatcher.getRelatedObject(gen, VOLTAGE_REGULATION));
        assertFalse(PropertyDispatcher.getBool(gen, REGULATING));

        // On an existing holder, powsybl gives a builder: ACT_ADD builds and returns the regulation.
        // Remote regulation: a terminal, and its target value
        Load load = gen.getTerminal().getVoltageLevel().newLoad().setId("L").setBus("B").setP0(1).setQ0(0).add();
        Object builder = PropertyDispatcher.invokeObject(gen, VOLTAGE_REGULATION);
        PropertyDispatcher.setInt(builder, VR_MODE, RegulationMode.VOLTAGE.ordinal());
        PropertyDispatcher.setRelatedObject(builder, REL_TERMINAL, load.getTerminal());
        PropertyDispatcher.setDouble(builder, VR_TARGET_VALUE, 398);
        PropertyDispatcher.setBool(builder, VR_REGULATING, true);
        Object regulation = add(builder);
        assertSame(gen.getVoltageRegulation(), regulation);
        assertEquals(398.0, gen.getVoltageRegulation().getTargetValue());
        assertTrue(PropertyDispatcher.getBool(regulation, VR_WITH_TERMINAL));
        assertSame(load.getTerminal(), PropertyDispatcher.getRelatedObject(regulation, REL_TERMINAL));
        assertSame(load.getTerminal(), PropertyDispatcher.getRelatedObject(gen, REL_REGULATING_TERMINAL));
        assertEquals(398.0, PropertyDispatcher.getDouble(gen, REGULATING_TARGET_V));

        PropertyDispatcher.setDouble(regulation, VR_TARGET_DEADBAND, 2);
        PropertyDispatcher.setBool(regulation, VR_REGULATING, false);
        assertEquals(2.0, gen.getVoltageRegulation().getTargetDeadband());
        assertFalse(PropertyDispatcher.getBool(gen, REGULATING));

        // Back to local regulation: the terminal and the target value change together
        Object toLocal = PropertyDispatcher.invokeObject(regulation, REL_TERMINAL);
        PropertyDispatcher.setRelatedObject(toLocal, REL_TERMINAL, null);
        PropertyDispatcher.setDouble(toLocal, VR_TARGET_VALUE, Double.NaN);
        assertNull(add(toLocal));
        assertFalse(PropertyDispatcher.getBool(regulation, VR_WITH_TERMINAL));
        assertNull(PropertyDispatcher.getRelatedObject(regulation, REL_TERMINAL));
    }

    @Test void localToRemoteRegulation() {
        Generator gen = addGenerator(addVoltageLevel());
        Load load = gen.getTerminal().getVoltageLevel().newLoad().setId("L").setBus("B").setP0(1).setQ0(0).add();
        Object regulation = PropertyDispatcher.getRelatedObject(gen, VOLTAGE_REGULATION);
        // Separate setters cannot do it: the target value must stay NaN without a terminal
        assertKind(BridgeErrors.VALIDATION, () -> PropertyDispatcher.setDouble(regulation, VR_TARGET_VALUE, 401));

        Object toRemote = PropertyDispatcher.invokeObject(regulation, REL_TERMINAL);
        PropertyDispatcher.setRelatedObject(toRemote, REL_TERMINAL, load.getTerminal());
        PropertyDispatcher.setDouble(toRemote, VR_TARGET_VALUE, 401);
        add(toRemote);
        assertSame(load.getTerminal(), gen.getVoltageRegulation().getTerminal());
        assertEquals(401.0, gen.getVoltageRegulation().getTargetValue());
        assertEquals(401.0, PropertyDispatcher.getDouble(gen, REGULATING_TARGET_V));
    }

    @Test void activePowerControlNewAndRemove() {
        Generator gen = addGenerator(addVoltageLevel());
        assertNull(PropertyDispatcher.getRelatedObject(gen, ACTIVE_POWER_CONTROL));

        Object a = PropertyDispatcher.invokeObject(gen, ACTIVE_POWER_CONTROL);
        PropertyDispatcher.setDouble(a, EXT_APC_DROOP, 4);
        PropertyDispatcher.setBool(a, EXT_APC_PARTICIPATE, true);
        PropertyDispatcher.setDouble(a, EXT_APC_MAX_TARGET_P, 150);
        Object apc = add(a);
        assertSame(gen.getExtension(ActivePowerControl.class), apc);
        assertSame(apc, PropertyDispatcher.getRelatedObject(gen, ACTIVE_POWER_CONTROL));
        assertEquals(4.0, PropertyDispatcher.getDouble(apc, EXT_APC_DROOP));
        assertTrue(PropertyDispatcher.getBool(apc, EXT_APC_PARTICIPATE));

        PropertyDispatcher.setRelatedObject(gen, ACTIVE_POWER_CONTROL, null);
        assertNull(gen.getExtension(ActivePowerControl.class));
    }

    @Test void removesIdentifiables() {
        VoltageLevel vl = addVoltageLevel();
        Generator gen = addGenerator(vl);
        assertNull(PropertyDispatcher.invokeObject(gen, ACT_REMOVE));
        assertNull(network.getGenerator("G"));

        Bus bus = vl.getBusBreakerView().getBus("B");
        PropertyDispatcher.invokeObject(bus, ACT_REMOVE);
        assertNull(vl.getBusBreakerView().getBus("B"));

        PropertyDispatcher.invokeObject(network.getSubstation("S"), ACT_REMOVE);
        assertNull(network.getVoltageLevel("VL"));
        assertEquals(0, network.getSubstationCount());
    }

    @Test void handleApiForGraalVM() {
        long net = NetworkRegistry.register(network);
        long s = PropertyDispatcher.invoke(net, SUBSTATION);
        PropertyDispatcher.setString(s, ID, "S");
        long substation = PropertyDispatcher.invoke(s, ACT_ADD);
        assertEquals("S", PropertyDispatcher.getString(substation, ID));
        assertEquals(0L, PropertyDispatcher.invoke(substation, ACT_REMOVE));
        assertNull(network.getSubstation("S"));
    }

    @Test void errors() {
        VoltageLevel vl = addVoltageLevel();
        addGenerator(vl);

        // Missing or duplicate id, invalid value: rejected by powsybl at add()
        Object noId = PropertyDispatcher.invokeObject(vl, LOAD);
        PropertyDispatcher.setDouble(noId, LOAD_P0, 1);
        PropertyDispatcher.setDouble(noId, LOAD_Q0, 1);
        assertKind(BridgeErrors.VALIDATION, () -> add(noId));
        Object duplicate = PropertyDispatcher.invokeObject(vl, GENERATOR);
        PropertyDispatcher.setString(duplicate, ID, "G");
        assertKind(BridgeErrors.VALIDATION, () -> add(duplicate));
        Object badBus = PropertyDispatcher.invokeObject(vl, LOAD);
        PropertyDispatcher.setString(badBus, ID, "L");
        PropertyDispatcher.setString(badBus, TERMINAL_BUS_BREAKER_BUS_ID, "NOPE");
        PropertyDispatcher.setDouble(badBus, LOAD_P0, 1);
        PropertyDispatcher.setDouble(badBus, LOAD_Q0, 1);
        assertKind(BridgeErrors.VALIDATION, () -> add(badBus));
        assertNull(network.getLoad("L"));

        // A field the adder does not have, nothing to create, not an adder
        Object load = PropertyDispatcher.invokeObject(vl, LOAD);
        assertKind(BridgeErrors.PROPERTY_NOT_FOUND, () -> PropertyDispatcher.setDouble(load, GEN_TARGET_P, 1));
        assertKind(BridgeErrors.PROPERTY_NOT_FOUND, () -> PropertyDispatcher.invokeObject(network.getGenerator("G"), GENERATOR));
        assertKind(BridgeErrors.PROPERTY_NOT_FOUND, () -> PropertyDispatcher.invokeObject(vl, 999999));
        assertKind(BridgeErrors.PROPERTY_NOT_FOUND, () -> add(vl));
        assertKind(BridgeErrors.VALIDATION, () -> PropertyDispatcher.setInt(load, LOAD_TYPE, 99));

        // A voltage level with a branch cannot be removed: IllegalStateException in powsybl
        VoltageLevel vl2 = network.getSubstation("S").newVoltageLevel().setId("VL2").setNominalV(225)
            .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl2.getBusBreakerView().newBus().setId("B2").add();
        network.newLine().setId("LINE").setVoltageLevel1("VL").setBus1("B").setVoltageLevel2("VL2").setBus2("B2")
            .setR(1).setX(1).add();
        assertKind(BridgeErrors.VALIDATION, () -> PropertyDispatcher.invokeObject(vl2, ACT_REMOVE));
        assertNotNull(network.getVoltageLevel("VL2"));
    }

    private static void assertKind(int kind, Executable call) {
        Throwable t = assertThrows(Throwable.class, call::run);
        assertEquals(kind, BridgeErrors.kindOf(t), () -> "unexpected " + t);
    }

    @FunctionalInterface
    private interface Executable {
        void run() throws Throwable;
    }

    @Test void kindOfPowsyblRefusals() {
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(new IllegalStateException("refused")));
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(new PowsyblException("rejected")));
    }
}
