package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.util.Arrays;
import java.util.Set;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * State estimation extensions through {@link PropertyDispatcher}: the handle-based API (GraalVM
 * backend) and the object-based API (JNI backend fallback).
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
            .setMinP(0).setMaxP(500).setTargetP(100).setTargetV(400).setVoltageRegulatorOn(true).add();
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

    // ── ToBeEstimated ─────────────────────────────────────────────────────

    @Test void twoWindingsTransformerToBeEstimated() {
        long h = reg(network.getTwoWindingsTransformer("T2"));
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_TWO_WT_TBE_EXISTS));
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_TWO_WT_TBE_RTC));
        assertEquals(0, PropertyDispatcher.getBool(h, EXT_TWO_WT_TBE_PTC));
        PropertyDispatcher.setBool(h, EXT_TWO_WT_TBE_PTC, 1);
        assertTrue(network.getTwoWindingsTransformer("T2")
            .getExtension(TwoWindingsTransformerToBeEstimated.class).shouldEstimatePhaseTapChanger());
        // No extension on a line-less 2WT is reported as absent, not as an error
        assertEquals(0, PropertyDispatcher.getBool(reg(network.getThreeWindingsTransformer("T3")), EXT_TWO_WT_TBE_EXISTS));
    }

    @Test void threeWindingsTransformerToBeEstimated() {
        ThreeWindingsTransformer t3 = network.getThreeWindingsTransformer("T3");
        long h = reg(t3);
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_THREE_WT_TBE_EXISTS));
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

    @Test void toBeEstimatedAbsentThrows() {
        long h = reg(network.getTwoWindingsTransformer("T2"));
        network.getTwoWindingsTransformer("T2").removeExtension(TwoWindingsTransformerToBeEstimated.class);
        assertEquals(0, PropertyDispatcher.getBool(h, EXT_TWO_WT_TBE_EXISTS));
        assertThrows(IllegalStateException.class, () -> PropertyDispatcher.getBool(h, EXT_TWO_WT_TBE_RTC));
    }

    // ── Observability ─────────────────────────────────────────────────────

    @Test void injectionObservability() {
        Generator g1 = network.getGenerator("G1");
        long h = reg(g1);
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_INJ_OBS_EXISTS));
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_INJ_OBS_OBSERVABLE));
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_EXISTS_OFF));
        assertEquals(1.5, PropertyDispatcher.getDouble(h, EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_STD_DEV_OFF));
        assertEquals(1, PropertyDispatcher.getInt(h, EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_REDUNDANT_OFF));
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_INJ_OBS_QUALITY_Q + OBS_QUALITY_EXISTS_OFF));
        assertEquals(-1, PropertyDispatcher.getInt(h, EXT_INJ_OBS_QUALITY_Q + OBS_QUALITY_REDUNDANT_OFF));
        assertEquals(0, PropertyDispatcher.getBool(h, EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_EXISTS_OFF));

        // Setting the standard deviation of an undefined quality creates it
        PropertyDispatcher.setDouble(h, EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_STD_DEV_OFF, 3.0);
        InjectionObservability<Generator> ext = g1.getExtension(InjectionObservability.class);
        assertEquals(3.0, ext.getNullableQualityV().getStandardDeviation());
        assertTrue(ext.getNullableQualityV().isRedundant().isEmpty());
        // ... and keeps the redundancy of an existing one
        PropertyDispatcher.setDouble(h, EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_STD_DEV_OFF, 4.0);
        assertEquals(4.0, ext.getNullableQualityP().getStandardDeviation());
        assertEquals(Boolean.TRUE, ext.getNullableQualityP().isRedundant().orElseThrow());

        PropertyDispatcher.setInt(h, EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_REDUNDANT_OFF, 0);
        assertEquals(Boolean.FALSE, ext.getNullableQualityP().isRedundant().orElseThrow());
        PropertyDispatcher.setInt(h, EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_REDUNDANT_OFF, -1);
        assertTrue(ext.getNullableQualityP().isRedundant().isEmpty());

        PropertyDispatcher.setBool(h, EXT_INJ_OBS_OBSERVABLE, 0);
        assertFalse(ext.isObservable());

        // Busbar sections are injections too
        long bbs = reg(network.getBusbarSection("BBS1"));
        assertEquals(0.8, PropertyDispatcher.getDouble(bbs, EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_STD_DEV_OFF));
        assertEquals(0, PropertyDispatcher.getInt(bbs, EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_REDUNDANT_OFF));
    }

    @Test void injectionObservabilityUndefinedQualityThrows() {
        long h = reg(network.getGenerator("G1"));
        assertThrows(BridgeErrors.PropertyNotFoundException.class,
            () -> PropertyDispatcher.getDouble(h, EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_STD_DEV_OFF));
        long load = reg(network.getLoad("L2"));
        assertEquals(0, PropertyDispatcher.getBool(load, EXT_INJ_OBS_EXISTS));
        assertThrows(BridgeErrors.ExtensionNotFoundException.class, () -> PropertyDispatcher.getBool(load, EXT_INJ_OBS_OBSERVABLE));
    }

    @Test void branchObservability() {
        Line line1 = network.getLine("LINE1");
        long h = reg(line1);
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_BRANCH_OBS_EXISTS));
        assertEquals(1, PropertyDispatcher.getBool(h, EXT_BRANCH_OBS_OBSERVABLE));
        assertEquals(0.1, PropertyDispatcher.getDouble(h, EXT_BRANCH_OBS_QUALITY_P1 + OBS_QUALITY_STD_DEV_OFF));
        assertEquals(0, PropertyDispatcher.getInt(h, EXT_BRANCH_OBS_QUALITY_P1 + OBS_QUALITY_REDUNDANT_OFF));
        assertEquals(0, PropertyDispatcher.getBool(h, EXT_BRANCH_OBS_QUALITY_P2 + OBS_QUALITY_EXISTS_OFF));
        assertEquals(0, PropertyDispatcher.getBool(h, EXT_BRANCH_OBS_QUALITY_Q1 + OBS_QUALITY_EXISTS_OFF));
        assertEquals(0.2, PropertyDispatcher.getDouble(h, EXT_BRANCH_OBS_QUALITY_Q2 + OBS_QUALITY_STD_DEV_OFF));

        PropertyDispatcher.setDouble(h, EXT_BRANCH_OBS_QUALITY_Q1 + OBS_QUALITY_STD_DEV_OFF, 0.7);
        PropertyDispatcher.setInt(h, EXT_BRANCH_OBS_QUALITY_Q1 + OBS_QUALITY_REDUNDANT_OFF, 1);
        BranchObservability<Line> ext = line1.getExtension(BranchObservability.class);
        assertEquals(0.7, ext.getNullableQualityQ1().getStandardDeviation());
        assertEquals(Boolean.TRUE, ext.getNullableQualityQ1().isRedundant().orElseThrow());
    }

    // ── Measurements ──────────────────────────────────────────────────────

    @Test void measurements() {
        long g1 = reg(network.getGenerator("G1"));
        assertEquals(1, PropertyDispatcher.getBool(g1, EXT_MEAS_EXISTS));
        long[] meas = PropertyDispatcher.getChildren(g1, MEASUREMENT);
        assertEquals(2, meas.length);

        long p = PropertyDispatcher.getRelatedByString(g1, REL_MEASUREMENT_BY_ID, "G1_P");
        assertNotEquals(0L, p);
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

        assertEquals(0L, PropertyDispatcher.getRelatedByString(g1, REL_MEASUREMENT_BY_ID, "UNKNOWN"));

        long line = reg(network.getLine("LINE1"));
        long i2 = PropertyDispatcher.getRelatedByString(line, REL_MEASUREMENT_BY_ID, "LINE1_I2");
        assertEquals(Measurement.Type.CURRENT.ordinal(), PropertyDispatcher.getInt(i2, MEAS_TYPE));
        assertEquals(ThreeSides.TWO.ordinal(), PropertyDispatcher.getInt(i2, MEAS_SIDE));

        // Connectable without the extension: no measurements, no error
        long load = reg(network.getLoad("L2"));
        assertEquals(0, PropertyDispatcher.getBool(load, EXT_MEAS_EXISTS));
        assertEquals(0, PropertyDispatcher.getChildren(load, MEASUREMENT).length);
        assertEquals(0L, PropertyDispatcher.getRelatedByString(load, REL_MEASUREMENT_BY_ID, "G1_P"));
    }

    // ── DiscreteMeasurements ──────────────────────────────────────────────

    @Test void discreteMeasurements() {
        long t2 = reg(network.getTwoWindingsTransformer("T2"));
        assertEquals(1, PropertyDispatcher.getBool(t2, EXT_DMEAS_EXISTS));
        long[] dms = PropertyDispatcher.getChildren(t2, DISCRETE_MEASUREMENT);
        assertEquals(1, dms.length);
        long tap = dms[0];
        assertEquals("T2_TAP", PropertyDispatcher.getString(tap, DMEAS_ID));
        assertEquals(DiscreteMeasurement.Type.TAP_POSITION.ordinal(), PropertyDispatcher.getInt(tap, DMEAS_TYPE));
        assertEquals(DiscreteMeasurement.TapChanger.RATIO_TAP_CHANGER.ordinal(), PropertyDispatcher.getInt(tap, DMEAS_TAP_CHANGER));
        assertEquals(DiscreteMeasurement.ValueType.INT.ordinal(), PropertyDispatcher.getInt(tap, DMEAS_VALUE_TYPE));
        assertEquals(3, PropertyDispatcher.getInt(tap, DMEAS_VALUE_INT));
        PropertyDispatcher.setInt(tap, DMEAS_VALUE_INT, 2);
        assertEquals(2, PropertyDispatcher.getInt(tap, DMEAS_VALUE_INT));

        long sw = reg(network.getSwitch("SW1"));
        long pos = PropertyDispatcher.getRelatedByString(sw, REL_DISCRETE_MEASUREMENT_BY_ID, "SW1_POS");
        assertEquals(-1, PropertyDispatcher.getInt(pos, DMEAS_TAP_CHANGER));
        assertEquals(DiscreteMeasurement.ValueType.BOOLEAN.ordinal(), PropertyDispatcher.getInt(pos, DMEAS_VALUE_TYPE));
        assertEquals(1, PropertyDispatcher.getBool(pos, DMEAS_VALUE_BOOL));
        PropertyDispatcher.setBool(pos, DMEAS_VALUE_BOOL, 0);
        assertEquals(0, PropertyDispatcher.getBool(pos, DMEAS_VALUE_BOOL));

        long state = PropertyDispatcher.getRelatedByString(sw, REL_DISCRETE_MEASUREMENT_BY_ID, "SW1_STATE");
        assertEquals("CLOSED", PropertyDispatcher.getString(state, DMEAS_VALUE_STRING));
        assertEquals(0, PropertyDispatcher.getBool(state, DMEAS_VALID));
        PropertyDispatcher.setString(state, DMEAS_VALUE_STRING, "OPEN");
        PropertyDispatcher.setBool(state, DMEAS_VALID, 1);
        assertEquals("OPEN", PropertyDispatcher.getString(state, DMEAS_VALUE_STRING));
        assertEquals(1, PropertyDispatcher.getBool(state, DMEAS_VALID));
    }

    // ── ObservabilityArea ─────────────────────────────────────────────────

    @Test void observabilityAreaBusBreaker() {
        long vl1 = reg(network.getVoltageLevel("VL1"));
        assertEquals(1, PropertyDispatcher.getBool(vl1, EXT_OBS_AREA_EXISTS));
        // SW1 is closed: B1 and B2 form a single bus view bus but belong to different areas
        assertEquals(0, PropertyDispatcher.getBool(vl1, EXT_OBS_AREA_CONSISTENT));
        long[] areas = PropertyDispatcher.getChildren(vl1, OBSERVABILITY_AREA);
        assertEquals(2, areas.length);

        long border = PropertyDispatcher.getRelatedByString(vl1, REL_OBS_AREA_BY_BBV_BUS, "B2");
        assertEquals(2, PropertyDispatcher.getInt(border, OBS_AREA_NUMBER));
        assertEquals(ObservabilityArea.ObservabilityStatus.BORDER.ordinal(), PropertyDispatcher.getInt(border, OBS_AREA_STATUS));
        assertEquals(0, PropertyDispatcher.getIntList(border, OBS_AREA_NODES).length);
        long[] buses = PropertyDispatcher.getChildren(border, OBS_AREA_BUSES);
        assertEquals(1, buses.length);
        assertEquals("B2", PropertyDispatcher.getString(buses[0], ID));
        long[] terminals = PropertyDispatcher.getChildren(border, OBS_AREA_TERMINALS);
        assertEquals(1, terminals.length); // T3 leg 1
        assertEquals("T3", PropertyDispatcher.getString(terminals[0], TERMINAL_CONNECTABLE_ID));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(vl1, REL_OBS_AREA_BY_BBV_BUS, "UNKNOWN"));

        // Sorted by connectable id, so the same on every run (powsybl gives them as a HashSet)
        long observable = PropertyDispatcher.getRelatedByString(vl1, REL_OBS_AREA_BY_BBV_BUS, "B1");
        assertEquals(java.util.List.of("G1", "LINE1", "T2"), Arrays.stream(PropertyDispatcher.getChildren(observable, OBS_AREA_TERMINALS))
            .mapToObj(t -> PropertyDispatcher.getString(t, TERMINAL_CONNECTABLE_ID)).toList());

        long vl2 = reg(network.getVoltageLevel("VL2"));
        assertEquals(0, PropertyDispatcher.getBool(vl2, EXT_OBS_AREA_EXISTS));
        assertEquals(0, PropertyDispatcher.getChildren(vl2, OBSERVABILITY_AREA).length);
    }

    @Test void observabilityAreaNodeBreaker() {
        long vl4 = reg(network.getVoltageLevel("VL4"));
        assertEquals(1, PropertyDispatcher.getBool(vl4, EXT_OBS_AREA_CONSISTENT));
        long area = PropertyDispatcher.getRelatedByIndex(vl4, REL_OBS_AREA_BY_NODE, 1);
        assertNotEquals(0L, area);
        assertEquals(7, PropertyDispatcher.getInt(area, OBS_AREA_NUMBER));
        assertEquals(ObservabilityArea.ObservabilityStatus.NON_OBSERVABLE.ordinal(), PropertyDispatcher.getInt(area, OBS_AREA_STATUS));
        assertArrayEquals(new int[] {0, 1, 2}, PropertyDispatcher.getIntList(area, OBS_AREA_NODES));
        long[] terminals = PropertyDispatcher.getChildren(area, OBS_AREA_TERMINALS);
        assertEquals(Set.of("BBS1", "L4", "LINE1"), Arrays.stream(terminals)
            .mapToObj(t -> PropertyDispatcher.getString(t, TERMINAL_CONNECTABLE_ID))
            .collect(java.util.stream.Collectors.toSet()));
    }

    // ── Object-based API (JNI fallback) ───────────────────────────────────

    @Test void objectApiMatchesHandleApi() {
        Generator g1 = network.getGenerator("G1");
        assertTrue(PropertyDispatcher.getBool(g1, EXT_INJ_OBS_EXISTS));
        assertEquals(1.5, PropertyDispatcher.getDouble(g1, EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_STD_DEV_OFF));
        Object[] meas = PropertyDispatcher.getChildObjects(g1, MEASUREMENT);
        assertEquals(2, meas.length);
        Object p = PropertyDispatcher.getRelatedObjectByString(g1, REL_MEASUREMENT_BY_ID, "G1_P");
        assertEquals(101.5, PropertyDispatcher.getDouble(p, MEAS_VALUE));
        PropertyDispatcher.setBool(p, MEAS_VALID, false);
        assertFalse(PropertyDispatcher.getBool(p, MEAS_VALID));
        Object area = PropertyDispatcher.getRelatedObjectByIndex(network.getVoltageLevel("VL4"), REL_OBS_AREA_BY_NODE, 0);
        assertArrayEquals(new int[] {0, 1, 2}, PropertyDispatcher.getIntList(area, OBS_AREA_NODES));
    }
}
