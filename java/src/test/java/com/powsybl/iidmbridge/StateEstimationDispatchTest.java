package com.powsybl.iidmbridge;

import com.powsybl.commons.PowsyblException;
import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidm.network.regulation.RegulationMode;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.util.Arrays;
import java.util.Set;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * State estimation extensions through {@link PropertyDispatcher}: each extension is an object of
 * its own, reached from its host with its type code. The handle-based API (GraalVM backend) and the
 * object-based API (JNI backend).
 */
class StateEstimationDispatchTest {

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

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    /**
     * Small network carrying every state estimation extension. Also used to generate
     * examples/state_estimation_test.xiidm.
     */
    static Network createNetwork() {
        Network n = Network.create("state_estimation_test", "test");
        Substation s1 = n.newSubstation().setId("S1").setCountry(Country.FR).add();
        VoltageLevel vl1 = s1.newVoltageLevel().setId("VL1").setNominalV(400)
            .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl1.getBusBreakerView().newBus().setId("B1").add();
        vl1.getBusBreakerView().newBus().setId("B2").add();
        vl1.getBusBreakerView().newSwitch().setId("SW1").setBus1("B1").setBus2("B2").setOpen(false).add();
        VoltageLevel vl2 = s1.newVoltageLevel().setId("VL2").setNominalV(225)
            .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl2.getBusBreakerView().newBus().setId("B3").add();
        VoltageLevel vl3 = s1.newVoltageLevel().setId("VL3").setNominalV(63)
            .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl3.getBusBreakerView().newBus().setId("B4").add();

        Generator g1 = vl1.newGenerator().setId("G1").setBus("B1").setConnectableBus("B1")
            .setMinP(0).setMaxP(500).setTargetP(100)
            .setLocalTargetV(400).newVoltageRegulation().withMode(RegulationMode.VOLTAGE).withRegulating(true).add().add();
        vl2.newLoad().setId("L2").setBus("B3").setConnectableBus("B3").setP0(80).setQ0(10).add();

        TwoWindingsTransformer t2 = s1.newTwoWindingsTransformer().setId("T2")
            .setVoltageLevel1("VL1").setBus1("B1").setConnectableBus1("B1")
            .setVoltageLevel2("VL2").setBus2("B3").setConnectableBus2("B3")
            .setR(0.5).setX(10).setG(0).setB(0).setRatedU1(400).setRatedU2(225).add();
        t2.newRatioTapChanger().setLowTapPosition(0).setTapPosition(3).setLoadTapChangingCapabilities(false)
            .beginStep().setRho(0.95).setR(0).setX(0).setG(0).setB(0).endStep()
            .beginStep().setRho(0.97).setR(0).setX(0).setG(0).setB(0).endStep()
            .beginStep().setRho(0.99).setR(0).setX(0).setG(0).setB(0).endStep()
            .beginStep().setRho(1.01).setR(0).setX(0).setG(0).setB(0).endStep()
            .add();

        ThreeWindingsTransformer t3 = s1.newThreeWindingsTransformer().setId("T3").setRatedU0(400)
            .newLeg1().setVoltageLevel("VL1").setBus("B2").setConnectableBus("B2")
                .setR(0.1).setX(5).setG(0).setB(0).setRatedU(400).add()
            .newLeg2().setVoltageLevel("VL2").setBus("B3").setConnectableBus("B3")
                .setR(0.1).setX(5).setG(0).setB(0).setRatedU(225).add()
            .newLeg3().setVoltageLevel("VL3").setBus("B4").setConnectableBus("B4")
                .setR(0.1).setX(5).setG(0).setB(0).setRatedU(63).add()
            .add();

        Substation s2 = n.newSubstation().setId("S2").setCountry(Country.FR).add();
        VoltageLevel vl4 = s2.newVoltageLevel().setId("VL4").setNominalV(400)
            .setTopologyKind(TopologyKind.NODE_BREAKER).add();
        BusbarSection bbs1 = vl4.getNodeBreakerView().newBusbarSection().setId("BBS1").setNode(0).add();
        vl4.getNodeBreakerView().newBreaker().setId("BR1").setNode1(0).setNode2(1).setOpen(false).add();
        vl4.getNodeBreakerView().newBreaker().setId("BR2").setNode1(0).setNode2(2).setOpen(false).add();
        vl4.newLoad().setId("L4").setNode(1).setP0(20).setQ0(5).add();
        Line line1 = n.newLine().setId("LINE1")
            .setVoltageLevel1("VL1").setBus1("B1").setConnectableBus1("B1")
            .setVoltageLevel2("VL4").setNode2(2)
            .setR(1).setX(20).setG1(0).setB1(0).setG2(0).setB2(0).add();

        // ── Observability ─────────────────────────────────────────────────
        g1.newExtension(InjectionObservabilityAdder.class)
            .withObservable(true)
            .withStandardDeviationP(1.5).withRedundantP(true)
            .withStandardDeviationQ(2.0)
            .add();
        bbs1.newExtension(InjectionObservabilityAdder.class)
            .withObservable(false)
            .withStandardDeviationV(0.8).withRedundantV(false)
            .add();
        line1.newExtension(BranchObservabilityAdder.class)
            .withObservable(true)
            .withStandardDeviationP1(0.1).withRedundantP1(false)
            .withStandardDeviationQ2(0.2)
            .add();

        // ── ToBeEstimated ─────────────────────────────────────────────────
        t2.newExtension(TwoWindingsTransformerToBeEstimatedAdder.class)
            .withRatioTapChangerStatus(true).withPhaseTapChangerStatus(false).add();
        t3.newExtension(ThreeWindingsTransformerToBeEstimatedAdder.class)
            .withRatioTapChanger1Status(true).withPhaseTapChanger3Status(true).add();

        // ── Measurements ──────────────────────────────────────────────────
        g1.newExtension(MeasurementsAdder.class).add();
        Measurements<Generator> g1Meas = g1.getExtension(Measurements.class);
        g1Meas.newMeasurement().setId("G1_P").setType(Measurement.Type.ACTIVE_POWER)
            .setValue(101.5).setStandardDeviation(1.0).setValid(true).add();
        g1Meas.newMeasurement().setId("G1_Q").setType(Measurement.Type.REACTIVE_POWER)
            .setValue(12.0).setStandardDeviation(0.5).setValid(false).add();
        line1.newExtension(MeasurementsAdder.class).add();
        Measurements<Line> lineMeas = line1.getExtension(Measurements.class);
        lineMeas.newMeasurement().setId("LINE1_P1").setType(Measurement.Type.ACTIVE_POWER)
            .setSide(ThreeSides.ONE).setValue(55.0).setStandardDeviation(0.3).setValid(true).add();
        lineMeas.newMeasurement().setId("LINE1_I2").setType(Measurement.Type.CURRENT)
            .setSide(ThreeSides.TWO).setValue(80.0).setStandardDeviation(2.0).setValid(true).add();
        bbs1.newExtension(MeasurementsAdder.class).add();
        Measurements<BusbarSection> bbsMeas = bbs1.getExtension(Measurements.class);
        bbsMeas.newMeasurement().setId("BBS1_V").setType(Measurement.Type.VOLTAGE)
            .setValue(401.0).setStandardDeviation(2.0).setValid(true).add();

        // ── DiscreteMeasurements ──────────────────────────────────────────
        t2.newExtension(DiscreteMeasurementsAdder.class).add();
        DiscreteMeasurements<TwoWindingsTransformer> t2Disc = t2.getExtension(DiscreteMeasurements.class);
        t2Disc.newDiscreteMeasurement().setId("T2_TAP").setType(DiscreteMeasurement.Type.TAP_POSITION)
            .setTapChanger(DiscreteMeasurement.TapChanger.RATIO_TAP_CHANGER).setValue(3).setValid(true).add();
        Switch sw1 = n.getSwitch("SW1");
        sw1.newExtension(DiscreteMeasurementsAdder.class).add();
        DiscreteMeasurements<Switch> swDisc = sw1.getExtension(DiscreteMeasurements.class);
        swDisc.newDiscreteMeasurement().setId("SW1_POS").setType(DiscreteMeasurement.Type.SWITCH_POSITION)
            .setValue(true).setValid(true).add();
        swDisc.newDiscreteMeasurement().setId("SW1_STATE").setType(DiscreteMeasurement.Type.OTHER)
            .setValue("CLOSED").setValid(false).add();

        // ── ObservabilityArea ─────────────────────────────────────────────
        vl1.newExtension(ObservabilityAreaAdder.class)
            .withObservabilityAreaByBusBreakerViewBuses(Set.of("B1"), 1, ObservabilityArea.ObservabilityStatus.OBSERVABLE)
            .withObservabilityAreaByBusBreakerViewBuses(Set.of("B2"), 2, ObservabilityArea.ObservabilityStatus.BORDER)
            .add();
        vl4.newExtension(ObservabilityAreaAdder.class)
            .withObservabilityAreaByNodes(Set.of(0, 1, 2), 7, ObservabilityArea.ObservabilityStatus.NON_OBSERVABLE)
            .add();
        return n;
    }

    /** The extension of the given type of a host: present. */
    private static long extension(Object host, int type) {
        long h = PropertyDispatcher.getRelated(reg(host), type);
        assertNotEquals(0L, h);
        assertEquals(1, PropertyDispatcher.getBool(h, type));
        return h;
    }

    // ── ToBeEstimated ─────────────────────────────────────────────────────

    @Test void twoWindingsTransformerToBeEstimated() {
        TwoWindingsTransformer t2 = network.getTwoWindingsTransformer("T2");
        long h = extension(t2, TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED);
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_TWO_WT_TBE_RTC));
        assertEquals(0, PropertyDispatcher.getBool(h, EXT_TWO_WT_TBE_PTC));
        PropertyDispatcher.setBool(h, EXT_TWO_WT_TBE_PTC, 1);
        assertTrue(t2.getExtension(TwoWindingsTransformerToBeEstimated.class).shouldEstimatePhaseTapChanger());
        // Absent: no extension object
        long t3 = reg(network.getThreeWindingsTransformer("T3"));
        assertEquals(0L, PropertyDispatcher.getRelated(t3, TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED));
    }

    @Test void threeWindingsTransformerToBeEstimated() {
        ThreeWindingsTransformer t3 = network.getThreeWindingsTransformer("T3");
        long h = extension(t3, THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED);
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_THREE_WT_TBE_RTC1));
        assertEquals(0, PropertyDispatcher.getBool(h, EXT_THREE_WT_TBE_RTC2));
        assertEquals(0, PropertyDispatcher.getBool(h, EXT_THREE_WT_TBE_PTC1));
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_THREE_WT_TBE_PTC3));
        PropertyDispatcher.setBool(h, EXT_THREE_WT_TBE_RTC2, 1);
        PropertyDispatcher.setBool(h, EXT_THREE_WT_TBE_PTC3, 0);
        ThreeWindingsTransformerToBeEstimated ext = t3.getExtension(ThreeWindingsTransformerToBeEstimated.class);
        assertTrue(ext.shouldEstimateRatioTapChanger2());
        assertFalse(ext.shouldEstimatePhaseTapChanger3());
    }

    @Test void toBeEstimatedRemovedAndCreated() {
        TwoWindingsTransformer t2 = network.getTwoWindingsTransformer("T2");
        long host = reg(t2);
        PropertyDispatcher.setRelated(host, TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED, 0L);
        assertNull(t2.getExtension(TwoWindingsTransformerToBeEstimated.class));
        assertEquals(0L, PropertyDispatcher.getRelated(host, TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED));

        long adder = PropertyDispatcher.invoke(host, TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED);
        PropertyDispatcher.setBool(adder, EXT_TWO_WT_TBE_PTC, 1);
        long h = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertSame(t2.getExtension(TwoWindingsTransformerToBeEstimated.class), NetworkRegistry.lookup(h));
        assertEquals(0, PropertyDispatcher.getBool(h, EXT_TWO_WT_TBE_RTC));
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_TWO_WT_TBE_PTC));

        ThreeWindingsTransformer t3 = network.getThreeWindingsTransformer("T3");
        long host3 = reg(t3);
        PropertyDispatcher.setRelated(host3, THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED, 0L);
        long adder3 = PropertyDispatcher.invoke(host3, THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED);
        PropertyDispatcher.setBool(adder3, EXT_THREE_WT_TBE_PTC2, 1);
        PropertyDispatcher.invoke(adder3, ACT_ADD);
        ThreeWindingsTransformerToBeEstimated ext3 = t3.getExtension(ThreeWindingsTransformerToBeEstimated.class);
        assertTrue(ext3.shouldEstimatePhaseTapChanger2());
        assertFalse(ext3.shouldEstimateRatioTapChanger1());
    }

    // ── Observability ─────────────────────────────────────────────────────

    @Test void injectionObservability() {
        Generator g1 = network.getGenerator("G1");
        long h = extension(g1, INJECTION_OBSERVABILITY);
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_OBS_OBSERVABLE));
        long p = PropertyDispatcher.getRelated(h, OBS_QUALITY_P);
        assertEquals(1.5, PropertyDispatcher.getDouble(p, OBS_QUALITY_STANDARD_DEVIATION));
        assertEquals(1, PropertyDispatcher.getInt(p, OBS_QUALITY_REDUNDANT));
        assertEquals(-1, PropertyDispatcher.getInt(PropertyDispatcher.getRelated(h, OBS_QUALITY_Q), OBS_QUALITY_REDUNDANT));
        assertEquals(0L, PropertyDispatcher.getRelated(h, OBS_QUALITY_V));

        // Setting the standard deviation of an undefined quality creates it
        PropertyDispatcher.setDouble(h, OBS_QUALITY_V, 3.0);
        InjectionObservability<Generator> ext = g1.getExtension(InjectionObservability.class);
        assertEquals(3.0, ext.getNullableQualityV().getStandardDeviation());
        assertTrue(ext.getNullableQualityV().isRedundant().isEmpty());
        // ... and keeps the redundancy of an existing one
        PropertyDispatcher.setDouble(h, OBS_QUALITY_P, 4.0);
        assertEquals(4.0, ext.getNullableQualityP().getStandardDeviation());
        assertEquals(Boolean.TRUE, ext.getNullableQualityP().isRedundant().orElseThrow());

        // A quality is an object of its own
        assertSame(ext.getNullableQualityP(), NetworkRegistry.lookup(p));
        PropertyDispatcher.setDouble(p, OBS_QUALITY_STANDARD_DEVIATION, 5.0);
        assertEquals(5.0, ext.getNullableQualityP().getStandardDeviation());
        PropertyDispatcher.setInt(p, OBS_QUALITY_REDUNDANT, 0);
        assertEquals(Boolean.FALSE, ext.getNullableQualityP().isRedundant().orElseThrow());
        PropertyDispatcher.setInt(p, OBS_QUALITY_REDUNDANT, -1);
        assertTrue(ext.getNullableQualityP().isRedundant().isEmpty());

        PropertyDispatcher.setBool(h, EXT_OBS_OBSERVABLE, 0);
        assertFalse(ext.isObservable());

        // Busbar sections are injections too
        long bbs = extension(network.getBusbarSection("BBS1"), INJECTION_OBSERVABILITY);
        long v = PropertyDispatcher.getRelated(bbs, OBS_QUALITY_V);
        assertEquals(0.8, PropertyDispatcher.getDouble(v, OBS_QUALITY_STANDARD_DEVIATION));
        assertEquals(0, PropertyDispatcher.getInt(v, OBS_QUALITY_REDUNDANT));
    }

    @Test void injectionObservabilityAbsent() {
        long load = reg(network.getLoad("L2"));
        assertEquals(0L, PropertyDispatcher.getRelated(load, INJECTION_OBSERVABILITY));
        // The extension's codes apply to the extension object only
        assertThrows(ClassCastException.class, () -> PropertyDispatcher.getBool(load, EXT_OBS_OBSERVABLE));
    }

    @Test void injectionObservabilityCreatedAndRemoved() {
        Load l2 = network.getLoad("L2");
        long host = reg(l2);
        long adder = PropertyDispatcher.invoke(host, INJECTION_OBSERVABILITY);
        PropertyDispatcher.setBool(adder, EXT_OBS_OBSERVABLE, 1);
        PropertyDispatcher.setDouble(adder, OBS_QUALITY_Q, 0.4);
        PropertyDispatcher.setInt(adder, OBS_QUALITY_Q, 1);
        long h = PropertyDispatcher.invoke(adder, ACT_ADD);
        InjectionObservability<Load> ext = l2.getExtension(InjectionObservability.class);
        assertSame(ext, NetworkRegistry.lookup(h));
        assertTrue(ext.isObservable());
        assertEquals(0.4, ext.getNullableQualityQ().getStandardDeviation());
        assertEquals(Boolean.TRUE, ext.getNullableQualityQ().isRedundant().orElseThrow());
        assertNull(ext.getNullableQualityP());

        PropertyDispatcher.setRelated(host, INJECTION_OBSERVABILITY, 0L);
        assertNull(l2.getExtension(InjectionObservability.class));
    }

    @Test void branchObservability() {
        Line line1 = network.getLine("LINE1");
        long h = extension(line1, BRANCH_OBSERVABILITY);
        assertEquals(0, PropertyDispatcher.getBool(h, INJECTION_OBSERVABILITY));
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_OBS_OBSERVABLE));
        long p1 = PropertyDispatcher.getRelated(h, OBS_QUALITY_P1);
        assertEquals(0.1, PropertyDispatcher.getDouble(p1, OBS_QUALITY_STANDARD_DEVIATION));
        assertEquals(0, PropertyDispatcher.getInt(p1, OBS_QUALITY_REDUNDANT));
        assertEquals(0L, PropertyDispatcher.getRelated(h, OBS_QUALITY_P2));
        assertEquals(0L, PropertyDispatcher.getRelated(h, OBS_QUALITY_Q1));
        assertEquals(0.2, PropertyDispatcher.getDouble(PropertyDispatcher.getRelated(h, OBS_QUALITY_Q2), OBS_QUALITY_STANDARD_DEVIATION));

        PropertyDispatcher.setDouble(h, OBS_QUALITY_Q1, 0.7);
        PropertyDispatcher.setInt(PropertyDispatcher.getRelated(h, OBS_QUALITY_Q1), OBS_QUALITY_REDUNDANT, 1);
        BranchObservability<Line> ext = line1.getExtension(BranchObservability.class);
        assertEquals(0.7, ext.getNullableQualityQ1().getStandardDeviation());
        assertEquals(Boolean.TRUE, ext.getNullableQualityQ1().isRedundant().orElseThrow());
        // The qualities of an injection are not those of a branch
        assertThrows(ClassCastException.class, () -> PropertyDispatcher.getRelated(h, OBS_QUALITY_P));
    }

    // ── Measurements ──────────────────────────────────────────────────────

    @Test void measurements() {
        long ext = extension(network.getGenerator("G1"), MEASUREMENTS);
        long[] meas = PropertyDispatcher.getChildren(ext, MEASUREMENT);
        assertEquals(2, meas.length);

        long p = PropertyDispatcher.getRelatedByString(ext, MEASUREMENT, "G1_P");
        assertNotEquals(0L, p);
        assertEquals(1, PropertyDispatcher.getBool(p, MEASUREMENT));
        assertEquals("G1_P", PropertyDispatcher.getString(p, MEAS_ID));
        assertEquals(Measurement.Type.ACTIVE_POWER.ordinal(), PropertyDispatcher.getInt(p, MEAS_TYPE));
        assertEquals(101.5, PropertyDispatcher.getDouble(p, MEAS_VALUE));
        assertEquals(1.0, PropertyDispatcher.getDouble(p, MEAS_STD_DEV));
        assertEquals(1, PropertyDispatcher.getBool(p, MEAS_VALID));
        assertEquals(-1, PropertyDispatcher.getInt(p, MEAS_SIDE));

        PropertyDispatcher.setDouble(p, MEAS_VALUE, 99.0);
        PropertyDispatcher.setDouble(p, MEAS_STD_DEV, 2.5);
        PropertyDispatcher.setBool(p, MEAS_VALID, 0);
        Measurement m = ((Measurements<?>) network.getGenerator("G1").getExtension(Measurements.class)).getMeasurement("G1_P");
        assertEquals(99.0, m.getValue());
        assertEquals(2.5, m.getStandardDeviation());
        assertFalse(m.isValid());

        assertEquals(0L, PropertyDispatcher.getRelatedByString(ext, MEASUREMENT, "UNKNOWN"));

        long line = extension(network.getLine("LINE1"), MEASUREMENTS);
        long i2 = PropertyDispatcher.getRelatedByString(line, MEASUREMENT, "LINE1_I2");
        assertEquals(Measurement.Type.CURRENT.ordinal(), PropertyDispatcher.getInt(i2, MEAS_TYPE));
        assertEquals(ThreeSides.TWO.ordinal(), PropertyDispatcher.getInt(i2, MEAS_SIDE));

        // Connectable without the extension
        assertEquals(0L, PropertyDispatcher.getRelated(reg(network.getLoad("L2")), MEASUREMENTS));
    }

    @Test void measurementsCreatedAndRemoved() {
        Load l2 = network.getLoad("L2");
        long ext = PropertyDispatcher.invoke(PropertyDispatcher.invoke(reg(l2), MEASUREMENTS), ACT_ADD);
        assertSame(l2.getExtension(Measurements.class), NetworkRegistry.lookup(ext));

        long adder = PropertyDispatcher.invoke(ext, MEASUREMENT);
        PropertyDispatcher.setString(adder, MEAS_ID, "L2_P");
        PropertyDispatcher.setInt(adder, MEAS_TYPE, Measurement.Type.ACTIVE_POWER.ordinal());
        PropertyDispatcher.setDouble(adder, MEAS_VALUE, 80.5);
        PropertyDispatcher.setDouble(adder, MEAS_STD_DEV, 0.5);
        PropertyDispatcher.setBool(adder, MEAS_VALID, 1);
        PropertyDispatcher.setKeyedString(adder, PROPERTY, "source", "scada");
        long m = PropertyDispatcher.invoke(adder, ACT_ADD);
        Measurement created = ((Measurements<?>) l2.getExtension(Measurements.class)).getMeasurement("L2_P");
        assertSame(created, NetworkRegistry.lookup(m));
        assertEquals(80.5, created.getValue());
        assertEquals("scada", created.getProperty("source"));

        // Its properties
        PropertyDispatcher.setKeyedString(m, PROPERTY, "quality", "good");
        assertArrayEquals(new String[] {"quality", "source"}, PropertyDispatcher.getStringList(m, PROPERTY_NAMES));
        assertEquals("good", PropertyDispatcher.getKeyedString(m, PROPERTY, "quality"));
        PropertyDispatcher.setKeyedString(m, PROPERTY, "quality", null);
        assertNull(created.getProperty("quality"));

        // The same id again is refused, unless a free one is derived from it
        long again = PropertyDispatcher.invoke(ext, MEASUREMENT);
        PropertyDispatcher.setString(again, MEAS_ID, "L2_P");
        PropertyDispatcher.setInt(again, MEAS_TYPE, Measurement.Type.REACTIVE_POWER.ordinal());
        PropertyDispatcher.setDouble(again, MEAS_VALUE, 5);
        assertThrows(PowsyblException.class, () -> PropertyDispatcher.invoke(again, ACT_ADD));
        PropertyDispatcher.setBool(again, ENSURE_ID_UNICITY, 1);
        long other = PropertyDispatcher.invoke(again, ACT_ADD);
        assertNotEquals("L2_P", PropertyDispatcher.getString(other, MEAS_ID));

        // A measurement may have no id: only an optional read gives it
        long anonymousAdder = PropertyDispatcher.invoke(ext, MEASUREMENT);
        PropertyDispatcher.setInt(anonymousAdder, MEAS_TYPE, Measurement.Type.VOLTAGE.ordinal());
        PropertyDispatcher.setDouble(anonymousAdder, MEAS_VALUE, 226);
        long anonymous = PropertyDispatcher.invoke(anonymousAdder, ACT_ADD);
        assertNull(PropertyDispatcher.getOptString(anonymous, MEAS_ID));
        assertThrows(BridgeErrors.PropertyNotFoundException.class, () -> PropertyDispatcher.getString(anonymous, MEAS_ID));
        assertEquals(3, PropertyDispatcher.getChildren(ext, MEASUREMENT).length);

        PropertyDispatcher.invoke(m, ACT_REMOVE);
        assertNull(((Measurements<?>) l2.getExtension(Measurements.class)).getMeasurement("L2_P"));
        // Cleaning keeps an extension that still has measurements, and removes an empty one
        PropertyDispatcher.invoke(ext, ACT_CLEAN_IF_EMPTY);
        assertNotNull(l2.getExtension(Measurements.class));
        PropertyDispatcher.invoke(other, ACT_REMOVE);
        PropertyDispatcher.invoke(anonymous, ACT_REMOVE);
        PropertyDispatcher.invoke(ext, ACT_CLEAN_IF_EMPTY);
        assertNull(l2.getExtension(Measurements.class));
    }

    // ── DiscreteMeasurements ──────────────────────────────────────────────

    @Test void discreteMeasurements() {
        long t2 = extension(network.getTwoWindingsTransformer("T2"), DISCRETE_MEASUREMENTS);
        long[] dms = PropertyDispatcher.getChildren(t2, DISCRETE_MEASUREMENT);
        assertEquals(1, dms.length);
        long tap = dms[0];
        assertEquals(1, PropertyDispatcher.getBool(tap, DISCRETE_MEASUREMENT));
        assertEquals("T2_TAP", PropertyDispatcher.getString(tap, DMEAS_ID));
        assertEquals(DiscreteMeasurement.Type.TAP_POSITION.ordinal(), PropertyDispatcher.getInt(tap, DMEAS_TYPE));
        assertEquals(DiscreteMeasurement.TapChanger.RATIO_TAP_CHANGER.ordinal(), PropertyDispatcher.getInt(tap, DMEAS_TAP_CHANGER));
        assertEquals(DiscreteMeasurement.ValueType.INT.ordinal(), PropertyDispatcher.getInt(tap, DMEAS_VALUE_TYPE));
        assertEquals(3, PropertyDispatcher.getInt(tap, DMEAS_VALUE_INT));
        PropertyDispatcher.setInt(tap, DMEAS_VALUE_INT, 2);
        assertEquals(2, PropertyDispatcher.getInt(tap, DMEAS_VALUE_INT));

        long sw = extension(network.getSwitch("SW1"), DISCRETE_MEASUREMENTS);
        long pos = PropertyDispatcher.getRelatedByString(sw, DISCRETE_MEASUREMENT, "SW1_POS");
        assertEquals(-1, PropertyDispatcher.getInt(pos, DMEAS_TAP_CHANGER));
        assertEquals(DiscreteMeasurement.ValueType.BOOLEAN.ordinal(), PropertyDispatcher.getInt(pos, DMEAS_VALUE_TYPE));
        assertEquals(1, PropertyDispatcher.getBool(pos, DMEAS_VALUE_BOOL));
        PropertyDispatcher.setBool(pos, DMEAS_VALUE_BOOL, 0);
        assertEquals(0, PropertyDispatcher.getBool(pos, DMEAS_VALUE_BOOL));

        long state = PropertyDispatcher.getRelatedByString(sw, DISCRETE_MEASUREMENT, "SW1_STATE");
        assertEquals("CLOSED", PropertyDispatcher.getString(state, DMEAS_VALUE_STRING));
        assertEquals(0, PropertyDispatcher.getBool(state, DMEAS_VALID));
        PropertyDispatcher.setString(state, DMEAS_VALUE_STRING, "OPEN");
        PropertyDispatcher.setBool(state, DMEAS_VALID, 1);
        assertEquals("OPEN", PropertyDispatcher.getString(state, DMEAS_VALUE_STRING));
        assertEquals(1, PropertyDispatcher.getBool(state, DMEAS_VALID));
    }

    @Test void discreteMeasurementCreatedAndRemoved() {
        long sw = extension(network.getSwitch("SW1"), DISCRETE_MEASUREMENTS);
        long adder = PropertyDispatcher.invoke(sw, DISCRETE_MEASUREMENT);
        PropertyDispatcher.setString(adder, DMEAS_ID, "SW1_COUNT");
        PropertyDispatcher.setInt(adder, DMEAS_TYPE, DiscreteMeasurement.Type.OTHER.ordinal());
        PropertyDispatcher.setInt(adder, DMEAS_VALUE_INT, 12);
        PropertyDispatcher.setBool(adder, DMEAS_VALID, 1);
        PropertyDispatcher.setKeyedString(adder, PROPERTY, "unit", "operations");
        long m = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(DiscreteMeasurement.ValueType.INT.ordinal(), PropertyDispatcher.getInt(m, DMEAS_VALUE_TYPE));
        assertEquals(12, PropertyDispatcher.getInt(m, DMEAS_VALUE_INT));
        assertEquals("operations", PropertyDispatcher.getKeyedString(m, PROPERTY, "unit"));
        assertEquals(3, PropertyDispatcher.getChildren(sw, DISCRETE_MEASUREMENT).length);

        PropertyDispatcher.invoke(m, ACT_REMOVE);
        assertEquals(2, PropertyDispatcher.getChildren(sw, DISCRETE_MEASUREMENT).length);
        // Not empty: kept
        PropertyDispatcher.invoke(sw, ACT_CLEAN_IF_EMPTY);
        assertNotNull(network.getSwitch("SW1").getExtension(DiscreteMeasurements.class));
        PropertyDispatcher.setRelated(reg(network.getSwitch("SW1")), DISCRETE_MEASUREMENTS, 0L);
        assertNull(network.getSwitch("SW1").getExtension(DiscreteMeasurements.class));
    }

    // ── ObservabilityArea ─────────────────────────────────────────────────

    @Test void observabilityAreaBusBreaker() {
        long ext = extension(network.getVoltageLevel("VL1"), OBSERVABILITY_AREA);
        // SW1 is closed: B1 and B2 form a single bus view bus but belong to different areas
        assertEquals(0, PropertyDispatcher.getBool(ext, EXT_OBS_AREA_CONSISTENT));
        long[] areas = PropertyDispatcher.getChildren(ext, AREA_CHARACTERISTICS);
        assertEquals(2, areas.length);
        assertEquals(1, PropertyDispatcher.getBool(areas[0], AREA_CHARACTERISTICS));
        assertEquals(1, PropertyDispatcher.getInt(areas[0], OBS_AREA_NUMBER)); // by area number

        long border = PropertyDispatcher.getRelatedByString(ext, REL_OBS_AREA_BY_BBV_BUS, "B2");
        assertEquals(2, PropertyDispatcher.getInt(border, OBS_AREA_NUMBER));
        assertEquals(ObservabilityArea.ObservabilityStatus.BORDER.ordinal(), PropertyDispatcher.getInt(border, OBS_AREA_STATUS));
        assertEquals(0, PropertyDispatcher.getIntList(border, OBS_AREA_NODES).length);
        long[] buses = PropertyDispatcher.getChildren(border, OBS_AREA_BUSES);
        assertEquals(1, buses.length);
        assertEquals("B2", PropertyDispatcher.getString(buses[0], ID));
        long[] terminals = PropertyDispatcher.getChildren(border, OBS_AREA_TERMINALS);
        assertEquals(1, terminals.length); // T3 leg 1
        assertEquals("T3", connectableId(terminals[0]));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(ext, REL_OBS_AREA_BY_BBV_BUS, "UNKNOWN"));

        // Sorted by connectable id, so the same on every run (powsybl gives them as a HashSet)
        long observable = PropertyDispatcher.getRelatedByString(ext, REL_OBS_AREA_BY_BBV_BUS, "B1");
        assertEquals(java.util.List.of("G1", "LINE1", "T2"), Arrays.stream(PropertyDispatcher.getChildren(observable, OBS_AREA_TERMINALS))
            .mapToObj(StateEstimationDispatchTest::connectableId).toList());

        assertEquals(0L, PropertyDispatcher.getRelated(reg(network.getVoltageLevel("VL2")), OBSERVABILITY_AREA));
    }

    @Test void observabilityAreaNodeBreaker() {
        long ext = extension(network.getVoltageLevel("VL4"), OBSERVABILITY_AREA);
        assertEquals(1, PropertyDispatcher.getBool(ext, EXT_OBS_AREA_CONSISTENT));
        long area = PropertyDispatcher.getRelatedByIndex(ext, REL_OBS_AREA_BY_NODE, 1);
        assertNotEquals(0L, area);
        assertEquals(7, PropertyDispatcher.getInt(area, OBS_AREA_NUMBER));
        assertEquals(ObservabilityArea.ObservabilityStatus.NON_OBSERVABLE.ordinal(), PropertyDispatcher.getInt(area, OBS_AREA_STATUS));
        assertArrayEquals(new int[] {0, 1, 2}, PropertyDispatcher.getIntList(area, OBS_AREA_NODES));
        long[] terminals = PropertyDispatcher.getChildren(area, OBS_AREA_TERMINALS);
        assertEquals(Set.of("BBS1", "L4", "LINE1"), Arrays.stream(terminals)
            .mapToObj(StateEstimationDispatchTest::connectableId)
            .collect(java.util.stream.Collectors.toSet()));

        PropertyDispatcher.setRelated(reg(network.getVoltageLevel("VL4")), OBSERVABILITY_AREA, 0L);
        assertNull(network.getVoltageLevel("VL4").getExtension(ObservabilityArea.class));
    }

    private static String connectableId(long terminal) {
        return PropertyDispatcher.getString(PropertyDispatcher.getRelated(terminal, REL_CONNECTABLE), ID);
    }

    // ── Object-based API (JNI fallback) ───────────────────────────────────

    @Test void objectApiMatchesHandleApi() {
        Generator g1 = network.getGenerator("G1");
        Object observability = PropertyDispatcher.getRelatedObject(g1, INJECTION_OBSERVABILITY);
        Object qualityP = PropertyDispatcher.getRelatedObject(observability, OBS_QUALITY_P);
        assertEquals(1.5, PropertyDispatcher.getDouble(qualityP, OBS_QUALITY_STANDARD_DEVIATION));
        Object measurements = PropertyDispatcher.getRelatedObject(g1, MEASUREMENTS);
        assertEquals(2, PropertyDispatcher.getChildObjects(measurements, MEASUREMENT).length);
        Object p = PropertyDispatcher.getRelatedObjectByString(measurements, MEASUREMENT, "G1_P");
        assertEquals(101.5, PropertyDispatcher.getDouble(p, MEAS_VALUE));
        PropertyDispatcher.setBool(p, MEAS_VALID, false);
        assertFalse(PropertyDispatcher.getBool(p, MEAS_VALID));
        Object areas = PropertyDispatcher.getRelatedObject(network.getVoltageLevel("VL4"), OBSERVABILITY_AREA);
        Object area = PropertyDispatcher.getRelatedObjectByIndex(areas, REL_OBS_AREA_BY_NODE, 0);
        assertArrayEquals(new int[] {0, 1, 2}, PropertyDispatcher.getIntList(area, OBS_AREA_NODES));
    }
}
