package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.util.Arrays;
import java.util.Set;
import java.util.stream.Collectors;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Topology through {@link PropertyDispatcher}: the buses of a terminal in each view, moving an
 * equipment's end, a bus's values and equipment, the members of the node-breaker and bus-breaker
 * views of a voltage level, list objects.
 */
class TopologyDispatchTest {

    private Network network;
    private VoltageLevel vl1;
    private VoltageLevel vl2;
    private Load load;
    private Load load2;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("topology", "test");
        Substation s = network.newSubstation().setId("S").setCountry(Country.FR).add();
        vl1 = s.newVoltageLevel().setId("VL1").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl1.getBusBreakerView().newBus().setId("B1").add();
        vl1.getBusBreakerView().newBus().setId("B1b").add();
        vl1.getBusBreakerView().newSwitch().setId("SW").setBus1("B1").setBus2("B1b").setOpen(false).add();
        load = vl1.newLoad().setId("L").setBus("B1").setP0(10).setQ0(1).add();
        vl2 = s.newVoltageLevel().setId("VL2").setNominalV(400).setTopologyKind(TopologyKind.NODE_BREAKER).add();
        VoltageLevel.NodeBreakerView nbv = vl2.getNodeBreakerView();
        nbv.newBusbarSection().setId("BBS").setNode(0).add();
        nbv.newDisconnector().setId("D").setNode1(0).setNode2(1).setOpen(false).add();
        nbv.newBreaker().setId("BR").setNode1(1).setNode2(2).setOpen(false).add();
        load2 = vl2.newLoad().setId("L2").setNode(2).setP0(5).setQ0(0).add();
        nbv.newInternalConnection().setNode1(3).setNode2(4).add();
        nbv.newInternalConnection().setNode1(3).setNode2(0).add();
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    private static Object obj(long handle) {
        return NetworkRegistry.lookup(handle);
    }

    private static Set<String> ids(long[] handles) {
        return Arrays.stream(handles).mapToObj(h -> PropertyDispatcher.getString(h, ID)).collect(Collectors.toSet());
    }

    private static int kind(Executable call) {
        return BridgeErrors.kindOf(assertThrows(Throwable.class, call::execute));
    }

    @FunctionalInterface
    private interface Executable {
        void execute();
    }

    @Test void terminalBuses() {
        long t = reg(load.getTerminal());
        Bus b1 = vl1.getBusBreakerView().getBus("B1");
        Bus merged = vl1.getBusView().getBuses().iterator().next();
        assertSame(b1, obj(PropertyDispatcher.getRelated(t, REL_BUS)));
        assertSame(b1, obj(PropertyDispatcher.getRelated(t, REL_CONNECTABLE_BUS)));
        assertSame(merged, obj(PropertyDispatcher.getRelated(t, REL_BV_BUS)));
        assertSame(merged, obj(PropertyDispatcher.getRelated(t, REL_BV_CONNECTABLE_BUS)));

        load.getTerminal().disconnect();
        assertEquals(0L, PropertyDispatcher.getRelated(t, REL_BUS));
        assertEquals(0L, PropertyDispatcher.getRelated(t, REL_BV_BUS));
        assertSame(b1, obj(PropertyDispatcher.getRelated(t, REL_CONNECTABLE_BUS)));
    }

    @Test void terminalCurrentAndConnectableBus() {
        vl1.getBusBreakerView().getBus("B1").setV(400);
        vl1.getBusBreakerView().getBus("B1b").setV(400);
        load.getTerminal().setP(30).setQ(40);
        long t = reg(load.getTerminal());
        double i = PropertyDispatcher.getDouble(t, TERMINAL_I);
        assertFalse(Double.isNaN(i));
        assertEquals(load.getTerminal().getI(), i);

        PropertyDispatcher.setString(t, TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, "B1b");
        assertEquals("B1b", load.getTerminal().getBusBreakerView().getConnectableBus().getId());
        // Not in a node-breaker topology
        assertEquals(BridgeErrors.VALIDATION, kind(() -> PropertyDispatcher.setString(reg(load2.getTerminal()),
            TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, "BBS")));
    }

    @Test void moves() {
        // To another bus, disconnected (the equipment gets a new terminal)
        long move = PropertyDispatcher.invoke(reg(load.getTerminal()), MOVE_CONNECTABLE);
        PropertyDispatcher.setString(move, TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, "B1b");
        PropertyDispatcher.setBool(move, TERMINAL_CONNECTED, 0);
        assertEquals(0L, PropertyDispatcher.invoke(move, ACT_ADD));
        assertEquals("B1b", load.getTerminal().getBusBreakerView().getConnectableBus().getId());
        assertFalse(load.getTerminal().isConnected());

        // To a node
        long toNode = PropertyDispatcher.invoke(reg(load2.getTerminal()), MOVE_CONNECTABLE);
        PropertyDispatcher.setInt(toNode, TERMINAL_NODE, 5);
        PropertyDispatcher.setString(toNode, TERMINAL_VOLTAGE_LEVEL_ID, "VL2");
        PropertyDispatcher.invoke(toNode, ACT_ADD);
        assertEquals(5, load2.getTerminal().getNodeBreakerView().getNode());

        // Whether it connects: missing
        long incomplete = PropertyDispatcher.invoke(reg(load.getTerminal()), MOVE_CONNECTABLE);
        PropertyDispatcher.setString(incomplete, TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, "B1");
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kind(() -> PropertyDispatcher.invoke(incomplete, ACT_ADD)));
    }

    @Test void buses() {
        Bus b1 = vl1.getBusBreakerView().getBus("B1");
        long b = reg(b1);
        load.getTerminal().setP(10).setQ(1);
        assertEquals(10, PropertyDispatcher.getDouble(b, BUS_P));
        assertEquals(1, PropertyDispatcher.getDouble(b, BUS_Q));
        PropertyDispatcher.setDouble(b, FICTITIOUS_P0, 1.5);
        PropertyDispatcher.setDouble(b, FICTITIOUS_Q0, 0.5);
        assertEquals(1.5, PropertyDispatcher.getDouble(b, FICTITIOUS_P0));
        assertEquals(0.5, b1.getFictitiousQ0());
        assertEquals(1, PropertyDispatcher.getBool(b, BUS_IN_MAIN_CONNECTED_COMPONENT));
        assertEquals(1, PropertyDispatcher.getBool(b, BUS_IN_MAIN_SYNCHRONOUS_COMPONENT));

        // Its equipment and connected terminals
        assertEquals(Set.of("L"), ids(PropertyDispatcher.getChildren(b, LOAD)));
        assertEquals(0, PropertyDispatcher.getChildren(b, GENERATOR).length);
        long[] terminals = PropertyDispatcher.getChildren(b, TERMINAL);
        assertEquals(1, terminals.length);
        assertSame(load.getTerminal(), obj(terminals[0]));
        assertEquals(1, PropertyDispatcher.evaluate(b, CHILD_COUNT, new double[] {TERMINAL}));

        b1.setV(400);
        PropertyDispatcher.invoke(b, ACT_UNSET_SOLVED_VALUES);
        assertTrue(Double.isNaN(b1.getV()));
    }

    @Test void nodeBreakerView() {
        long vl = reg(vl2);
        assertSame(network.getBusbarSection("BBS"), obj(PropertyDispatcher.getRelatedByString(vl, BUSBAR_SECTION, "BBS")));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(vl, BUSBAR_SECTION, "NONE"));
        assertTrue(PropertyDispatcher.getInt(vl, VL_NBV_MAXIMUM_NODE_INDEX) >= 4);
        assertEquals(1, PropertyDispatcher.evaluate(vl, VL_NBV_ATTACHED_EQUIPMENT, new double[] {2}));
        assertEquals(1, PropertyDispatcher.evaluate(vl, VL_NBV_ATTACHED_EQUIPMENT, new double[] {1})); // switches
        assertEquals(0, PropertyDispatcher.evaluate(vl, VL_NBV_ATTACHED_EQUIPMENT, new double[] {7}));

        // Lists: the switches at a node, the nodes internally connected to one
        long atNode1 = PropertyDispatcher.getRelatedByIndex(vl, REL_NBV_SWITCH, 1);
        assertEquals(Set.of("D", "BR"), ids(PropertyDispatcher.getChildren(atNode1, ELEMENTS)));
        long connected = PropertyDispatcher.getRelatedByIndex(vl, INTERNAL_CONNECTION, 3);
        int[] nodes = PropertyDispatcher.getIntList(connected, ELEMENTS);
        Arrays.sort(nodes);
        assertArrayEquals(new int[] {0, 4}, nodes);

        // The fictitious injection at a node
        assertEquals(0, PropertyDispatcher.evaluate(vl, FICTITIOUS_P0, new double[] {2}));
        long change = PropertyDispatcher.invoke(vl, FICTITIOUS_P0);
        PropertyDispatcher.setInt(change, TERMINAL_NODE, 2);
        PropertyDispatcher.setDouble(change, FICTITIOUS_P0, 2.5);
        PropertyDispatcher.invoke(change, ACT_ADD);
        assertEquals(2.5, PropertyDispatcher.evaluate(vl, FICTITIOUS_P0, new double[] {2}));
        assertEquals(0, vl2.getNodeBreakerView().getFictitiousQ0(2));
        long wrongField = PropertyDispatcher.invoke(vl, FICTITIOUS_Q0);
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kind(() -> PropertyDispatcher.setDouble(wrongField, FICTITIOUS_P0, 1)));

        // Removals
        long removal = PropertyDispatcher.invoke(vl, VL_NBV_REMOVE_INTERNAL_CONNECTIONS);
        PropertyDispatcher.setInt(removal, IC_NODE1, 4);
        PropertyDispatcher.setInt(removal, IC_NODE2, 3);
        PropertyDispatcher.invoke(removal, ACT_ADD);
        assertEquals(java.util.List.of(0), vl2.getNodeBreakerView().getNodesInternalConnectedTo(3));
        PropertyDispatcher.setString(vl, VL_NBV_REMOVE_SWITCH, "BR");
        assertNull(network.getSwitch("BR"));
        assertEquals(BridgeErrors.VALIDATION, kind(() -> PropertyDispatcher.setString(vl, VL_NBV_REMOVE_SWITCH, "BR")));
    }

    @Test void busBreakerView() {
        long vl = reg(vl1);
        String busViewBusId = vl1.getBusView().getBuses().iterator().next().getId();
        long buses = PropertyDispatcher.getRelatedByString(vl, REL_BBV_BUSES_OF_BV_BUS, busViewBusId);
        long[] elements = PropertyDispatcher.getChildren(buses, ELEMENTS);
        assertEquals("B1", PropertyDispatcher.getString(elements[0], ID));
        assertEquals("B1b", PropertyDispatcher.getString(elements[1], ID));

        // Buses with equipment cannot go; switches can
        assertEquals(BridgeErrors.VALIDATION, kind(() -> PropertyDispatcher.invoke(vl, ACT_REMOVE_ALL_BUSES)));
        PropertyDispatcher.setString(vl, VL_BBV_REMOVE_SWITCH, "SW");
        assertNull(network.getSwitch("SW"));
        vl1.getBusBreakerView().newSwitch().setId("SW2").setBus1("B1").setBus2("B1b").setOpen(true).add();
        PropertyDispatcher.invoke(vl, ACT_REMOVE_ALL_SWITCHES);
        assertEquals(0, vl1.getBusBreakerView().getSwitchCount());
        load.remove();
        PropertyDispatcher.invoke(vl, ACT_REMOVE_ALL_BUSES);
        assertEquals(0, vl1.getBusBreakerView().getBusCount());
    }
}
