package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Tie lines (two boundary lines paired) and grounds through {@link PropertyDispatcher}: the tie
 * line's boundary lines and characteristics, its connection, removal and creation; a ground's
 * creation, lookup and removal.
 */
class TieLineAndGroundDispatchTest {

    private Network network;
    private VoltageLevel vl1;
    private BoundaryLine half1;
    private BoundaryLine half2;
    private TieLine tieLine;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("tie_lines", "test");
        vl1 = network.newSubstation().setId("S1").add()
            .newVoltageLevel().setId("VL1").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl1.getBusBreakerView().newBus().setId("B1").add();
        VoltageLevel vl2 = network.newSubstation().setId("S2").add()
            .newVoltageLevel().setId("VL2").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl2.getBusBreakerView().newBus().setId("B2").add();
        half1 = vl1.newBoundaryLine().setId("HALF1").setBus("B1").setP0(0).setQ0(0)
            .setR(1).setX(10).setG(0).setB(0).setPairingKey("XNODE").add();
        half2 = vl2.newBoundaryLine().setId("HALF2").setBus("B2").setP0(0).setQ0(0)
            .setR(2).setX(20).setG(0).setB(0).setPairingKey("XNODE").add();
        tieLine = network.newTieLine().setId("TIE").setBoundaryLine1("HALF1").setBoundaryLine2("HALF2").add();
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

    @Test void tieLines() {
        long t = reg(tieLine);
        long n = reg(network);
        assertEquals(1, PropertyDispatcher.getBool(t, TIE_LINE));
        assertEquals(1, PropertyDispatcher.getBool(t, BRANCH));
        assertEquals(0, PropertyDispatcher.getBool(reg(half1), TIE_LINE));
        assertEquals(tieLine.getR(), PropertyDispatcher.getDouble(t, LINE_R));
        assertEquals(30, PropertyDispatcher.getDouble(t, LINE_X), 1e-9);
        assertEquals("XNODE", PropertyDispatcher.getString(t, BL_PAIRING_KEY));

        // Its boundary lines, and theirs
        assertSame(half1, obj(PropertyDispatcher.getRelatedByIndex(t, BOUNDARY_LINE, 0)));
        assertSame(half2, obj(PropertyDispatcher.getRelatedByString(t, BOUNDARY_LINE, "VL2")));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(t, BOUNDARY_LINE, "NONE"));
        assertSame(tieLine, obj(PropertyDispatcher.getRelated(reg(half2), TIE_LINE)));

        // In the network, and a branch
        assertEquals(1, PropertyDispatcher.getChildren(n, TIE_LINE).length);
        assertSame(tieLine, obj(PropertyDispatcher.findById(n, TIE_LINE, "TIE")));
        assertSame(tieLine, obj(PropertyDispatcher.findById(n, BRANCH, "TIE")));
        assertSame(half1.getTerminal(), obj(PropertyDispatcher.getRelated(t, REL_TERMINAL1)));

        // Disconnecting it disconnects its boundary lines
        assertNotEquals(0L, PropertyDispatcher.invoke(t, ACT_DISCONNECT));
        assertFalse(half1.getTerminal().isConnected());
        assertFalse(half2.getTerminal().isConnected());
        assertNotEquals(0L, PropertyDispatcher.invoke(t, ACT_CONNECT));
        assertTrue(half2.getTerminal().isConnected());
    }

    @Test void removalAndCreation() {
        long n = reg(network);
        PropertyDispatcher.invoke(reg(tieLine), ACT_REMOVE);
        assertNull(network.getTieLine("TIE"));
        assertEquals(0L, PropertyDispatcher.getRelated(reg(half1), TIE_LINE));

        long adder = PropertyDispatcher.invoke(n, TIE_LINE);
        PropertyDispatcher.setString(adder, ID, "TIE2");
        PropertyDispatcher.setString(adder, BOUNDARY_LINE1_ID, "HALF1");
        PropertyDispatcher.setString(adder, BOUNDARY_LINE2_ID, "HALF2");
        TieLine created = (TieLine) obj(PropertyDispatcher.invoke(adder, ACT_ADD));
        assertSame(half2, created.getBoundaryLine2());

        // Removed with the flows at the boundary as the boundary lines' P0 and Q0
        half1.getTerminal().setP(10).setQ(5);
        half1.getTerminal().getBusView().getBus().setV(400).setAngle(0);
        double boundaryP = half1.getBoundary().getP();
        PropertyDispatcher.invoke(reg(created), ACT_REMOVE_UPDATING_BOUNDARY_LINES);
        assertFalse(half1.isPaired());
        assertEquals(-boundaryP, half1.getP0(), 1e-9);
    }

    @Test void grounds() {
        long vl = reg(vl1);
        long adder = PropertyDispatcher.invoke(vl, GROUND);
        PropertyDispatcher.setString(adder, ID, "GROUND");
        PropertyDispatcher.setString(adder, TERMINAL_BUS_BREAKER_BUS_ID, "B1");
        long ground = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(1, PropertyDispatcher.getBool(ground, GROUND));
        assertEquals(1, PropertyDispatcher.getBool(ground, INJECTION));
        assertEquals(1, PropertyDispatcher.getChildren(vl, GROUND).length);
        assertEquals(1, PropertyDispatcher.getChildren(reg(network), GROUND).length);
        assertSame(obj(ground), obj(PropertyDispatcher.findById(reg(network), GROUND, "GROUND")));
        assertEquals(1, PropertyDispatcher.evaluate(vl, CHILD_COUNT, new double[] {GROUND}));
        PropertyDispatcher.invoke(ground, ACT_REMOVE);
        assertNull(network.getGround("GROUND"));
    }
}
