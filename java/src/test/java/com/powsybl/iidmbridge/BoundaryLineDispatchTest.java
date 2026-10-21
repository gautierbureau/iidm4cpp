package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Boundary lines through {@link PropertyDispatcher}: their values, pairing key, generation and
 * boundary, and the targets of their generation from the solved values.
 */
class BoundaryLineDispatchTest {

    private Network network;
    private BoundaryLine line;
    private BoundaryLine plain;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("boundary", "test");
        VoltageLevel vl = network.newVoltageLevel().setId("VL").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl.getBusBreakerView().newBus().setId("B").add().setV(402).setAngle(-1);
        line = vl.newBoundaryLine().setId("BL").setBus("B").setP0(10).setQ0(5).setR(1).setX(10).setG(0).setB(0)
            .setPairingKey("XNODE")
            .newGeneration().setTargetP(20).setMinP(0).setMaxP(100).setTargetQ(5).setVoltageRegulationOn(true).setTargetV(401).add()
            .add();
        line.getGeneration().newMinMaxReactiveLimits().setMinQ(-50).setMaxQ(60).add();
        plain = vl.newBoundaryLine().setId("PLAIN").setBus("B").setP0(1).setQ0(1).setR(1).setX(1).setG(0).setB(0).add();
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

    @Test void values() {
        long l = reg(line);
        PropertyDispatcher.setDouble(l, BL_R, 2);
        PropertyDispatcher.setDouble(l, BL_X, 20);
        PropertyDispatcher.setDouble(l, BL_G, 1e-6);
        PropertyDispatcher.setDouble(l, BL_B, 1e-5);
        assertEquals(20, line.getX());
        assertEquals(1e-5, PropertyDispatcher.getDouble(l, BL_B));
        assertEquals(0, PropertyDispatcher.getBool(l, BL_PAIRED));

        // Pairing key: none, read as optional
        assertEquals("XNODE", PropertyDispatcher.getString(l, BL_PAIRING_KEY));
        PropertyDispatcher.setString(l, BL_PAIRING_KEY, "XNODE2");
        assertEquals("XNODE2", line.getPairingKey());
        assertNull(PropertyDispatcher.getOptString(reg(plain), BL_PAIRING_KEY));
        assertThrows(BridgeErrors.PropertyNotFoundException.class, () -> PropertyDispatcher.getString(reg(plain), BL_PAIRING_KEY));
    }

    @Test void generation() {
        long g = PropertyDispatcher.getRelated(reg(line), BOUNDARY_LINE_GENERATION);
        assertSame(line.getGeneration(), obj(g));
        assertEquals(1, PropertyDispatcher.getBool(g, BOUNDARY_LINE_GENERATION));
        assertEquals(0L, PropertyDispatcher.getRelated(reg(plain), BOUNDARY_LINE_GENERATION));
        assertEquals(20, PropertyDispatcher.getDouble(g, GENERATION_TARGET_P));
        assertEquals(100, PropertyDispatcher.getDouble(g, GENERATION_MAX_P));
        assertEquals(1, PropertyDispatcher.getBool(g, GENERATION_VOLTAGE_REGULATION_ON));
        assertEquals(60, PropertyDispatcher.getDouble(PropertyDispatcher.getRelated(g, MIN_MAX_REACTIVE_LIMITS), RL_MAX_Q));

        PropertyDispatcher.setDouble(g, GENERATION_TARGET_P, 25);
        PropertyDispatcher.setDouble(g, GENERATION_MIN_P, -10);
        PropertyDispatcher.setDouble(g, GENERATION_MAX_P, 90);
        PropertyDispatcher.setDouble(g, GENERATION_TARGET_Q, 4);
        PropertyDispatcher.setDouble(g, GENERATION_TARGET_V, 399);
        PropertyDispatcher.setBool(g, GENERATION_VOLTAGE_REGULATION_ON, 0);
        BoundaryLine.Generation generation = line.getGeneration();
        assertEquals(25, generation.getTargetP());
        assertEquals(-10, generation.getMinP());
        assertEquals(90, generation.getMaxP());
        assertEquals(4, generation.getTargetQ());
        assertEquals(399, generation.getTargetV());
        assertFalse(generation.isVoltageRegulationOn());
    }

    @Test void boundary() {
        line.getTerminal().setP(-12).setQ(-6);
        long boundary = PropertyDispatcher.getRelated(reg(line), BOUNDARY);
        assertSame(line.getBoundary(), obj(boundary));
        assertEquals(1, PropertyDispatcher.getBool(boundary, BOUNDARY));
        assertEquals(line.getBoundary().getV(), PropertyDispatcher.getDouble(boundary, BOUNDARY_V));
        assertEquals(line.getBoundary().getAngle(), PropertyDispatcher.getDouble(boundary, BOUNDARY_ANGLE));
        assertEquals(line.getBoundary().getP(), PropertyDispatcher.getDouble(boundary, BOUNDARY_P));
        assertEquals(line.getBoundary().getQ(), PropertyDispatcher.getDouble(boundary, BOUNDARY_Q));
        assertEquals(line.getBoundary().getI(), PropertyDispatcher.getDouble(boundary, BOUNDARY_I));
        assertSame(line, obj(PropertyDispatcher.getRelated(boundary, BOUNDARY_LINE)));
        assertSame(network.getVoltageLevel("VL"), obj(PropertyDispatcher.getRelated(boundary, REL_VOLTAGE_LEVEL)));
    }

    @Test void solvedValues() {
        line.getTerminal().setP(-12).setQ(-6);
        long l = reg(line);
        PropertyDispatcher.invoke(l, ACT_SET_TARGET_P_TO_P);
        assertEquals(12, line.getGeneration().getTargetP());
        PropertyDispatcher.invoke(l, ACT_SET_TARGET_Q_TO_Q);
        assertEquals(6, line.getGeneration().getTargetQ());
        PropertyDispatcher.invoke(l, ACT_SET_TARGET_V_TO_V);
        assertEquals(402, line.getGeneration().getTargetV());
        line.getTerminal().setP(-8);
        PropertyDispatcher.invoke(l, ACT_APPLY_SOLVED_VALUES);
        assertEquals(8, line.getGeneration().getTargetP());
    }
}
