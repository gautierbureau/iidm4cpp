package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.ShuntCompensatorNonLinearModel;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Exercises every branch of PropertyDispatcher.getDouble().
 *
 * Previously LCC_LOSS_FACTOR (1400), LCC_POWER_FACTOR (1401), and
 * VSC_LOSS_FACTOR (1300) were missing from the switch, causing
 * IllegalArgumentException at runtime (caught by Dynawo unit tests).
 */
class PropertyDispatcherGetDoubleTest {

    private Network network;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("test", "test");
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    // ── Helpers ───────────────────────────────────────────────────────────────

    private long reg(Object obj) { return NetworkRegistry.register(obj); }

    private Substation addSubstation() {
        return network.newSubstation().setId("S").setCountry(Country.FR).add();
    }

    private VoltageLevel addVoltageLevel(Substation sub, double nomV) {
        return sub.newVoltageLevel()
                .setId("VL").setNominalV(nomV)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
    }

    private Bus addBus(VoltageLevel vl) {
        return vl.getBusBreakerView().newBus().setId("B").add();
    }

    // ── Generator ────────────────────────────────────────────────────────────

    @Test void gen_targetP() {
        Generator g = buildGenerator(100, 20, 50, 200);
        assertEquals(100.0, PropertyDispatcher.getDouble(reg(g), GEN_TARGET_P));
    }

    @Test void gen_targetQ() {
        Generator g = buildGenerator(100, 20, 50, 200);
        assertEquals(20.0, PropertyDispatcher.getDouble(reg(g), GEN_TARGET_Q));
    }

    @Test void gen_targetV() {
        Generator g = buildGenerator(100, 20, 50, 200);
        assertEquals(400.0, PropertyDispatcher.getDouble(reg(g), GEN_TARGET_V));
    }

    @Test void gen_minP() {
        Generator g = buildGenerator(100, 20, 50, 200);
        assertEquals(50.0, PropertyDispatcher.getDouble(reg(g), GEN_MIN_P));
    }

    @Test void gen_maxP() {
        Generator g = buildGenerator(100, 20, 50, 200);
        assertEquals(200.0, PropertyDispatcher.getDouble(reg(g), GEN_MAX_P));
    }

    private Generator buildGenerator(double targetP, double targetQ, double minP, double maxP) {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 400);
        Bus b = addBus(vl);
        return vl.newGenerator()
                .setId("G").setBus(b.getId()).setConnectableBus(b.getId())
                .setTargetP(targetP).setTargetQ(targetQ).setTargetV(400.0)
                .setMinP(minP).setMaxP(maxP).setVoltageRegulatorOn(true)
                .add();
    }

    // ── Load ─────────────────────────────────────────────────────────────────

    @Test void load_p0() {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 400);
        Bus b = addBus(vl);
        Load load = vl.newLoad().setId("L").setBus(b.getId()).setConnectableBus(b.getId())
                .setP0(50).setQ0(10).add();
        assertEquals(50.0, PropertyDispatcher.getDouble(reg(load), LOAD_P0));
        assertEquals(10.0, PropertyDispatcher.getDouble(reg(load), LOAD_Q0));
    }

    // ── Terminal ─────────────────────────────────────────────────────────────

    @Test void terminal_pq() {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 400);
        Bus b = addBus(vl);
        Load load = vl.newLoad().setId("L").setBus(b.getId()).setConnectableBus(b.getId())
                .setP0(0).setQ0(0).add();
        Terminal t = load.getTerminal();
        t.setP(11.0).setQ(22.0);
        assertEquals(11.0, PropertyDispatcher.getDouble(reg(t), TERMINAL_P));
        assertEquals(22.0, PropertyDispatcher.getDouble(reg(t), TERMINAL_Q));
    }

    // ── Bus ──────────────────────────────────────────────────────────────────

    @Test void bus_vAngle() {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 400);
        Bus b = addBus(vl);
        b.setV(380).setAngle(5);
        assertEquals(380.0, PropertyDispatcher.getDouble(reg(b), BUS_V));
        assertEquals(5.0,   PropertyDispatcher.getDouble(reg(b), BUS_ANGLE));
    }

    // ── VoltageLevel ─────────────────────────────────────────────────────────

    @Test void vl_nominalV() {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 380);
        assertEquals(380.0, PropertyDispatcher.getDouble(reg(vl), VL_NOMINAL_V));
    }

    // ── Line ─────────────────────────────────────────────────────────────────

    @Test void line_params() {
        Substation s1 = network.newSubstation().setId("S1").setCountry(Country.FR).add();
        Substation s2 = network.newSubstation().setId("S2").setCountry(Country.DE).add();
        VoltageLevel vl1 = s1.newVoltageLevel().setId("VL1").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        VoltageLevel vl2 = s2.newVoltageLevel().setId("VL2").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b1 = vl1.getBusBreakerView().newBus().setId("B1").add();
        Bus b2 = vl2.getBusBreakerView().newBus().setId("B2").add();
        Line line = network.newLine().setId("L12")
                .setVoltageLevel1(vl1.getId()).setBus1(b1.getId()).setConnectableBus1(b1.getId())
                .setVoltageLevel2(vl2.getId()).setBus2(b2.getId()).setConnectableBus2(b2.getId())
                .setR(1).setX(10).setG1(0).setB1(1e-4).setG2(0).setB2(1e-4).add();
        assertEquals(1.0,   PropertyDispatcher.getDouble(reg(line), LINE_R));
        assertEquals(10.0,  PropertyDispatcher.getDouble(reg(line), LINE_X));
        assertEquals(1e-4,  PropertyDispatcher.getDouble(reg(line), LINE_B1));
        assertEquals(1e-4,  PropertyDispatcher.getDouble(reg(line), LINE_B2));
    }

    // ── HvdcLine ─────────────────────────────────────────────────────────────

    @Test void hvdc_params() {
        HvdcLine hvdc = buildHvdcLine();
        assertEquals(0.5,   PropertyDispatcher.getDouble(reg(hvdc), HVDC_R));
        assertEquals(400.0, PropertyDispatcher.getDouble(reg(hvdc), HVDC_NOMINAL_V));
        assertEquals(300.0, PropertyDispatcher.getDouble(reg(hvdc), HVDC_ACTIVE_POWER_SETPOINT));
        assertEquals(500.0, PropertyDispatcher.getDouble(reg(hvdc), HVDC_MAX_P));
    }

    // ── LccConverterStation — the previously broken cases ────────────────────

    @Test void lcc_lossFactor() {
        LccConverterStation lcc = buildIsolatedLcc(0.03f, 0.90f);
        assertEquals(0.03, PropertyDispatcher.getDouble(reg(lcc), LCC_LOSS_FACTOR), 1e-6,
                "LCC_LOSS_FACTOR (1400) was previously missing from getDouble");
    }

    @Test void lcc_powerFactor() {
        LccConverterStation lcc = buildIsolatedLcc(0.02f, 0.85f);
        assertEquals(0.85, PropertyDispatcher.getDouble(reg(lcc), LCC_POWER_FACTOR), 1e-6,
                "LCC_POWER_FACTOR (1401) was previously missing from getDouble");
    }

    // ── VscConverterStation — the previously broken case ─────────────────────

    @Test void vsc_lossFactor() {
        VscConverterStation vsc = buildVscConverterStation(0.011f);
        assertEquals(0.011, PropertyDispatcher.getDouble(reg(vsc), VSC_LOSS_FACTOR), 1e-6,
                "VSC_LOSS_FACTOR (1300) was previously missing from getDouble");
    }

    @Test void vsc_voltageSetpoint() {
        VscConverterStation vsc = buildVscConverterStation(0.011f);
        assertEquals(400.0, PropertyDispatcher.getDouble(reg(vsc), VSC_VOLTAGE_SETPOINT));
    }

    @Test void vsc_reactivePowerSetpoint() {
        VscConverterStation vsc = buildVscConverterStation(0.011f);
        assertEquals(50.0, PropertyDispatcher.getDouble(reg(vsc), VSC_REACTIVE_POWER_SETPOINT));
    }

    // ── StaticVarCompensator ─────────────────────────────────────────────────

    @Test void svc_params() {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 400);
        Bus b = addBus(vl);
        // SVC regulation requires a terminal — use a load on the same VL
        Load refLoad = vl.newLoad().setId("LSVC").setBus(b.getId()).setConnectableBus(b.getId())
                .setP0(0).setQ0(0).add();
        StaticVarCompensator svc = vl.newStaticVarCompensator().setId("SVC")
                .setBus(b.getId()).setConnectableBus(b.getId())
                .setBmin(-1e-4).setBmax(1e-4)
                .setVoltageSetpoint(400).setReactivePowerSetpoint(0)
                .setRegulationMode(StaticVarCompensator.RegulationMode.VOLTAGE)
                .setRegulatingTerminal(refLoad.getTerminal())
                .setRegulating(true)
                .add();
        assertEquals(-1e-4, PropertyDispatcher.getDouble(reg(svc), SVC_B_MIN), 1e-9);
        assertEquals( 1e-4, PropertyDispatcher.getDouble(reg(svc), SVC_B_MAX), 1e-9);
        assertEquals(400.0, PropertyDispatcher.getDouble(reg(svc), SVC_VOLTAGE_SETPOINT));
    }

    // ── DanglingLine ─────────────────────────────────────────────────────────

    @Test void danglingLine_params() {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 400);
        Bus b = addBus(vl);
        DanglingLine dl = vl.newDanglingLine().setId("DL")
                .setBus(b.getId()).setConnectableBus(b.getId())
                .setP0(10).setQ0(5).setR(1).setX(5).setG(0).setB(1e-5).add();
        assertEquals(10.0, PropertyDispatcher.getDouble(reg(dl), DL_P0));
        assertEquals(5.0,  PropertyDispatcher.getDouble(reg(dl), DL_Q0));
        assertEquals(1.0,  PropertyDispatcher.getDouble(reg(dl), DL_R));
        assertEquals(5.0,  PropertyDispatcher.getDouble(reg(dl), DL_X));
        assertEquals(1e-5, PropertyDispatcher.getDouble(reg(dl), DL_B), 1e-10);
    }

    // ── ShuntCompensator ─────────────────────────────────────────────────────

    @Test void shunt_linear() {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 400);
        Bus b = addBus(vl);
        ShuntCompensator sc = vl.newShuntCompensator().setId("SC")
                .setBus(b.getId()).setConnectableBus(b.getId())
                .setSectionCount(1)
                .newLinearModel().setBPerSection(1e-4).setMaximumSectionCount(3).add()
                .add();
        // SHUNT_B_PER_SECTION and SHUNT_G_PER_SECTION delegate to ShuntCompensator.getB()/getG()
        // which returns B for the current section count (not per-section value directly)
        assertDoesNotThrow(() -> PropertyDispatcher.getDouble(reg(sc), SHUNT_B_PER_SECTION));
        assertDoesNotThrow(() -> PropertyDispatcher.getDouble(reg(sc), SHUNT_B));
    }

    // ── TwoWindingsTransformer ────────────────────────────────────────────────

    @Test void twoWT_params() {
        Substation s = addSubstation();
        VoltageLevel vl1 = s.newVoltageLevel().setId("VL1H").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        VoltageLevel vl2 = s.newVoltageLevel().setId("VL1L").setNominalV(20)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b1 = vl1.getBusBreakerView().newBus().setId("BH").add();
        Bus b2 = vl2.getBusBreakerView().newBus().setId("BL").add();
        TwoWindingsTransformer twt = s.newTwoWindingsTransformer().setId("TWT")
                .setVoltageLevel1(vl1.getId()).setBus1(b1.getId()).setConnectableBus1(b1.getId())
                .setVoltageLevel2(vl2.getId()).setBus2(b2.getId()).setConnectableBus2(b2.getId())
                .setR(0.5).setX(10).setG(0).setB(0).setRatedU1(400).setRatedU2(20).add();
        assertEquals(0.5,  PropertyDispatcher.getDouble(reg(twt), TWO_WT_R));
        assertEquals(10.0, PropertyDispatcher.getDouble(reg(twt), TWO_WT_X));
        assertEquals(400.0,PropertyDispatcher.getDouble(reg(twt), TWO_WT_RATED_U1));
        assertEquals(20.0, PropertyDispatcher.getDouble(reg(twt), TWO_WT_RATED_U2));
    }

    // ── Battery ───────────────────────────────────────────────────────────────

    @Test void battery_params() {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 400);
        Bus b = addBus(vl);
        Battery bat = vl.newBattery().setId("BAT")
                .setBus(b.getId()).setConnectableBus(b.getId())
                .setTargetP(30).setTargetQ(5).setMinP(0).setMaxP(50).add();
        assertEquals(30.0, PropertyDispatcher.getDouble(reg(bat), BAT_TARGET_P));
        assertEquals(5.0,  PropertyDispatcher.getDouble(reg(bat), BAT_TARGET_Q));
        assertEquals(0.0,  PropertyDispatcher.getDouble(reg(bat), BAT_MIN_P));
        assertEquals(50.0, PropertyDispatcher.getDouble(reg(bat), BAT_MAX_P));
    }

    // ── ThreeWindingsTransformer ──────────────────────────────────────────────

    @Test void threeWT_ratedU0() {
        ThreeWindingsTransformer twt3 = buildThreeWT();
        assertDoesNotThrow(() -> PropertyDispatcher.getDouble(reg(twt3), THREE_WT_RATED_U0));
    }

    @Test void threeWT_leg1_params() {
        ThreeWindingsTransformer twt3 = buildThreeWT();
        assertEquals(1.0, PropertyDispatcher.getDouble(reg(twt3), THREE_WT_LEG1_R));
        assertEquals(10.0, PropertyDispatcher.getDouble(reg(twt3), THREE_WT_LEG1_X));
        assertEquals(400.0, PropertyDispatcher.getDouble(reg(twt3), THREE_WT_LEG1_RATED_U));
    }

    @Test void threeWT_leg2_params() {
        ThreeWindingsTransformer twt3 = buildThreeWT();
        assertEquals(2.0, PropertyDispatcher.getDouble(reg(twt3), THREE_WT_LEG2_R));
        assertEquals(20.0, PropertyDispatcher.getDouble(reg(twt3), THREE_WT_LEG2_X));
        assertEquals(220.0, PropertyDispatcher.getDouble(reg(twt3), THREE_WT_LEG2_RATED_U));
    }

    @Test void threeWT_leg3_params() {
        ThreeWindingsTransformer twt3 = buildThreeWT();
        assertEquals(3.0, PropertyDispatcher.getDouble(reg(twt3), THREE_WT_LEG3_R));
        assertEquals(30.0, PropertyDispatcher.getDouble(reg(twt3), THREE_WT_LEG3_X));
        assertEquals(20.0, PropertyDispatcher.getDouble(reg(twt3), THREE_WT_LEG3_RATED_U));
    }

    // ── CurrentLimits ─────────────────────────────────────────────────────────

    @Test void currentLimits_permanentLimit() {
        Substation s1 = network.newSubstation().setId("CS1").setCountry(Country.FR).add();
        Substation s2 = network.newSubstation().setId("CS2").setCountry(Country.DE).add();
        VoltageLevel vl1 = s1.newVoltageLevel().setId("CVL1").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        VoltageLevel vl2 = s2.newVoltageLevel().setId("CVL2").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b1 = vl1.getBusBreakerView().newBus().setId("CB1").add();
        Bus b2 = vl2.getBusBreakerView().newBus().setId("CB2").add();
        Line line = network.newLine().setId("CL")
                .setVoltageLevel1(vl1.getId()).setBus1(b1.getId()).setConnectableBus1(b1.getId())
                .setVoltageLevel2(vl2.getId()).setBus2(b2.getId()).setConnectableBus2(b2.getId())
                .setR(1).setX(10).setG1(0).setB1(0).setG2(0).setB2(0).add();
        line.newCurrentLimits1().setPermanentLimit(1000).add();
        LoadingLimits cl = line.getCurrentLimits1().orElseThrow();
        assertEquals(1000.0, PropertyDispatcher.getDouble(reg(cl), CL_PERMANENT_LIMIT));
    }

    // ── RatioTapChangerStep ───────────────────────────────────────────────────

    @Test void rtcStep_rho() {
        Substation s = addSubstation();
        VoltageLevel vl1 = s.newVoltageLevel().setId("VLH2").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        VoltageLevel vl2 = s.newVoltageLevel().setId("VLL2").setNominalV(20)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b1 = vl1.getBusBreakerView().newBus().setId("BH2").add();
        Bus b2 = vl2.getBusBreakerView().newBus().setId("BL2").add();
        TwoWindingsTransformer twt = s.newTwoWindingsTransformer().setId("TWT2")
                .setVoltageLevel1(vl1.getId()).setBus1(b1.getId()).setConnectableBus1(b1.getId())
                .setVoltageLevel2(vl2.getId()).setBus2(b2.getId()).setConnectableBus2(b2.getId())
                .setR(0.5).setX(10).setG(0).setB(0).setRatedU1(400).setRatedU2(20).add();
        twt.newRatioTapChanger().setTapPosition(0).setLowTapPosition(0)
                .beginStep().setRho(1.0).setR(0).setX(0).setG(0).setB(0).endStep()
                .add();
        RatioTapChangerStep step = twt.getRatioTapChanger().getStep(0);
        assertEquals(1.0, PropertyDispatcher.getDouble(reg(step), RTC_STEP_RHO));
    }

    // ── Unknown property throws ───────────────────────────────────────────────

    @Test void unknownPropertyThrows() {
        Substation s = addSubstation();
        VoltageLevel vl = addVoltageLevel(s, 400);
        Bus b = addBus(vl);
        Load load = vl.newLoad().setId("LU").setBus(b.getId()).setConnectableBus(b.getId())
                .setP0(0).setQ0(0).add();
        long h = reg(load);
        assertThrows(IllegalArgumentException.class,
                () -> PropertyDispatcher.getDouble(h, 99999));
    }

    // ── Builder helpers ───────────────────────────────────────────────────────

    /** Builds a standalone LCC pair (not reused by buildHvdcLine) with the given params. */
    private LccConverterStation buildIsolatedLcc(float lossFactor, float powerFactor) {
        String suffix = lossFactor + "_" + powerFactor;
        Substation s1 = network.newSubstation().setId("SL1_" + suffix).setCountry(Country.FR).add();
        Substation s2 = network.newSubstation().setId("SL2_" + suffix).setCountry(Country.DE).add();
        VoltageLevel vl1 = s1.newVoltageLevel().setId("VLL1_" + suffix).setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        VoltageLevel vl2 = s2.newVoltageLevel().setId("VLL2_" + suffix).setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b1 = vl1.getBusBreakerView().newBus().setId("BLL1_" + suffix).add();
        Bus b2 = vl2.getBusBreakerView().newBus().setId("BLL2_" + suffix).add();
        String id1 = "ILCC1_" + suffix;
        String id2 = "ILCC2_" + suffix;
        LccConverterStation cs1 = vl1.newLccConverterStation().setId(id1)
                .setBus(b1.getId()).setConnectableBus(b1.getId())
                .setLossFactor(lossFactor).setPowerFactor(powerFactor).add();
        vl2.newLccConverterStation().setId(id2)
                .setBus(b2.getId()).setConnectableBus(b2.getId())
                .setLossFactor(0.01f).setPowerFactor(0.90f).add();
        network.newHvdcLine().setId("IHVDC_" + suffix)
                .setConverterStationId1(id1).setConverterStationId2(id2)
                .setR(0.5).setNominalV(400).setActivePowerSetpoint(100).setMaxP(300)
                .setConvertersMode(HvdcLine.ConvertersMode.SIDE_1_RECTIFIER_SIDE_2_INVERTER)
                .add();
        return cs1;
    }

    private VscConverterStation buildVscConverterStation(float lossFactor) {
        Substation s = network.newSubstation().setId("SV" + lossFactor).setCountry(Country.FR).add();
        VoltageLevel vl = s.newVoltageLevel().setId("VLV" + lossFactor).setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b = vl.getBusBreakerView().newBus().setId("BV" + lossFactor).add();
        return vl.newVscConverterStation().setId("VSC" + lossFactor)
                .setBus(b.getId()).setConnectableBus(b.getId())
                .setLossFactor(lossFactor)
                .setVoltageRegulatorOn(true)
                .setVoltageSetpoint(400)
                .setReactivePowerSetpoint(50)
                .add();
    }

    private HvdcLine buildHvdcLine() {
        Substation s1 = network.newSubstation().setId("SH1").setCountry(Country.FR).add();
        Substation s2 = network.newSubstation().setId("SH2").setCountry(Country.DE).add();
        VoltageLevel vl1 = s1.newVoltageLevel().setId("VLH1").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        VoltageLevel vl2 = s2.newVoltageLevel().setId("VLH2").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b1 = vl1.getBusBreakerView().newBus().setId("BHH1").add();
        Bus b2 = vl2.getBusBreakerView().newBus().setId("BHH2").add();
        vl1.newLccConverterStation().setId("LCC1")
                .setBus(b1.getId()).setConnectableBus(b1.getId())
                .setLossFactor(0.02f).setPowerFactor(0.95f).add();
        vl2.newLccConverterStation().setId("LCC2")
                .setBus(b2.getId()).setConnectableBus(b2.getId())
                .setLossFactor(0.02f).setPowerFactor(0.95f).add();
        return network.newHvdcLine().setId("HVDC1")
                .setConverterStationId1("LCC1").setConverterStationId2("LCC2")
                .setR(0.5).setNominalV(400).setActivePowerSetpoint(300).setMaxP(500)
                .setConvertersMode(HvdcLine.ConvertersMode.SIDE_1_RECTIFIER_SIDE_2_INVERTER)
                .add();
    }

    private ThreeWindingsTransformer buildThreeWT() {
        Substation s = network.newSubstation().setId("S3").setCountry(Country.FR).add();
        VoltageLevel vl1 = s.newVoltageLevel().setId("3VL1").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        VoltageLevel vl2 = s.newVoltageLevel().setId("3VL2").setNominalV(220)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        VoltageLevel vl3 = s.newVoltageLevel().setId("3VL3").setNominalV(20)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b1 = vl1.getBusBreakerView().newBus().setId("3B1").add();
        Bus b2 = vl2.getBusBreakerView().newBus().setId("3B2").add();
        Bus b3 = vl3.getBusBreakerView().newBus().setId("3B3").add();
        return s.newThreeWindingsTransformer().setId("3WT")
                .newLeg1().setR(1).setX(10).setG(0).setB(0).setRatedU(400)
                    .setVoltageLevel(vl1.getId()).setBus(b1.getId()).setConnectableBus(b1.getId()).add()
                .newLeg2().setR(2).setX(20).setG(0).setB(0).setRatedU(220)
                    .setVoltageLevel(vl2.getId()).setBus(b2.getId()).setConnectableBus(b2.getId()).add()
                .newLeg3().setR(3).setX(30).setG(0).setB(0).setRatedU(20)
                    .setVoltageLevel(vl3.getId()).setBus(b3.getId()).setConnectableBus(b3.getId()).add()
                .add();
    }
}
