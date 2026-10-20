package com.powsybl.iidmbridge;

import com.powsybl.commons.PowsyblException;
import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidm.network.regulation.RegulationMode;
import com.powsybl.iidm.serde.NetworkSerDe;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import com.powsybl.iidmbridge.jni.IidmBridgeRegistry;
import org.junit.jupiter.api.*;

import java.nio.file.Path;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/** Classification of dispatcher failures for the C++ side ({@link BridgeErrors}). */
class BridgeErrorsTest {

    private Network network;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = createNetwork();
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    /**
     * Network used by the C++ backend error tests (test/data/backend_errors_test.xiidm):
     * G has ActivePowerControl, G2 has none, VL1 has a SlackTerminal on G, L2 is in VL2.
     */
    static Network createNetwork() {
        Network n = Network.create("backend_errors_test", "test");
        Substation s = n.newSubstation().setId("S").add();
        VoltageLevel vl1 = s.newVoltageLevel().setId("VL1").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl1.getBusBreakerView().newBus().setId("B1").add();
        VoltageLevel vl2 = s.newVoltageLevel().setId("VL2").setNominalV(225).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl2.getBusBreakerView().newBus().setId("B2").add();
        Generator g = vl1.newGenerator().setId("G").setBus("B1").setConnectableBus("B1")
            .setMinP(0).setMaxP(200).setTargetP(100)
            .setLocalTargetV(400).newVoltageRegulation().withMode(RegulationMode.VOLTAGE).withRegulating(true).add().add();
        vl1.newGenerator().setId("G2").setBus("B1").setConnectableBus("B1")
            .setMinP(0).setMaxP(50).setTargetP(10)
            .setLocalTargetV(400).newVoltageRegulation().withMode(RegulationMode.VOLTAGE).withRegulating(true).add().add();
        vl1.newLoad().setId("L").setBus("B1").setConnectableBus("B1").setP0(10).setQ0(0).add();
        vl2.newLoad().setId("L2").setBus("B2").setConnectableBus("B2").setP0(10).setQ0(0).add();
        g.newExtension(ActivePowerControlAdder.class).withParticipate(true).withDroop(4).withMaxTargetP(150).add();
        vl1.newExtension(SlackTerminalAdder.class).withTerminal(g.getTerminal()).add();
        return n;
    }

    /** Regenerates test/data/backend_errors_test.xiidm (run from the java/ directory). */
    public static void main(String[] args) {
        NetworkSerDe.write(createNetwork(), Path.of(args.length > 0 ? args[0] : "../test/data/backend_errors_test.xiidm"));
    }

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    private static int kindOfFailure(Executable call) {
        Throwable t = assertThrows(Throwable.class, call::run);
        return BridgeErrors.kindOf(t);
    }

    @FunctionalInterface
    interface Executable {
        void run() throws Throwable;
    }

    @Test void invalidHandle() {
        assertEquals(BridgeErrors.INVALID_HANDLE, kindOfFailure(() -> PropertyDispatcher.getDouble(0L, GEN_TARGET_P)));
        assertEquals(BridgeErrors.INVALID_HANDLE, kindOfFailure(() -> PropertyDispatcher.getDouble(123456789L, GEN_TARGET_P)));
    }

    @Test void unknownCodeAndWrongObjectType() {
        long g = reg(network.getGenerator("G"));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getDouble(g, 999999)));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getChildren(g, 999999)));
        // MEAS_VALUE applies to Measurement objects, not generators (ClassCastException)
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getDouble(g, MEAS_VALUE)));
    }

    @Test void absentExtension() {
        long g2 = reg(network.getGenerator("G2"));
        // Extension objects: absent is null; their codes do not apply to the host
        assertEquals(0L, PropertyDispatcher.getRelated(g2, ACTIVE_POWER_CONTROL));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getDouble(g2, EXT_APC_DROOP)));
        // Extensions read through their host
        assertEquals(BridgeErrors.EXTENSION_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getDouble(g2, EXT_CRC_Q_PERCENT)));
        long vl2 = reg(network.getVoltageLevel("VL2"));
        assertEquals(BridgeErrors.EXTENSION_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getRelated(vl2, REL_SLACK_TERMINAL)));
    }

    @Test void absentOptionalPartIsPropertyNotFound() {
        Substation s = network.getSubstation("S");
        TwoWindingsTransformer t = s.newTwoWindingsTransformer().setId("T")
            .setVoltageLevel1("VL1").setBus1("B1").setVoltageLevel2("VL2").setBus2("B2")
            .setR(1).setX(10).setG(0).setB(0).setRatedU1(400).setRatedU2(225).add();
        long h = reg(t);
        // getRatioTapChanger() is null → NullPointerException
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getDouble(h, TWO_WT_RTC_TARGET_V)));
    }

    @Test void rejectedValues() {
        long apc = PropertyDispatcher.getRelated(reg(network.getGenerator("G")), ACTIVE_POWER_CONTROL);
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.setDouble(apc, EXT_APC_MAX_TARGET_P, 500)));
        long vl1 = reg(network.getVoltageLevel("VL1"));
        long l2Terminal = reg(network.getLoad("L2").getTerminal());
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.setRelated(vl1, REL_SLACK_TERMINAL, l2Terminal)));
    }

    @Test void networkNotFound() {
        assertEquals(BridgeErrors.NETWORK_NOT_FOUND, kindOfFailure(() -> IidmBridgeRegistry.get("no-such-network")));
    }

    @Test void classification() {
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(new PowsyblException("x")));
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(new IllegalArgumentException("x")));
        // powsybl refuses operations in the current state with IllegalStateException
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(new IllegalStateException("x")));
        assertEquals(BridgeErrors.GENERIC, BridgeErrors.kindOf(new ArithmeticException("x")));
        assertEquals(BridgeErrors.GENERIC, BridgeErrors.kindOf(new OutOfMemoryError()));
    }

    @Test void messages() {
        assertEquals("plain", BridgeErrors.message(new PowsyblException("plain")));
        assertEquals("java.lang.ArithmeticException: boom", BridgeErrors.message(new ArithmeticException("boom")));
        assertEquals("java.lang.NullPointerException", BridgeErrors.message(new NullPointerException()));
    }
}
