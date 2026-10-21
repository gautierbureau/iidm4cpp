package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.nio.file.Path;
import java.util.Arrays;
import java.util.List;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Reference priorities, reference terminals and the observability area adder through
 * {@link PropertyDispatcher}, on test/data/references_test.xiidm: the extensions' objects, their
 * changes, and the powsybl static helpers.
 */
class ReferenceExtensionDispatchTest {

    private Network network;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = com.powsybl.iidm.serde.NetworkSerDe.read(Path.of("../test/data/references_test.xiidm"));
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

    private static List<String> connectables(long[] priorities) {
        return Arrays.stream(priorities)
            .mapToObj(p -> ((Terminal) obj(PropertyDispatcher.getRelated(p, REL_TERMINAL))).getConnectable().getId() + "="
                + PropertyDispatcher.getInt(p, EXT_RP_PRIORITY))
            .toList();
    }

    private static void setPriority(Object connectable, int side, int priority) {
        long change = PropertyDispatcher.invoke(reg(connectable), EXT_RP_PRIORITY);
        if (side >= 0) PropertyDispatcher.setInt(change, TERMINAL_SIDE, side);
        PropertyDispatcher.setInt(change, EXT_RP_PRIORITY, priority);
        PropertyDispatcher.invoke(change, ACT_ADD);
    }

    @Test void referencePriorities() {
        long n = reg(network);
        assertEquals(List.of("G2=1", "G1=2", "L12=3"), connectables(PropertyDispatcher.getChildren(n, REFERENCE_PRIORITY)));
        long g1 = reg(network.getGenerator("G1"));
        long ext = PropertyDispatcher.getRelated(g1, REFERENCE_PRIORITIES);
        long[] priorities = PropertyDispatcher.getChildren(ext, REFERENCE_PRIORITY);
        assertEquals(1, PropertyDispatcher.getBool(priorities[0], REFERENCE_PRIORITY));
        assertEquals(List.of("G1=2"), connectables(priorities));

        // The helpers
        assertEquals(2, PropertyDispatcher.evaluate(g1, EXT_RP_PRIORITY, new double[0]));
        long line = reg(network.getLine("L12"));
        assertEquals(3, PropertyDispatcher.evaluate(line, EXT_RP_PRIORITY, new double[] {TwoSides.TWO.ordinal()}));
        assertEquals(0, PropertyDispatcher.evaluate(line, EXT_RP_PRIORITY, new double[] {TwoSides.ONE.ordinal()}));
        setPriority(network.getLine("L12"), TwoSides.ONE.ordinal(), 4);
        assertEquals(4, ReferencePriority.get(network.getLine("L12"), TwoSides.ONE));
        setPriority(network.getGenerator("G2"), -1, 5);
        assertEquals(5, ReferencePriority.get(network.getGenerator("G2")));
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> setPriority(network.getGenerator("G2"), -1, -1))));

        // A new priority on the extension, replacing that of its terminal; deletions
        long adder = PropertyDispatcher.invoke(ext, REFERENCE_PRIORITY);
        PropertyDispatcher.setRelated(adder, REL_TERMINAL, reg(network.getGenerator("G1").getTerminal()));
        PropertyDispatcher.setInt(adder, EXT_RP_PRIORITY, 7);
        long added = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(7, PropertyDispatcher.getInt(added, EXT_RP_PRIORITY));
        assertEquals(List.of("G1=7"), connectables(PropertyDispatcher.getChildren(ext, REFERENCE_PRIORITY)));
        PropertyDispatcher.invoke(ext, ACT_DELETE_REFERENCE_PRIORITIES);
        assertEquals(0, PropertyDispatcher.getChildren(ext, REFERENCE_PRIORITY).length);
        PropertyDispatcher.invoke(n, ACT_DELETE_REFERENCE_PRIORITIES);
        assertEquals(0, PropertyDispatcher.getChildren(n, REFERENCE_PRIORITY).length);
    }

    @Test void referenceTerminals() {
        long n = reg(network);
        long ext = PropertyDispatcher.getRelated(n, REFERENCE_TERMINALS);
        Terminal g1 = network.getGenerator("G1").getTerminal();
        Terminal g2 = network.getGenerator("G2").getTerminal();
        assertSame(g1, obj(PropertyDispatcher.getChildren(ext, TERMINAL)[0]));
        PropertyDispatcher.setRelated(ext, ADD_REFERENCE_TERMINAL, reg(g2));
        assertEquals(2, PropertyDispatcher.getChildren(ext, TERMINAL).length);

        // Replaced, reset, added by the helper
        long change = PropertyDispatcher.invoke(ext, TERMINAL);
        PropertyDispatcher.setRelated(change, ADD_REFERENCE_TERMINAL, reg(g2));
        assertEquals(0L, PropertyDispatcher.invoke(change, ACT_ADD));
        assertEquals(java.util.Set.of(g2), ReferenceTerminals.getTerminals(network));
        PropertyDispatcher.invoke(ext, ACT_RESET_REFERENCE_TERMINALS);
        assertEquals(0, PropertyDispatcher.getChildren(ext, TERMINAL).length);
        PropertyDispatcher.invoke(reg(g1), ACT_ADD_REFERENCE_TERMINAL);
        assertEquals(java.util.Set.of(g1), ReferenceTerminals.getTerminals(network));
        PropertyDispatcher.invoke(n, ACT_RESET_REFERENCE_TERMINALS);
        assertTrue(ReferenceTerminals.getTerminals(network).isEmpty());

        // A new extension with its terminals
        network.removeExtension(ReferenceTerminals.class);
        long adder = PropertyDispatcher.invoke(n, REFERENCE_TERMINALS);
        PropertyDispatcher.setRelated(adder, ADD_REFERENCE_TERMINAL, reg(g1));
        PropertyDispatcher.setRelated(adder, ADD_REFERENCE_TERMINAL, reg(g2));
        long added = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(2, PropertyDispatcher.getChildren(added, TERMINAL).length);
    }

    @Test void observabilityAreaAdder() {
        VoltageLevel vl2 = network.getVoltageLevel("VL2");
        long adder = PropertyDispatcher.invoke(reg(vl2), OBSERVABILITY_AREA);
        long area = PropertyDispatcher.invoke(adder, AREA_CHARACTERISTICS);
        PropertyDispatcher.setInt(area, OBS_AREA_NUMBER, 1);
        PropertyDispatcher.setInt(area, OBS_AREA_STATUS, ObservabilityArea.ObservabilityStatus.OBSERVABLE.ordinal());
        PropertyDispatcher.setString(area, OBS_AREA_BUS_BREAKER_BUS_ID, "B2");
        assertEquals(0L, PropertyDispatcher.invoke(area, ACT_ADD)); // nested
        long ext = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(1, PropertyDispatcher.getChildren(ext, AREA_CHARACTERISTICS).length);
        ObservabilityArea created = vl2.getExtension(ObservabilityArea.class);
        assertEquals(ObservabilityArea.ObservabilityStatus.OBSERVABLE, created.getObservabilityAreas().iterator().next().getStatus());

        // By nodes, by a bus view bus
        VoltageLevel vl3 = network.getVoltageLevel("VL3");
        assertEquals(2, PropertyDispatcher.getChildren(PropertyDispatcher.getRelated(reg(vl3), OBSERVABILITY_AREA), AREA_CHARACTERISTICS).length);
        vl3.removeExtension(ObservabilityArea.class);
        long nodes = PropertyDispatcher.invoke(reg(vl3), OBSERVABILITY_AREA);
        long byNodes = PropertyDispatcher.invoke(nodes, AREA_CHARACTERISTICS);
        PropertyDispatcher.setInt(byNodes, OBS_AREA_NUMBER, 3);
        PropertyDispatcher.setInt(byNodes, OBS_AREA_STATUS, ObservabilityArea.ObservabilityStatus.BORDER.ordinal());
        PropertyDispatcher.setInt(byNodes, OBS_AREA_NODE, 0);
        PropertyDispatcher.setInt(byNodes, OBS_AREA_NODE, 1);
        PropertyDispatcher.invoke(byNodes, ACT_ADD);
        PropertyDispatcher.invoke(nodes, ACT_ADD);
        assertEquals(3, vl3.getExtension(ObservabilityArea.class).getObservabilityAreas().iterator().next().getAreaNumber());

        VoltageLevel vl1 = network.getVoltageLevel("VL1");
        vl1.removeExtension(ObservabilityArea.class);
        long bus = PropertyDispatcher.invoke(reg(vl1), OBSERVABILITY_AREA);
        long byBus = PropertyDispatcher.invoke(bus, AREA_CHARACTERISTICS);
        PropertyDispatcher.setInt(byBus, OBS_AREA_NUMBER, 4);
        PropertyDispatcher.setInt(byBus, OBS_AREA_STATUS, ObservabilityArea.ObservabilityStatus.NON_OBSERVABLE.ordinal());
        PropertyDispatcher.setString(byBus, OBS_AREA_BUS_ID, vl1.getBusView().getBuses().iterator().next().getId());
        PropertyDispatcher.invoke(byBus, ACT_ADD);
        PropertyDispatcher.invoke(bus, ACT_ADD);
        assertEquals(4, vl1.getExtension(ObservabilityArea.class).getObservabilityAreas().iterator().next().getAreaNumber());
    }
}
