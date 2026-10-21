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

    // Every kind of injection, with its nested adders: the models and sections of a shunt compensator,
    // the generation of a boundary line
    @Test void injections() {
        VoltageLevel vl = addVoltageLevel();
        Object bat = injection(vl, BATTERY, "BAT");
        PropertyDispatcher.setDouble(bat, BAT_MIN_P, -50);
        PropertyDispatcher.setDouble(bat, BAT_MAX_P, 50);
        PropertyDispatcher.setDouble(bat, BAT_TARGET_P, 10);
        PropertyDispatcher.setDouble(bat, LOCAL_TARGET_Q, 0);
        PropertyDispatcher.setBool(bat, EQUIVALENT, true);
        Battery battery = (Battery) add(bat);
        assertEquals(10, battery.getTargetP());
        assertTrue(battery.isEquivalent());

        Object sc = injection(vl, SHUNT_COMPENSATOR, "SC");
        PropertyDispatcher.setInt(sc, SHUNT_SECTION_COUNT, 1);
        Object linear = PropertyDispatcher.invokeObject(sc, SHUNT_COMPENSATOR_LINEAR_MODEL);
        PropertyDispatcher.setDouble(linear, SHUNT_B_PER_SECTION, 1e-5);
        PropertyDispatcher.setDouble(linear, SHUNT_G_PER_SECTION, 0);
        PropertyDispatcher.setInt(linear, SHUNT_MAX_SECTION_COUNT, 2);
        assertNull(add(linear)); // nested
        ShuntCompensator shunt = (ShuntCompensator) add(sc);
        assertEquals(2, shunt.getMaximumSectionCount());

        Object sc2 = injection(vl, SHUNT_COMPENSATOR, "SC2");
        PropertyDispatcher.setInt(sc2, SHUNT_SECTION_COUNT, 2);
        PropertyDispatcher.setInt(sc2, SHUNT_SOLVED_SECTION_COUNT, 1);
        Object nonLinear = PropertyDispatcher.invokeObject(sc2, SHUNT_COMPENSATOR_NON_LINEAR_MODEL);
        for (double b : new double[] {1e-5, 3e-5}) {
            Object section = PropertyDispatcher.invokeObject(nonLinear, SHUNT_SECTION);
            PropertyDispatcher.setDouble(section, SHUNT_SECTION_B, b);
            PropertyDispatcher.setDouble(section, SHUNT_SECTION_G, 0);
            assertNull(add(section));
        }
        assertNull(add(nonLinear));
        ShuntCompensator shunt2 = (ShuntCompensator) add(sc2);
        assertEquals(3e-5, shunt2.getB());
        assertEquals(1, shunt2.getSolvedSectionCount());

        Object svc = injection(vl, STATIC_VAR_COMPENSATOR, "SVC");
        PropertyDispatcher.setDouble(svc, SVC_B_MIN, -1e-3);
        PropertyDispatcher.setDouble(svc, SVC_B_MAX, 1e-3);
        PropertyDispatcher.setDouble(svc, LOCAL_TARGET_Q, 0);
        assertEquals(1e-3, ((StaticVarCompensator) add(svc)).getBmax());

        Object vsc = injection(vl, VSC_CONVERTER_STATION, "VSC");
        PropertyDispatcher.setDouble(vsc, CONVERTER_STATION_LOSS_FACTOR, 1.1);
        PropertyDispatcher.setDouble(vsc, LOCAL_TARGET_Q, 0);
        assertEquals(1.1f, ((VscConverterStation) add(vsc)).getLossFactor());

        Object lcc = injection(vl, LCC_CONVERTER_STATION, "LCC");
        PropertyDispatcher.setDouble(lcc, CONVERTER_STATION_LOSS_FACTOR, 1.2);
        PropertyDispatcher.setDouble(lcc, LCC_POWER_FACTOR, 0.8);
        assertEquals(0.8f, ((LccConverterStation) add(lcc)).getPowerFactor());

        Object bl = injection(vl, BOUNDARY_LINE, "BL");
        for (int code : new int[] {BL_P0, BL_Q0, BL_R, BL_X, BL_G, BL_B}) {
            PropertyDispatcher.setDouble(bl, code, 1);
        }
        PropertyDispatcher.setString(bl, BL_PAIRING_KEY, "XNODE");
        Object generation = PropertyDispatcher.invokeObject(bl, BOUNDARY_LINE_GENERATION);
        PropertyDispatcher.setDouble(generation, GENERATION_MIN_P, 0);
        PropertyDispatcher.setDouble(generation, GENERATION_MAX_P, 100);
        PropertyDispatcher.setDouble(generation, GENERATION_TARGET_P, 50);
        PropertyDispatcher.setDouble(generation, GENERATION_TARGET_Q, 10);
        PropertyDispatcher.setDouble(generation, GENERATION_TARGET_V, 400);
        PropertyDispatcher.setBool(generation, GENERATION_VOLTAGE_REGULATION_ON, false);
        assertNull(add(generation));
        BoundaryLine line = (BoundaryLine) add(bl);
        assertEquals("XNODE", line.getPairingKey());
        assertEquals(50, line.getGeneration().getTargetP());

        // powsybl's checks: a shunt without a model
        Object noModel = injection(vl, SHUNT_COMPENSATOR, "SC3");
        PropertyDispatcher.setInt(noModel, SHUNT_SECTION_COUNT, 0);
        assertThrows(ValidationException.class, () -> add(noModel));
    }

    // Lines, transformers and HVDC lines: where each side of a branch is connected, the legs of a three
    // windings transformer, the converter stations of an HVDC line
    @Test void branches() {
        Substation s = network.newSubstation().setId("S").add();
        String[] ids = {"VL1", "VL2", "VL3"};
        double[] nominalV = {400, 225, 63};
        for (int i = 0; i < 3; i++) {
            s.newVoltageLevel().setId(ids[i]).setNominalV(nominalV[i]).setTopologyKind(TopologyKind.BUS_BREAKER).add()
                .getBusBreakerView().newBus().setId(ids[i] + "_B").add();
        }
        Object line = PropertyDispatcher.invokeObject(network, LINE);
        PropertyDispatcher.setString(line, ID, "L");
        PropertyDispatcher.setString(line, BRANCH_VOLTAGE_LEVEL1, "VL1");
        PropertyDispatcher.setString(line, BRANCH_BUS1, "VL1_B");
        PropertyDispatcher.setString(line, BRANCH_CONNECTABLE_BUS1, "VL1_B");
        PropertyDispatcher.setString(line, BRANCH_VOLTAGE_LEVEL2, "VL2");
        PropertyDispatcher.setString(line, BRANCH_CONNECTABLE_BUS2, "VL2_B");
        for (int code : new int[] {LINE_R, LINE_X, LINE_G1, LINE_B1, LINE_G2, LINE_B2}) {
            PropertyDispatcher.setDouble(line, code, 1);
        }
        Line l = (Line) add(line);
        assertEquals("VL2_B", l.getTerminal2().getBusBreakerView().getConnectableBus().getId());
        assertFalse(l.getTerminal2().isConnected()); // no bus: disconnected

        Object t2 = PropertyDispatcher.invokeObject(s, TWO_WINDINGS_TRANSFORMER);
        PropertyDispatcher.setString(t2, ID, "T2");
        PropertyDispatcher.setString(t2, BRANCH_VOLTAGE_LEVEL1, "VL1");
        PropertyDispatcher.setString(t2, BRANCH_BUS1, "VL1_B");
        PropertyDispatcher.setString(t2, BRANCH_VOLTAGE_LEVEL2, "VL2");
        PropertyDispatcher.setString(t2, BRANCH_BUS2, "VL2_B");
        PropertyDispatcher.setDouble(t2, TWO_WT_R, 0.5);
        PropertyDispatcher.setDouble(t2, TWO_WT_X, 20);
        PropertyDispatcher.setDouble(t2, TWO_WT_G, 0);
        PropertyDispatcher.setDouble(t2, TWO_WT_B, 0);
        PropertyDispatcher.setDouble(t2, TWO_WT_RATED_U1, 400);
        PropertyDispatcher.setDouble(t2, TWO_WT_RATED_U2, 225);
        PropertyDispatcher.setDouble(t2, TWO_WT_RATED_S, 300);
        assertEquals(300, ((TwoWindingsTransformer) add(t2)).getRatedS());

        Object t3 = PropertyDispatcher.invokeObject(s, THREE_WINDINGS_TRANSFORMER);
        PropertyDispatcher.setString(t3, ID, "T3");
        PropertyDispatcher.setDouble(t3, THREE_WT_RATED_U0, 400);
        for (int side = 0; side < 3; side++) {
            Object leg = PropertyDispatcher.invokeObject(t3, THREE_WINDINGS_TRANSFORMER_LEG);
            PropertyDispatcher.setInt(leg, LEG_SIDE, side);
            PropertyDispatcher.setDouble(leg, LEG_R, side + 1);
            PropertyDispatcher.setDouble(leg, LEG_X, 10);
            PropertyDispatcher.setDouble(leg, LEG_G, 0);
            PropertyDispatcher.setDouble(leg, LEG_B, 0);
            PropertyDispatcher.setDouble(leg, LEG_RATED_U, nominalV[side]);
            PropertyDispatcher.setString(leg, TERMINAL_VOLTAGE_LEVEL_ID, ids[side]);
            PropertyDispatcher.setString(leg, TERMINAL_BUS_BREAKER_BUS_ID, ids[side] + "_B");
            assertNull(add(leg)); // nested
        }
        ThreeWindingsTransformer twt3 = (ThreeWindingsTransformer) add(t3);
        assertEquals(3, twt3.getLeg3().getR());
        assertEquals("VL3", twt3.getLeg3().getTerminal().getVoltageLevel().getId());
        // A leg's field before its side: rejected
        Object noSide = PropertyDispatcher.invokeObject(PropertyDispatcher.invokeObject(s, THREE_WINDINGS_TRANSFORMER),
            THREE_WINDINGS_TRANSFORMER_LEG);
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.setDouble(noSide, LEG_R, 1))));

        for (int i = 1; i <= 2; i++) {
            network.getVoltageLevel("VL" + i).newVscConverterStation().setId("VSC" + i).setBus("VL" + i + "_B")
                .setLossFactor(1).setLocalTargetQ(0).add();
        }
        Object hvdc = PropertyDispatcher.invokeObject(network, HVDC_LINE);
        PropertyDispatcher.setString(hvdc, ID, "HVDC");
        PropertyDispatcher.setDouble(hvdc, HVDC_R, 1);
        PropertyDispatcher.setDouble(hvdc, HVDC_NOMINAL_V, 400);
        PropertyDispatcher.setDouble(hvdc, HVDC_ACTIVE_POWER_SETPOINT, 100);
        PropertyDispatcher.setDouble(hvdc, HVDC_MAX_P, 200);
        PropertyDispatcher.setInt(hvdc, HVDC_CONVERTERS_MODE, HvdcLine.ConvertersMode.SIDE_1_INVERTER_SIDE_2_RECTIFIER.ordinal());
        PropertyDispatcher.setString(hvdc, HVDC_CONVERTER_STATION_ID1, "VSC1");
        PropertyDispatcher.setString(hvdc, HVDC_CONVERTER_STATION_ID2, "VSC2");
        HvdcLine hvdcLine = (HvdcLine) add(hvdc);
        assertSame(network.getVscConverterStation("VSC2"), hvdcLine.getConverterStation2());
        assertEquals(HvdcLine.ConvertersMode.SIDE_1_INVERTER_SIDE_2_RECTIFIER, hvdcLine.getConvertersMode());
    }

    // Busbar sections, switches and internal connections of a node-breaker voltage level; a switch of
    // a bus-breaker one
    @Test void topology() {
        VoltageLevel vl = network.newVoltageLevel().setId("NB").setNominalV(225).setTopologyKind(TopologyKind.NODE_BREAKER).add();
        Object bbs = PropertyDispatcher.invokeObject(vl, BUSBAR_SECTION);
        PropertyDispatcher.setString(bbs, ID, "BBS");
        PropertyDispatcher.setInt(bbs, TERMINAL_NODE, 0);
        assertEquals(0, ((BusbarSection) add(bbs)).getTerminal().getNodeBreakerView().getNode());
        Object breaker = PropertyDispatcher.invokeObject(vl, REL_NBV_SWITCH);
        PropertyDispatcher.setString(breaker, ID, "BR");
        PropertyDispatcher.setInt(breaker, SW_NODE1, 0);
        PropertyDispatcher.setInt(breaker, SW_NODE2, 1);
        PropertyDispatcher.setInt(breaker, SW_KIND, SwitchKind.BREAKER.ordinal());
        PropertyDispatcher.setBool(breaker, SW_OPEN, true);
        PropertyDispatcher.setBool(breaker, SW_RETAINED, true);
        Switch sw = (Switch) add(breaker);
        assertEquals(SwitchKind.BREAKER, sw.getKind());
        assertTrue(sw.isOpen());
        assertTrue(sw.isRetained());
        Object ic = PropertyDispatcher.invokeObject(vl, INTERNAL_CONNECTION);
        PropertyDispatcher.setInt(ic, IC_NODE1, 1);
        PropertyDispatcher.setInt(ic, IC_NODE2, 2);
        assertNull(add(ic)); // add() returns nothing
        assertEquals(1, vl.getNodeBreakerView().getInternalConnectionCount());
        // A switch without its kind: rejected
        Object noKind = PropertyDispatcher.invokeObject(vl, REL_NBV_SWITCH);
        PropertyDispatcher.setString(noKind, ID, "SW");
        PropertyDispatcher.setInt(noKind, SW_NODE1, 2);
        PropertyDispatcher.setInt(noKind, SW_NODE2, 3);
        assertThrows(ValidationException.class, () -> add(noKind));

        VoltageLevel bb = addVoltageLevel();
        bb.getBusBreakerView().newBus().setId("B2").add();
        Object coupler = PropertyDispatcher.invokeObject(bb, REL_BBV_SWITCH);
        PropertyDispatcher.setString(coupler, ID, "CPL");
        PropertyDispatcher.setString(coupler, SW_BUS1, "B");
        PropertyDispatcher.setString(coupler, SW_BUS2, "B2");
        PropertyDispatcher.setBool(coupler, SW_OPEN, false);
        add(coupler);
        assertEquals("B2", bb.getBusBreakerView().getBus2("CPL").getId());
        // Node-breaker switches in a bus-breaker voltage level: rejected
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invokeObject(bb, REL_NBV_SWITCH))));
    }

    private static Object injection(VoltageLevel vl, int type, String id) {
        Object adder = PropertyDispatcher.invokeObject(vl, type);
        PropertyDispatcher.setString(adder, ID, id);
        PropertyDispatcher.setString(adder, TERMINAL_BUS_BREAKER_BUS_ID, "B");
        return adder;
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
