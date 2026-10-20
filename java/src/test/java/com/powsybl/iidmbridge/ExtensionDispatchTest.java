package com.powsybl.iidmbridge;

import com.powsybl.commons.PowsyblException;
import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidm.network.regulation.RegulationMode;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * ActivePowerControl (all fields, Generator and Battery), SlackTerminal and
 * CoordinatedReactiveControl through {@link PropertyDispatcher}: extensions are objects of their own.
 */
class ExtensionDispatchTest {

    private Network network;
    private Generator gen;
    private Battery bat;
    private Load load;
    private VoltageLevel vl1;
    private VoltageLevel vl2;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("ext", "test");
        Substation s = network.newSubstation().setId("S").add();
        vl1 = s.newVoltageLevel().setId("VL1").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl1.getBusBreakerView().newBus().setId("B1").add();
        vl2 = s.newVoltageLevel().setId("VL2").setNominalV(225).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl2.getBusBreakerView().newBus().setId("B2").add();
        gen = vl1.newGenerator().setId("G").setBus("B1").setConnectableBus("B1")
            .setMinP(0).setMaxP(200).setTargetP(100)
            .setLocalTargetV(400).newVoltageRegulation().withMode(RegulationMode.VOLTAGE).withRegulating(true).add().add();
        bat = vl1.newBattery().setId("BAT").setBus("B1").setConnectableBus("B1")
            .setMinP(-50).setMaxP(50).setTargetP(0).setLocalTargetQ(0).add();
        load = vl1.newLoad().setId("L").setBus("B1").setConnectableBus("B1").setP0(10).setQ0(0).add();
        vl2.newLoad().setId("L2").setBus("B2").setConnectableBus("B2").setP0(10).setQ0(0).add();

        gen.newExtension(ActivePowerControlAdder.class)
            .withParticipate(true).withDroop(4).withParticipationFactor(0.3)
            .withMinTargetP(20).add();
        bat.newExtension(ActivePowerControlAdder.class)
            .withParticipate(false).withDroop(1).withParticipationFactor(0.6)
            .withMinTargetP(-40).withMaxTargetP(40).add();
        vl1.newExtension(SlackTerminalAdder.class).withTerminal(gen.getTerminal()).add();
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    // ── ActivePowerControl ────────────────────────────────────────────────

    @Test void generatorActivePowerControlFields() {
        long h = PropertyDispatcher.getRelated(reg(gen), ACTIVE_POWER_CONTROL);
        assertEquals(0.3, PropertyDispatcher.getDouble(h, EXT_APC_PARTICIPATION_FACTOR));
        assertEquals(20.0, PropertyDispatcher.getDouble(h, EXT_APC_MIN_TARGET_P));
        assertTrue(Double.isNaN(PropertyDispatcher.getDouble(h, EXT_APC_MAX_TARGET_P)));

        PropertyDispatcher.setDouble(h, EXT_APC_PARTICIPATION_FACTOR, 0.8);
        PropertyDispatcher.setDouble(h, EXT_APC_MAX_TARGET_P, 150);
        ActivePowerControl<Generator> apc = gen.getExtension(ActivePowerControl.class);
        assertEquals(0.8, apc.getParticipationFactor());
        assertEquals(150.0, apc.getMaxTargetP().orElseThrow());

        // NaN unsets the limit
        PropertyDispatcher.setDouble(h, EXT_APC_MIN_TARGET_P, Double.NaN);
        assertTrue(apc.getMinTargetP().isEmpty());
    }

    @Test void batteryActivePowerControlFields() {
        // Same extension type, same codes as on a generator
        long h = PropertyDispatcher.getRelated(reg(bat), ACTIVE_POWER_CONTROL);
        assertEquals(0.6, PropertyDispatcher.getDouble(h, EXT_APC_PARTICIPATION_FACTOR));
        assertEquals(-40.0, PropertyDispatcher.getDouble(h, EXT_APC_MIN_TARGET_P));
        assertEquals(40.0, PropertyDispatcher.getDouble(h, EXT_APC_MAX_TARGET_P));
        PropertyDispatcher.setDouble(h, EXT_APC_MAX_TARGET_P, 30);
        assertEquals(30.0, ((ActivePowerControl<?>) bat.getExtension(ActivePowerControl.class)).getMaxTargetP().orElseThrow());
    }

    @Test void targetPLimitOutsideMinMaxPThrows() {
        long h = PropertyDispatcher.getRelated(reg(gen), ACTIVE_POWER_CONTROL);
        assertThrows(PowsyblException.class, () -> PropertyDispatcher.setDouble(h, EXT_APC_MAX_TARGET_P, 500));
    }

    @Test void activePowerControlAbsent() {
        long h = reg(load);
        assertEquals(0L, PropertyDispatcher.getRelated(h, ACTIVE_POWER_CONTROL));
        // The extension's codes apply to the extension object only
        assertThrows(ClassCastException.class, () -> PropertyDispatcher.getDouble(reg(gen), EXT_APC_PARTICIPATION_FACTOR));
    }

    // ── SlackTerminal ─────────────────────────────────────────────────────

    @Test void slackTerminalSetThroughHandles() {
        long st = PropertyDispatcher.getRelated(reg(vl1), SLACK_TERMINAL);
        assertSame(vl1.getExtension(SlackTerminal.class), NetworkRegistry.lookup(st));
        assertEquals(0, PropertyDispatcher.getBool(st, EXT_ST_EMPTY));
        long term = PropertyDispatcher.getRelated(st, REL_TERMINAL);
        assertSame(gen.getTerminal(), NetworkRegistry.lookup(term));

        PropertyDispatcher.setRelated(st, REL_TERMINAL, reg(load.getTerminal()));
        assertSame(load.getTerminal(), vl1.getExtension(SlackTerminal.class).getTerminal());

        // 0 (INVALID_HANDLE) unsets it
        PropertyDispatcher.setRelated(st, REL_TERMINAL, 0L);
        assertNull(vl1.getExtension(SlackTerminal.class).getTerminal());
        assertEquals(0L, PropertyDispatcher.getRelated(st, REL_TERMINAL));
        assertEquals(1, PropertyDispatcher.getBool(st, EXT_ST_EMPTY));

        // Without a slack terminal in any variant, cleaning removes the extension
        PropertyDispatcher.invoke(st, ACT_CLEAN_IF_EMPTY);
        assertNull(vl1.getExtension(SlackTerminal.class));
        assertEquals(0L, PropertyDispatcher.getRelated(reg(vl1), SLACK_TERMINAL));
    }

    @Test void slackTerminalSetThroughObjects() {
        Object st = PropertyDispatcher.getRelatedObject(vl1, SLACK_TERMINAL);
        PropertyDispatcher.setRelatedObject(st, REL_TERMINAL, bat.getTerminal());
        assertSame(bat.getTerminal(), vl1.getExtension(SlackTerminal.class).getTerminal());
        // Cleaning keeps an extension that has a slack terminal
        PropertyDispatcher.invokeObject(st, ACT_CLEAN_IF_EMPTY);
        assertSame(st, vl1.getExtension(SlackTerminal.class));
    }

    @Test void slackTerminalFromAnotherVoltageLevelThrows() {
        Object st = PropertyDispatcher.getRelatedObject(vl1, SLACK_TERMINAL);
        Terminal other = network.getLoad("L2").getTerminal();
        assertThrows(PowsyblException.class, () -> PropertyDispatcher.setRelatedObject(st, REL_TERMINAL, other));
    }

    @Test void slackTerminalCreatedAndRemoved() {
        long vl = reg(vl2);
        assertEquals(0L, PropertyDispatcher.getRelated(vl, SLACK_TERMINAL));
        long adder = PropertyDispatcher.invoke(vl, SLACK_TERMINAL);
        PropertyDispatcher.setRelated(adder, REL_TERMINAL, reg(network.getLoad("L2").getTerminal()));
        long st = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertSame(vl2.getExtension(SlackTerminal.class), NetworkRegistry.lookup(st));
        assertSame(network.getLoad("L2").getTerminal(), vl2.getExtension(SlackTerminal.class).getTerminal());

        PropertyDispatcher.setRelated(vl, SLACK_TERMINAL, 0L);
        assertNull(vl2.getExtension(SlackTerminal.class));
    }

    // ── CoordinatedReactiveControl ────────────────────────────────────────

    @Test void coordinatedReactiveControlCreatedReadAndRemoved() {
        long g = reg(gen);
        assertEquals(0L, PropertyDispatcher.getRelated(g, COORDINATED_REACTIVE_CONTROL));
        long adder = PropertyDispatcher.invoke(g, COORDINATED_REACTIVE_CONTROL);
        PropertyDispatcher.setDouble(adder, EXT_CRC_Q_PERCENT, 40);
        long crc = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(1, PropertyDispatcher.getBool(crc, COORDINATED_REACTIVE_CONTROL));
        assertEquals(0, PropertyDispatcher.getBool(crc, ACTIVE_POWER_CONTROL));

        long again = PropertyDispatcher.getRelated(g, COORDINATED_REACTIVE_CONTROL);
        assertEquals(1, PropertyDispatcher.isSameObject(crc, again));
        assertEquals(40.0, PropertyDispatcher.getDouble(again, EXT_CRC_Q_PERCENT));
        PropertyDispatcher.setDouble(again, EXT_CRC_Q_PERCENT, 60);
        assertEquals(60.0, gen.getExtension(CoordinatedReactiveControl.class).getQPercent());

        PropertyDispatcher.setRelated(g, COORDINATED_REACTIVE_CONTROL, 0L);
        assertNull(gen.getExtension(CoordinatedReactiveControl.class));
    }

    @Test void unknownRelationForSetThrows() {
        assertThrows(IllegalArgumentException.class,
            () -> PropertyDispatcher.setRelatedObject(vl1, REL_BUS, gen.getTerminal()));
    }
}
