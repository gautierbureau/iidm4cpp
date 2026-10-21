package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Overload checks of branches and three windings transformers through {@link PropertyDispatcher}:
 * permanent limits (evaluate), temporary limits (a check, run by ACT_ADD), overloads.
 */
class OverloadDispatchTest {

    private static final int CURRENT = LimitType.CURRENT.ordinal();

    private Network network;
    private Line line;
    private ThreeWindingsTransformer t3;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("overloads", "test");
        Substation s = network.newSubstation().setId("S").add();
        for (String id : new String[] {"1", "2", "3"}) {
            VoltageLevel vl = s.newVoltageLevel().setId("VL" + id).setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
            vl.getBusBreakerView().newBus().setId("B" + id).add().setV(400);
        }
        line = network.newLine().setId("LINE").setVoltageLevel1("VL1").setBus1("B1").setVoltageLevel2("VL2").setBus2("B2")
            .setR(1).setX(10).setG1(0).setB1(0).setG2(0).setB2(0).add();
        line.newOperationalLimitsGroup1("DEFAULT").newCurrentLimits().setPermanentLimit(800).setPermanentLimitName("PATL")
            .beginTemporaryLimit().setName("TL10").setAcceptableDuration(600).setValue(900).endTemporaryLimit()
            .beginTemporaryLimit().setName("TL1").setAcceptableDuration(60).setValue(1000).endTemporaryLimit()
            .add();
        line.setSelectedOperationalLimitsGroup1("DEFAULT");
        // 950 A at 400 kV
        line.getTerminal1().setP(950 * Math.sqrt(3) * 0.4).setQ(0);

        t3 = s.newThreeWindingsTransformer().setId("T3")
            .newLeg1().setR(0.1).setX(5).setRatedU(400).setVoltageLevel("VL1").setBus("B1").add()
            .newLeg2().setR(0.1).setX(5).setRatedU(400).setVoltageLevel("VL2").setBus("B2").add()
            .newLeg3().setR(0.1).setX(5).setRatedU(400).setVoltageLevel("VL3").setBus("B3").add()
            .add();
        t3.getLeg3().newOperationalLimitsGroup("WINTER").newCurrentLimits().setPermanentLimit(400)
            .beginTemporaryLimit().setName("TL20").setAcceptableDuration(1200).setValue(500).endTemporaryLimit()
            .add();
        t3.getLeg3().setSelectedOperationalLimitsGroup("WINTER");
        t3.getLeg3().getTerminal().setP(450 * Math.sqrt(3) * 0.4).setQ(0); // 450 A
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

    /** Runs a check of the temporary limits of a side; a NaN limit reduction: none. */
    private static long check(Object checked, int code, int side, double limitReduction) {
        long check = PropertyDispatcher.invoke(reg(checked), code);
        PropertyDispatcher.setInt(check, TERMINAL_SIDE, side);
        if (!Double.isNaN(limitReduction)) {
            PropertyDispatcher.setDouble(check, LIMIT_REDUCTION, limitReduction);
        }
        PropertyDispatcher.setInt(check, LL_LIMIT_TYPE, CURRENT);
        return PropertyDispatcher.invoke(check, ACT_ADD);
    }

    @Test void permanentLimits() {
        long l = reg(line);
        assertEquals(1, PropertyDispatcher.getBool(l, OVERLOADED));
        assertEquals(0, PropertyDispatcher.evaluate(l, OVERLOADED, new double[] {2}));
        assertEquals(60, PropertyDispatcher.getInt(l, OVERLOAD_DURATION));
        assertEquals(1, PropertyDispatcher.evaluate(l, CHECK_PERMANENT_LIMIT, new double[] {0, CURRENT}));
        assertEquals(0, PropertyDispatcher.evaluate(l, CHECK_PERMANENT_LIMIT, new double[] {1, CURRENT})); // no flow there
        assertEquals(0, PropertyDispatcher.evaluate(l, CHECK_PERMANENT_LIMIT, new double[] {0, 2, CURRENT}));

        long t = reg(t3);
        assertEquals(1, PropertyDispatcher.getBool(t, OVERLOADED));
        assertEquals(1200, PropertyDispatcher.getInt(t, OVERLOAD_DURATION));
        assertEquals(1, PropertyDispatcher.evaluate(t, CHECK_PERMANENT_LIMIT, new double[] {2, CURRENT}));
        assertEquals(0, PropertyDispatcher.evaluate(t, CHECK_PERMANENT_LIMIT, new double[] {0, CURRENT}));

        // Not for other objects
        long load = reg(network.getVoltageLevel("VL1").newLoad().setId("L").setBus("B1").setP0(1).setQ0(0).add());
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND,
            BridgeErrors.kindOf(assertThrows(Throwable.class, () -> PropertyDispatcher.evaluate(load, OVERLOADED, new double[] {1}))));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND,
            BridgeErrors.kindOf(assertThrows(Throwable.class, () -> PropertyDispatcher.invoke(load, CHECK_TEMPORARY_LIMITS))));
    }

    @Test void temporaryLimits() {
        // Over TL10 (900 A), under TL1 (1000 A)
        long overload = check(line, CHECK_TEMPORARY_LIMITS, 0, Double.NaN);
        assertEquals(1, PropertyDispatcher.getBool(overload, OVERLOAD));
        assertEquals(900, PropertyDispatcher.getDouble(overload, OVERLOAD_PREVIOUS_LIMIT));
        assertEquals("TL10", PropertyDispatcher.getString(overload, OVERLOAD_PREVIOUS_LIMIT_NAME));
        assertEquals("DEFAULT", PropertyDispatcher.getString(overload, OVERLOAD_OLG_ID));
        assertEquals(1, PropertyDispatcher.getDouble(overload, OVERLOAD_LIMIT_SCALING));
        long temporaryLimit = PropertyDispatcher.getRelated(overload, TEMPORARY_LIMIT);
        assertEquals("TL1", PropertyDispatcher.getString(temporaryLimit, TL_NAME));
        assertSame(line.getCurrentLimits1().orElseThrow().getTemporaryLimit(60), obj(temporaryLimit));

        // Limits halved: over all of them
        long reduced = check(line, CHECK_TEMPORARY_LIMITS, 0, 0.5);
        assertEquals("TL1", PropertyDispatcher.getString(reduced, OVERLOAD_PREVIOUS_LIMIT_NAME));
        assertEquals(0.5, PropertyDispatcher.getDouble(reduced, OVERLOAD_LIMIT_SCALING));
        long unacceptable = PropertyDispatcher.getRelated(reduced, TEMPORARY_LIMIT);
        assertEquals(0, PropertyDispatcher.getInt(unacceptable, TL_ACCEPTABLE_DURATION));
        assertEquals(1, PropertyDispatcher.getBool(unacceptable, TL_FICTITIOUS));

        // No overload: none; every selected group: a list
        assertEquals(0L, check(line, CHECK_TEMPORARY_LIMITS, 1, Double.NaN));
        long all = check(line, CHECK_ALL_TEMPORARY_LIMITS, 0, Double.NaN);
        assertEquals(1, PropertyDispatcher.getChildren(all, ELEMENTS).length);
        assertEquals(0, PropertyDispatcher.getChildren(check(line, CHECK_ALL_TEMPORARY_LIMITS, 0, 2), ELEMENTS).length);

        // Three windings transformer: side 3 over its permanent limit, under TL20
        long t3Overload = check(t3, CHECK_TEMPORARY_LIMITS, 2, Double.NaN);
        assertEquals(400, PropertyDispatcher.getDouble(t3Overload, OVERLOAD_PREVIOUS_LIMIT));
        assertEquals(LoadingLimits.DEFAULT_PERMANENT_LIMIT_NAME, PropertyDispatcher.getString(t3Overload, OVERLOAD_PREVIOUS_LIMIT_NAME));
        assertEquals(1, PropertyDispatcher.getChildren(check(t3, CHECK_ALL_TEMPORARY_LIMITS, 2, 0.95), ELEMENTS).length);

        // The side and the type are needed
        long incomplete = PropertyDispatcher.invoke(reg(line), CHECK_TEMPORARY_LIMITS);
        PropertyDispatcher.setInt(incomplete, LL_LIMIT_TYPE, CURRENT);
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND,
            BridgeErrors.kindOf(assertThrows(Throwable.class, () -> PropertyDispatcher.invoke(incomplete, ACT_ADD))));
    }
}
