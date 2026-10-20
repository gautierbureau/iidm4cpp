package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.util.Arrays;
import java.util.List;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * The codes of the base classes (Identifiable, Connectable, Injection, Branch, HvdcConverterStation)
 * and the primitives behind them: optional values, string lists, keyed strings, identity, type tests.
 */
class IdentifiableDispatchTest {

    private Network network;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("net", "test");
        Substation s = network.newSubstation().setId("S").add();
        VoltageLevel vl1 = s.newVoltageLevel().setId("VL1").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        VoltageLevel vl2 = s.newVoltageLevel().setId("VL2").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl1.getBusBreakerView().newBus().setId("B1").add();
        vl2.getBusBreakerView().newBus().setId("B2").add();
        vl1.newLoad().setId("L").setName("load").setBus("B1").setP0(10).setQ0(1).add();
        vl1.newShuntCompensator().setId("SH").setBus("B1").setSectionCount(1)
            .newLinearModel().setBPerSection(1e-5).setMaximumSectionCount(2).add().add();
        network.newLine().setId("LINE").setVoltageLevel1("VL1").setBus1("B1").setVoltageLevel2("VL2").setBus2("B2")
            .setR(1).setX(10).add();
        vl1.newVscConverterStation().setId("VSC1").setBus("B1").setLossFactor(0.01f).setLocalTargetQ(0).add();
        vl2.newVscConverterStation().setId("VSC2").setBus("B2").setLossFactor(0.02f).setLocalTargetQ(0).add();
        network.newHvdcLine().setId("HVDC").setConverterStationId1("VSC1").setConverterStationId2("VSC2")
            .setR(1).setNominalV(400).setMaxP(100).setActivePowerSetpoint(50)
            .setConvertersMode(HvdcLine.ConvertersMode.SIDE_1_RECTIFIER_SIDE_2_INVERTER).add();
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    @Test void names() {
        Load load = network.getLoad("L");
        assertEquals("load", PropertyDispatcher.getString(load, NAME));
        assertEquals("load", PropertyDispatcher.getOptString(load, NAME));
        Line line = network.getLine("LINE");
        assertEquals("LINE", PropertyDispatcher.getString(line, NAME)); // name or id
        assertNull(PropertyDispatcher.getOptString(line, NAME));
        assertEquals("LINE", PropertyDispatcher.getOptString(line, ID)); // any string property
        Substation s = network.getSubstation("S");
        assertEquals("", PropertyDispatcher.getString(s, SUBSTATION_TSO));
        assertNull(PropertyDispatcher.getOptString(s, SUBSTATION_TSO));
        assertNull(PropertyDispatcher.getOptString(s, COUNTRY));
    }

    @Test void optionalInts() {
        ShuntCompensator shunt = network.getShuntCompensator("SH");
        assertEquals(1, PropertyDispatcher.getOptInt(shunt, SHUNT_SECTION_COUNT));
        assertEquals(PropertyDispatcher.NO_INT, PropertyDispatcher.getOptInt(shunt, SHUNT_SOLVED_SECTION_COUNT));
        PropertyDispatcher.setInt(shunt, SHUNT_SOLVED_SECTION_COUNT, 2);
        assertEquals(2, PropertyDispatcher.getOptInt(reg(shunt), SHUNT_SOLVED_SECTION_COUNT));
        assertEquals(2, PropertyDispatcher.getOptInt(shunt, SHUNT_MAX_SECTION_COUNT)); // any int property
    }

    @Test void properties() {
        long load = reg(network.getLoad("L"));
        assertNull(PropertyDispatcher.getKeyedString(load, PROPERTY, "k"));
        PropertyDispatcher.setKeyedString(load, PROPERTY, "k", "v");
        PropertyDispatcher.setKeyedString(load, PROPERTY, "a", "w");
        assertEquals("v", PropertyDispatcher.getKeyedString(load, PROPERTY, "k"));
        assertArrayEquals(new String[] {"a", "k"}, PropertyDispatcher.getStringList(load, PROPERTY_NAMES)); // sorted
        PropertyDispatcher.setKeyedString(load, PROPERTY, "k", null);
        assertFalse(network.getLoad("L").hasProperty("k"));
        assertArrayEquals(new String[] {"a"}, PropertyDispatcher.getStringList(load, PROPERTY_NAMES));
    }

    @Test void aliases() {
        long line = reg(network.getLine("LINE"));
        PropertyDispatcher.setKeyedString(line, ALIAS_TYPE, "b-alias", "");        // no type
        PropertyDispatcher.setKeyedString(line, ALIAS_TYPE, "a-alias", "code");
        assertArrayEquals(new String[] {"a-alias", "b-alias"}, PropertyDispatcher.getStringList(line, ALIASES));
        assertEquals("code", PropertyDispatcher.getKeyedString(line, ALIAS_TYPE, "a-alias"));
        assertNull(PropertyDispatcher.getKeyedString(line, ALIAS_TYPE, "b-alias"));
        assertEquals("a-alias", PropertyDispatcher.getKeyedString(line, ALIAS_FROM_TYPE, "code"));
        assertSame(network.getLine("LINE"), network.getIdentifiable("a-alias"));

        // The alias is taken: made unique
        long load = reg(network.getLoad("L"));
        PropertyDispatcher.setKeyedString(load, UNIQUE_ALIAS_TYPE, "a-alias", "");
        String[] loadAliases = PropertyDispatcher.getStringList(load, ALIASES);
        assertEquals(1, loadAliases.length);
        assertNotEquals("a-alias", loadAliases[0]);
        assertThrows(PowsyblExceptionHolder.TYPE, () -> PropertyDispatcher.setKeyedString(load, ALIAS_TYPE, "a-alias", ""));

        PropertyDispatcher.setKeyedString(line, ALIAS_TYPE, "a-alias", null);
        assertArrayEquals(new String[] {"b-alias"}, PropertyDispatcher.getStringList(line, ALIASES));
    }

    @Test void typeTests() {
        Load load = network.getLoad("L");
        assertEquals(IdentifiableType.LOAD.ordinal(), PropertyDispatcher.getInt(load, IDENTIFIABLE_TYPE));
        for (int type : new int[] {IDENTIFIABLE, CONNECTABLE, INJECTION, LOAD}) {
            assertTrue(PropertyDispatcher.getBool(load, type), "type " + type);
        }
        for (int type : new int[] {BRANCH, GENERATOR, HVDC_CONVERTER_STATION, SUBSTATION}) {
            assertFalse(PropertyDispatcher.getBool(load, type), "type " + type);
        }
        Line line = network.getLine("LINE");
        assertTrue(PropertyDispatcher.getBool(line, BRANCH));
        assertFalse(PropertyDispatcher.getBool(line, INJECTION));
        VscConverterStation vsc = network.getVscConverterStation("VSC1");
        assertTrue(PropertyDispatcher.getBool(vsc, HVDC_CONVERTER_STATION));
        assertTrue(PropertyDispatcher.getBool(vsc, VSC_CONVERTER_STATION));
        assertFalse(PropertyDispatcher.getBool(vsc, LCC_CONVERTER_STATION));
    }

    @Test void identity() {
        Load load = network.getLoad("L");
        long h1 = reg(load);
        long h2 = reg(load);
        assertNotEquals(h1, h2);
        assertEquals(1, PropertyDispatcher.isSameObject(h1, h2));
        assertEquals(0, PropertyDispatcher.isSameObject(h1, reg(network.getLine("LINE"))));
    }

    @Test void lists() {
        Object net = network;
        assertEquals(network.getIdentifiables().size(), PropertyDispatcher.getChildObjects(net, IDENTIFIABLE).length);
        // powsybl's order (a HashMap by id: the same in a JVM and in a native image)
        assertEquals(List.of("L", "LINE", "SH", "VSC1", "VSC2"),
            Arrays.stream(PropertyDispatcher.getChildObjects(net, CONNECTABLE)).map(o -> ((Identifiable<?>) o).getId()).sorted().toList());
        assertEquals(List.of("LINE"),
            Arrays.stream(PropertyDispatcher.getChildObjects(net, BRANCH)).map(o -> ((Identifiable<?>) o).getId()).toList());
        assertEquals(2, PropertyDispatcher.getChildObjects(net, HVDC_CONVERTER_STATION).length);
        assertEquals(4, PropertyDispatcher.getChildObjects(network.getVoltageLevel("VL1"), CONNECTABLE).length); // L, SH, LINE, VSC1
        Object[] terminals = PropertyDispatcher.getChildObjects(network.getLine("LINE"), TERMINAL);
        assertEquals(2, terminals.length);
        assertEquals(ThreeSides.TWO.ordinal(), PropertyDispatcher.getInt(terminals[1], TERMINAL_SIDE));
        assertEquals(-1, PropertyDispatcher.getInt(network.getLoad("L").getTerminal(), TERMINAL_SIDE));
    }

    @Test void lookups() {
        assertSame(network.getLoad("L"), PropertyDispatcher.findObjectById(network, IDENTIFIABLE, "L"));
        assertSame(network.getLoad("L"), PropertyDispatcher.findObjectById(network, INJECTION, "L"));
        assertNull(PropertyDispatcher.findObjectById(network, INJECTION, "LINE"));
        assertSame(network.getLine("LINE"), PropertyDispatcher.findObjectById(network, BRANCH, "LINE"));
        assertSame(network.getLine("LINE"), PropertyDispatcher.findObjectById(network, CONNECTABLE, "LINE"));
        assertSame(network.getVscConverterStation("VSC2"), PropertyDispatcher.findObjectById(network, HVDC_CONVERTER_STATION, "VSC2"));
        assertSame(network.getSubstation("S"), PropertyDispatcher.findObjectById(network, IDENTIFIABLE, "S"));
    }

    @Test void relations() {
        Line line = network.getLine("LINE");
        assertSame(line.getTerminal2(), PropertyDispatcher.getRelatedObjectByString(line, REL_TERMINAL, "VL2"));
        HvdcLine hvdc = network.getHvdcLine("HVDC");
        assertSame(network.getVscConverterStation("VSC1"), PropertyDispatcher.getRelatedObject(hvdc, REL_CONVERTER_STATION1));
        assertSame(network.getVscConverterStation("VSC2"), PropertyDispatcher.getRelatedObject(hvdc, REL_CONVERTER_STATION2));
        VscConverterStation vsc1 = network.getVscConverterStation("VSC1");
        assertSame(network.getVscConverterStation("VSC2"), PropertyDispatcher.getRelatedObject(vsc1, REL_OTHER_CONVERTER_STATION));
        assertSame(hvdc, PropertyDispatcher.getRelatedObject(vsc1, REL_HVDC_LINE));
        assertEquals(HvdcConverterStation.HvdcType.VSC.ordinal(), PropertyDispatcher.getInt(vsc1, CONVERTER_STATION_HVDC_TYPE));
        assertEquals(0.01, PropertyDispatcher.getDouble(vsc1, CONVERTER_STATION_LOSS_FACTOR), 1e-6);
        PropertyDispatcher.setDouble(vsc1, CONVERTER_STATION_LOSS_FACTOR, 0.03);
        assertEquals(0.03f, vsc1.getLossFactor());
        assertSame(network.getBusBreakerView().getBus("B2"), PropertyDispatcher.getRelatedObjectByString(network.getVoltageLevel("VL2"), REL_BBV_BUS, "B2"));
        assertNull(PropertyDispatcher.getRelatedObjectByString(network.getVoltageLevel("VL2"), REL_BBV_BUS, "B1"));
    }

    @Test void connectAndEquivalent() {
        Load load = network.getLoad("L");
        assertNull(PropertyDispatcher.invokeObject(load, ACT_CONNECT)); // already connected
        assertSame(load, PropertyDispatcher.invokeObject(load, ACT_DISCONNECT));
        assertFalse(load.getTerminal().isConnected());
        Terminal terminal = load.getTerminal();
        assertSame(terminal, PropertyDispatcher.invokeObject(terminal, ACT_CONNECT));
        assertTrue(terminal.isConnected());
        assertSame(network.getLine("LINE"), PropertyDispatcher.invokeObject(network.getLine("LINE"), ACT_DISCONNECT));
        assertFalse(PropertyDispatcher.getBool(load, EQUIVALENT));
        PropertyDispatcher.setBool(load, EQUIVALENT, true);
        assertTrue(load.isEquivalent());
        assertThrows(BridgeErrors.PropertyNotFoundException.class, () -> PropertyDispatcher.setBool(load, TERMINAL_CONNECTED, false));
    }

    /** The exception powsybl throws for an alias that is already taken. */
    private static final class PowsyblExceptionHolder {
        static final Class<com.powsybl.commons.PowsyblException> TYPE = com.powsybl.commons.PowsyblException.class;
    }
}
