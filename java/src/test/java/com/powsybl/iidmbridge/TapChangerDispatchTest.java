package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.regulation.RegulationMode;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;
import org.junit.jupiter.api.function.Executable;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Tap changers, their steps and the legs of three windings transformers: objects of their own,
 * reached from their holder.
 */
class TapChangerDispatchTest {

    private Network network;
    private TwoWindingsTransformer twt;
    private ThreeWindingsTransformer twt3;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("net", "test");
        Substation s = network.newSubstation().setId("S").add();
        for (String id : new String[] {"VL1", "VL2", "VL3"}) {
            VoltageLevel vl = s.newVoltageLevel().setId(id).setNominalV(id.equals("VL1") ? 400 : 225)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
            vl.getBusBreakerView().newBus().setId(id + "_B").add();
        }
        twt = s.newTwoWindingsTransformer().setId("T2")
            .setVoltageLevel1("VL1").setBus1("VL1_B").setVoltageLevel2("VL2").setBus2("VL2_B")
            .setR(1).setX(10).setG(0).setB(0).setRatedU1(400).setRatedU2(225).add();
        twt.newRatioTapChanger().setLowTapPosition(-1).setTapPosition(0).setLoadTapChangingCapabilities(true)
            .beginStep().setRho(0.9).setR(0).setX(0).setG(0).setB(0).endStep()
            .beginStep().setRho(1.0).setR(0).setX(0).setG(0).setB(0).endStep()
            .beginStep().setRho(1.1).setR(0).setX(0).setG(0).setB(0).endStep()
            .newVoltageRegulation().withMode(RegulationMode.VOLTAGE).withTargetValue(230)
                .withTerminal(twt.getTerminal2()).withTargetDeadband(2).withRegulating(true).add()
            .add();
        twt.newPhaseTapChanger().setLowTapPosition(0).setTapPosition(1)
            .setRegulationMode(PhaseTapChanger.RegulationMode.ACTIVE_POWER_CONTROL).setRegulationValue(50)
            .setTargetDeadband(5).setRegulating(false).setRegulationTerminal(twt.getTerminal1())
            .beginStep().setAlpha(-10).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
            .beginStep().setAlpha(0).setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
            .add();
        twt3 = s.newThreeWindingsTransformer().setId("T3")
            .newLeg1().setR(1).setX(10).setG(0).setB(0).setRatedU(400).setVoltageLevel("VL1").setBus("VL1_B").add()
            .newLeg2().setR(2).setX(20).setG(0).setB(0).setRatedU(225).setVoltageLevel("VL2").setBus("VL2_B").add()
            .newLeg3().setR(3).setX(30).setG(0).setB(0).setRatedU(225).setRatedS(100).setVoltageLevel("VL3").setBus("VL3_B").add()
            .add();
        twt3.getLeg2().newRatioTapChanger().setLowTapPosition(1).setTapPosition(1)
            .beginStep().setRho(1).setR(0).setX(0).setG(0).setB(0).endStep()
            .add();
        twt3.getLeg1().newOperationalLimitsGroup("G1").newCurrentLimits().setPermanentLimit(500).add();
        twt3.getLeg1().newOperationalLimitsGroup("G2");
        twt3.getLeg1().setSelectedOperationalLimitsGroup("G1");
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

    private static int kindOfFailure(Executable call) {
        return BridgeErrors.kindOf(assertThrows(Throwable.class, call));
    }

    @Test void tapChangersOfTheirHolders() {
        assertSame(twt.getRatioTapChanger(), obj(PropertyDispatcher.getRelated(reg(twt), RATIO_TAP_CHANGER)));
        assertSame(twt.getPhaseTapChanger(), obj(PropertyDispatcher.getRelated(reg(twt), PHASE_TAP_CHANGER)));
        long leg2 = PropertyDispatcher.getRelatedByIndex(reg(twt3), THREE_WINDINGS_TRANSFORMER_LEG, ThreeSides.TWO.ordinal());
        assertSame(twt3.getLeg2().getRatioTapChanger(), obj(PropertyDispatcher.getRelated(leg2, RATIO_TAP_CHANGER)));
        assertEquals(0L, PropertyDispatcher.getRelated(leg2, PHASE_TAP_CHANGER)); // none
    }

    @Test void positions() {
        long rtc = reg(twt.getRatioTapChanger());
        assertEquals(-1, PropertyDispatcher.getInt(rtc, TC_LOW_TAP_POSITION));
        assertEquals(1, PropertyDispatcher.getInt(rtc, TC_HIGH_TAP_POSITION));
        assertEquals(3, PropertyDispatcher.getInt(rtc, TC_STEP_COUNT));
        assertEquals(0, PropertyDispatcher.getInt(rtc, TC_TAP_POSITION));
        assertEquals(0, PropertyDispatcher.getOptInt(rtc, TC_TAP_POSITION));
        assertEquals(0, PropertyDispatcher.getOptInt(rtc, TC_NEUTRAL_POSITION)); // rho 1
        PropertyDispatcher.setInt(rtc, TC_TAP_POSITION, 1);
        assertEquals(1, twt.getRatioTapChanger().getTapPosition());
        assertEquals(1, PropertyDispatcher.getBool(rtc, TC_LOAD_TAP_CHANGING_CAPABILITIES));

        // Unset (a network that accepts it): no value, getInt fails, no current step
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.invoke(rtc, ACT_UNSET_TAP_POSITION)));
        network.setMinimumAcceptableValidationLevel(ValidationLevel.EQUIPMENT);
        assertEquals(0L, PropertyDispatcher.invoke(rtc, ACT_UNSET_TAP_POSITION));
        assertEquals(PropertyDispatcher.NO_INT, PropertyDispatcher.getOptInt(rtc, TC_TAP_POSITION));
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.getInt(rtc, TC_TAP_POSITION)));
        assertEquals(0L, PropertyDispatcher.getRelated(rtc, REL_CURRENT_STEP));
    }

    @Test void solvedPosition() {
        long rtc = reg(twt.getRatioTapChanger());
        assertEquals(PropertyDispatcher.NO_INT, PropertyDispatcher.getOptInt(rtc, TC_SOLVED_TAP_POSITION));
        assertEquals(0L, PropertyDispatcher.getRelated(rtc, REL_SOLVED_CURRENT_STEP));
        PropertyDispatcher.setInt(rtc, TC_SOLVED_TAP_POSITION, -1);
        assertEquals(-1, PropertyDispatcher.getOptInt(rtc, TC_SOLVED_TAP_POSITION));
        assertSame(twt.getRatioTapChanger().getStep(-1), obj(PropertyDispatcher.getRelated(rtc, REL_SOLVED_CURRENT_STEP)));
        PropertyDispatcher.invoke(rtc, ACT_APPLY_SOLVED_VALUES);
        assertEquals(-1, twt.getRatioTapChanger().getTapPosition());
        PropertyDispatcher.invoke(rtc, ACT_UNSET_SOLVED_TAP_POSITION);
        assertEquals(PropertyDispatcher.NO_INT, PropertyDispatcher.getOptInt(rtc, TC_SOLVED_TAP_POSITION));
        PropertyDispatcher.setInt(rtc, TC_TAP_POSITION, 1);
        PropertyDispatcher.invoke(rtc, ACT_SET_TAP_POSITION_TO_SOLVED_TAP_POSITION); // none: no change
        assertEquals(1, twt.getRatioTapChanger().getTapPosition());
    }

    @Test void stepsFromLowToHigh() {
        RatioTapChanger tc = twt.getRatioTapChanger();
        long rtc = reg(tc);
        long[] steps = PropertyDispatcher.getChildren(rtc, RATIO_TAP_CHANGER_STEP);
        assertEquals(3, steps.length);
        assertEquals(0.9, PropertyDispatcher.getDouble(steps[0], STEP_RHO), 1e-12); // position -1
        assertEquals(1.1, PropertyDispatcher.getDouble(steps[2], STEP_RHO), 1e-12); // position 1
        assertSame(tc.getStep(1), obj(PropertyDispatcher.getRelatedByIndex(rtc, RATIO_TAP_CHANGER_STEP, 1)));
        assertSame(tc.getStep(0), obj(PropertyDispatcher.getRelated(rtc, REL_CURRENT_STEP)));
        assertSame(tc.getStep(0), obj(PropertyDispatcher.getRelated(rtc, REL_NEUTRAL_STEP)));
        assertEquals(BridgeErrors.VALIDATION,
            kindOfFailure(() -> PropertyDispatcher.getRelatedByIndex(rtc, RATIO_TAP_CHANGER_STEP, 2)));
        // A ratio tap changer has no phase tap changer steps
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND,
            kindOfFailure(() -> PropertyDispatcher.getChildren(rtc, PHASE_TAP_CHANGER_STEP)));

        PropertyDispatcher.setDouble(steps[2], STEP_X, 3);
        assertEquals(3, tc.getStep(1).getX());
        long ptcStep = PropertyDispatcher.getRelatedByIndex(reg(twt.getPhaseTapChanger()), PHASE_TAP_CHANGER_STEP, 0);
        assertEquals(-10, PropertyDispatcher.getDouble(ptcStep, PTC_STEP_ALPHA));
        assertEquals(1, PropertyDispatcher.getDouble(ptcStep, STEP_RHO));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getDouble(steps[0], PTC_STEP_ALPHA)));
    }

    @Test void ratioTapChangerRegulatesThroughItsVoltageRegulation() {
        long rtc = reg(twt.getRatioTapChanger());
        assertEquals(1, PropertyDispatcher.getBool(rtc, REGULATING));
        long regulation = PropertyDispatcher.getRelated(rtc, VOLTAGE_REGULATION);
        assertEquals(230, PropertyDispatcher.getDouble(regulation, VR_TARGET_VALUE));
        assertEquals(2, PropertyDispatcher.getDouble(regulation, VR_TARGET_DEADBAND));
        assertArrayEquals(new int[] {RegulationMode.VOLTAGE.ordinal()}, PropertyDispatcher.getIntList(rtc, REGULATING_MODES));
        assertSame(twt.getTerminal2(), obj(PropertyDispatcher.getRelated(regulation, REL_TERMINAL)));
        // Its own setRegulating is deprecated: not dispatched
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.setBool(rtc, REGULATING, 0)));
        PropertyDispatcher.setBool(regulation, VR_REGULATING, 0);
        assertEquals(0, PropertyDispatcher.getBool(rtc, REGULATING));
    }

    @Test void phaseTapChanger() {
        long ptc = reg(twt.getPhaseTapChanger());
        assertEquals(PhaseTapChanger.RegulationMode.ACTIVE_POWER_CONTROL.ordinal(), PropertyDispatcher.getInt(ptc, PTC_REGULATION_MODE));
        assertEquals(50, PropertyDispatcher.getDouble(ptc, PTC_REGULATION_VALUE));
        assertEquals(5, PropertyDispatcher.getDouble(ptc, PTC_TARGET_DEADBAND));
        assertEquals(0, PropertyDispatcher.getBool(ptc, REGULATING));
        assertSame(twt.getTerminal1(), obj(PropertyDispatcher.getRelated(ptc, REL_REGULATION_TERMINAL)));

        PropertyDispatcher.setInt(ptc, PTC_REGULATION_MODE, PhaseTapChanger.RegulationMode.CURRENT_LIMITER.ordinal());
        PropertyDispatcher.setDouble(ptc, PTC_REGULATION_VALUE, 300);
        // One regulating tap changer at most
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.setBool(ptc, REGULATING, 1)));
        twt.getRatioTapChanger().getVoltageRegulation().setRegulating(false);
        PropertyDispatcher.setBool(ptc, REGULATING, 1);
        PropertyDispatcher.setRelated(ptc, REL_REGULATION_TERMINAL, reg(twt.getTerminal2()));
        PhaseTapChanger tc = twt.getPhaseTapChanger();
        assertEquals(PhaseTapChanger.RegulationMode.CURRENT_LIMITER, tc.getRegulationMode());
        assertEquals(300, tc.getRegulationValue());
        assertTrue(tc.isRegulating());
        assertSame(twt.getTerminal2(), tc.getRegulationTerminal());
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.setInt(ptc, PTC_REGULATION_MODE, 2)));
    }

    @Test void propertiesAndRemoval() {
        long ptc = reg(twt.getPhaseTapChanger());
        PropertyDispatcher.setKeyedString(ptc, PROPERTY, "k", "v");
        assertEquals("v", twt.getPhaseTapChanger().getProperty("k"));
        long step = PropertyDispatcher.getRelatedByIndex(ptc, PHASE_TAP_CHANGER_STEP, 1);
        PropertyDispatcher.setKeyedString(step, PROPERTY, "s", "w");
        assertArrayEquals(new String[] {"s"}, PropertyDispatcher.getStringList(step, PROPERTY_NAMES));

        assertEquals(0L, PropertyDispatcher.invoke(ptc, ACT_REMOVE));
        assertFalse(twt.hasPhaseTapChanger());
        assertEquals(0L, PropertyDispatcher.getRelated(reg(twt), PHASE_TAP_CHANGER));
    }

    @Test void legs() {
        long t3 = reg(twt3);
        long[] legs = PropertyDispatcher.getChildren(t3, THREE_WINDINGS_TRANSFORMER_LEG);
        assertEquals(3, legs.length);
        for (ThreeSides side : ThreeSides.values()) {
            long leg = legs[side.ordinal()];
            assertSame(twt3.getLeg(side), obj(leg));
            assertTrue(PropertyDispatcher.isSameObject(leg, PropertyDispatcher.getRelatedByIndex(t3, THREE_WINDINGS_TRANSFORMER_LEG, side.ordinal())) == 1);
            assertEquals(side.ordinal(), PropertyDispatcher.getInt(leg, LEG_SIDE));
            assertSame(twt3, obj(PropertyDispatcher.getRelated(leg, THREE_WINDINGS_TRANSFORMER)));
            assertSame(twt3.getTerminal(side), obj(PropertyDispatcher.getRelated(leg, REL_TERMINAL)));
            assertSame(twt3.getTerminal(side), obj(PropertyDispatcher.getRelatedByIndex(t3, REL_TERMINAL, side.ordinal())));
        }
        assertEquals(BridgeErrors.VALIDATION,
            kindOfFailure(() -> PropertyDispatcher.getRelatedByIndex(t3, THREE_WINDINGS_TRANSFORMER_LEG, 3)));

        assertEquals(100, PropertyDispatcher.getDouble(legs[2], LEG_RATED_S));
        PropertyDispatcher.setDouble(legs[2], LEG_RATED_S, Double.NaN);
        assertTrue(Double.isNaN(twt3.getLeg3().getRatedS()));
        PropertyDispatcher.setDouble(legs[0], LEG_RATED_U, 410);
        assertEquals(410, twt3.getLeg1().getRatedU());
    }

    @Test void legLimits() {
        long leg1 = reg(twt3.getLeg1());
        long[] groups = PropertyDispatcher.getChildren(leg1, OPERATIONAL_LIMITS_GROUP);
        assertEquals(2, groups.length);
        long selected = PropertyDispatcher.getRelated(leg1, REL_SELECTED_OLG);
        assertEquals("G1", PropertyDispatcher.getString(selected, OLG_ID));
        long limits = PropertyDispatcher.getRelated(leg1, CURRENT_LIMITS);
        assertEquals(500, PropertyDispatcher.getDouble(limits, CL_PERMANENT_LIMIT));
        assertTrue(PropertyDispatcher.isSameObject(limits, PropertyDispatcher.getRelated(selected, CURRENT_LIMITS)) == 1);
        long leg2 = reg(twt3.getLeg2());
        assertEquals(0L, PropertyDispatcher.getRelated(leg2, REL_SELECTED_OLG));
        assertEquals(0L, PropertyDispatcher.getRelated(leg2, CURRENT_LIMITS));
    }
}
