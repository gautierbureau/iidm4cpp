package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.ShuntCompensatorNonLinearModel;
import com.powsybl.iidm.network.extensions.ActivePowerControl;
import com.powsybl.iidm.network.extensions.CoordinatedReactiveControl;
import com.powsybl.iidm.network.extensions.HvdcAngleDroopActivePowerControl;
import com.powsybl.iidm.network.extensions.HvdcOperatorActivePowerRange;
import com.powsybl.iidm.network.extensions.VoltagePerReactivePowerControl;
import com.powsybl.iidm.network.extensions.SlackTerminal;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import java.util.Collection;

import java.util.stream.Stream;
import java.util.stream.StreamSupport;

import static com.powsybl.iidmbridge.PropertyCodes.*;

/**
 * Dispatches property get/set operations to the correct PowSyBl IIDM Java methods
 * based on integer property codes defined in {@link PropertyCodes}.
 *
 * Used by the GraalVM backend entry points and can also be reused from the JNI side.
 */
public final class PropertyDispatcher {

    private PropertyDispatcher() {}

    // ── Double getters ────────────────────────────────────────────────────

    public static double getDouble(long handle, int property) {
        Object obj = NetworkRegistry.lookup(handle);
        return switch (property) {
            case GEN_TARGET_P  -> ((Generator) obj).getTargetP();
            case GEN_TARGET_Q  -> ((Generator) obj).getTargetQ();
            case GEN_TARGET_V  -> ((Generator) obj).getTargetV();
            case GEN_MIN_P     -> ((Generator) obj).getMinP();
            case GEN_MAX_P     -> ((Generator) obj).getMaxP();
            case LOAD_P0       -> ((Load) obj).getP0();
            case LOAD_Q0       -> ((Load) obj).getQ0();
            case TERMINAL_P    -> ((Terminal) obj).getP();
            case TERMINAL_Q    -> ((Terminal) obj).getQ();
            case BUS_V         -> ((Bus) obj).getV();
            case BUS_ANGLE     -> ((Bus) obj).getAngle();
            case VL_NOMINAL_V  -> ((VoltageLevel) obj).getNominalV();
            case VL_LOW_VOLTAGE_LIMIT  -> ((VoltageLevel) obj).getLowVoltageLimit();
            case VL_HIGH_VOLTAGE_LIMIT -> ((VoltageLevel) obj).getHighVoltageLimit();
            case LINE_R  -> ((Line) obj).getR();
            case LINE_X  -> ((Line) obj).getX();
            case LINE_G1 -> ((Line) obj).getG1();
            case LINE_B1 -> ((Line) obj).getB1();
            case LINE_G2 -> ((Line) obj).getG2();
            case LINE_B2 -> ((Line) obj).getB2();
            case TWO_WT_R        -> ((TwoWindingsTransformer) obj).getR();
            case TWO_WT_X        -> ((TwoWindingsTransformer) obj).getX();
            case TWO_WT_G        -> ((TwoWindingsTransformer) obj).getG();
            case TWO_WT_B        -> ((TwoWindingsTransformer) obj).getB();
            case TWO_WT_RATED_U1 -> ((TwoWindingsTransformer) obj).getRatedU1();
            case TWO_WT_RATED_U2 -> ((TwoWindingsTransformer) obj).getRatedU2();
            case TWO_WT_RATED_S  -> ((TwoWindingsTransformer) obj).getRatedS();
            case HVDC_R                     -> ((HvdcLine) obj).getR();
            case HVDC_NOMINAL_V             -> ((HvdcLine) obj).getNominalV();
            case HVDC_ACTIVE_POWER_SETPOINT -> ((HvdcLine) obj).getActivePowerSetpoint();
            case HVDC_MAX_P                 -> ((HvdcLine) obj).getMaxP();
            case DL_P0 -> ((DanglingLine) obj).getP0();
            case DL_Q0 -> ((DanglingLine) obj).getQ0();
            case DL_R  -> ((DanglingLine) obj).getR();
            case DL_X  -> ((DanglingLine) obj).getX();
            case DL_G  -> ((DanglingLine) obj).getG();
            case DL_B  -> ((DanglingLine) obj).getB();
            case SHUNT_B_PER_SECTION -> ((ShuntCompensator) obj).getB();
            case SHUNT_G_PER_SECTION -> ((ShuntCompensator) obj).getG();
            case SVC_B_MIN                   -> ((StaticVarCompensator) obj).getBmin();
            case SVC_B_MAX                   -> ((StaticVarCompensator) obj).getBmax();
            case SVC_VOLTAGE_SETPOINT        -> ((StaticVarCompensator) obj).getVoltageSetpoint();
            case SVC_REACTIVE_POWER_SETPOINT -> ((StaticVarCompensator) obj).getReactivePowerSetpoint();
            case VSC_VOLTAGE_SETPOINT        -> ((VscConverterStation) obj).getVoltageSetpoint();
            case VSC_REACTIVE_POWER_SETPOINT -> ((VscConverterStation) obj).getReactivePowerSetpoint();
            case EXT_APC_DROOP -> ((Generator) obj).getExtension(ActivePowerControl.class).getDroop();
            case EXT_CRC_Q_PERCENT -> ((Generator) obj).getExtension(CoordinatedReactiveControl.class).getQPercent();
            case EXT_HADAPC_DROOP -> ((HvdcLine) obj).getExtension(HvdcAngleDroopActivePowerControl.class).getDroop();
            case EXT_HADAPC_P0 -> ((HvdcLine) obj).getExtension(HvdcAngleDroopActivePowerControl.class).getP0();
            case EXT_HOAR_OPR_CS1_TO_CS2 -> ((HvdcLine) obj).getExtension(HvdcOperatorActivePowerRange.class).getOprFromCS1toCS2();
            case EXT_HOAR_OPR_CS2_TO_CS1 -> ((HvdcLine) obj).getExtension(HvdcOperatorActivePowerRange.class).getOprFromCS2toCS1();
            case EXT_VPRC_SLOPE -> ((StaticVarCompensator) obj).getExtension(VoltagePerReactivePowerControl.class).getSlope();
            case TWO_WT_RTC_TARGET_V  -> ((TwoWindingsTransformer) obj).getRatioTapChanger().getTargetV();
            case TWO_WT_PTC_REG_VALUE -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().getRegulationValue();
            case BBS_V     -> ((BusbarSection) obj).getV();
            case BBS_ANGLE -> ((BusbarSection) obj).getAngle();
            // Battery
            case BAT_TARGET_P -> ((Battery) obj).getTargetP();
            case BAT_TARGET_Q -> ((Battery) obj).getTargetQ();
            case BAT_MIN_P    -> ((Battery) obj).getMinP();
            case BAT_MAX_P    -> ((Battery) obj).getMaxP();
            // Generator reactive limits (min-max kind)
            case GEN_MIN_Q -> ((Generator) obj).getReactiveLimits(MinMaxReactiveLimits.class).getMinQ();
            case GEN_MAX_Q -> ((Generator) obj).getReactiveLimits(MinMaxReactiveLimits.class).getMaxQ();
            // VSC reactive limits
            case VSC_MIN_Q -> ((VscConverterStation) obj).getReactiveLimits(MinMaxReactiveLimits.class).getMinQ();
            case VSC_MAX_Q -> ((VscConverterStation) obj).getReactiveLimits(MinMaxReactiveLimits.class).getMaxQ();
            // ReactiveCapabilityCurve.Point
            case POINT_P     -> ((ReactiveCapabilityCurve.Point) obj).getP();
            case POINT_MIN_Q -> ((ReactiveCapabilityCurve.Point) obj).getMinQ();
            case POINT_MAX_Q -> ((ReactiveCapabilityCurve.Point) obj).getMaxQ();
            // RatioTapChangerStep
            case RTC_STEP_RHO -> ((RatioTapChangerStep) obj).getRho();
            case RTC_STEP_R   -> ((RatioTapChangerStep) obj).getR();
            case RTC_STEP_X   -> ((RatioTapChangerStep) obj).getX();
            case RTC_STEP_G   -> ((RatioTapChangerStep) obj).getG();
            case RTC_STEP_B   -> ((RatioTapChangerStep) obj).getB();
            // PhaseTapChangerStep
            case PTC_STEP_ALPHA -> ((PhaseTapChangerStep) obj).getAlpha();
            case PTC_STEP_RHO   -> ((PhaseTapChangerStep) obj).getRho();
            case PTC_STEP_R     -> ((PhaseTapChangerStep) obj).getR();
            case PTC_STEP_X     -> ((PhaseTapChangerStep) obj).getX();
            case PTC_STEP_G     -> ((PhaseTapChangerStep) obj).getG();
            case PTC_STEP_B     -> ((PhaseTapChangerStep) obj).getB();
            // ShuntCompensatorNonLinearModel.Section (non-linear model)
            case SHUNT_SECTION_B -> ((ShuntCompensatorNonLinearModel.Section) obj).getB();
            case SHUNT_SECTION_G -> ((ShuntCompensatorNonLinearModel.Section) obj).getG();
            // ThreeWindingsTransformer leg doubles
            case THREE_WT_LEG1_R       -> ((ThreeWindingsTransformer) obj).getLeg1().getR();
            case THREE_WT_LEG1_X       -> ((ThreeWindingsTransformer) obj).getLeg1().getX();
            case THREE_WT_LEG1_G       -> ((ThreeWindingsTransformer) obj).getLeg1().getG();
            case THREE_WT_LEG1_B       -> ((ThreeWindingsTransformer) obj).getLeg1().getB();
            case THREE_WT_LEG1_RATED_U -> ((ThreeWindingsTransformer) obj).getLeg1().getRatedU();
            case THREE_WT_LEG1_RATED_S -> ((ThreeWindingsTransformer) obj).getLeg1().getRatedS();
            case THREE_WT_LEG1_RTC_TARGET_V  -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().getTargetV();
            case THREE_WT_LEG1_PTC_REG_VALUE -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().getRegulationValue();
            case THREE_WT_LEG2_R       -> ((ThreeWindingsTransformer) obj).getLeg2().getR();
            case THREE_WT_LEG2_X       -> ((ThreeWindingsTransformer) obj).getLeg2().getX();
            case THREE_WT_LEG2_G       -> ((ThreeWindingsTransformer) obj).getLeg2().getG();
            case THREE_WT_LEG2_B       -> ((ThreeWindingsTransformer) obj).getLeg2().getB();
            case THREE_WT_LEG2_RATED_U -> ((ThreeWindingsTransformer) obj).getLeg2().getRatedU();
            case THREE_WT_LEG2_RATED_S -> ((ThreeWindingsTransformer) obj).getLeg2().getRatedS();
            case THREE_WT_LEG2_RTC_TARGET_V  -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().getTargetV();
            case THREE_WT_LEG2_PTC_REG_VALUE -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().getRegulationValue();
            case THREE_WT_LEG3_R       -> ((ThreeWindingsTransformer) obj).getLeg3().getR();
            case THREE_WT_LEG3_X       -> ((ThreeWindingsTransformer) obj).getLeg3().getX();
            case THREE_WT_LEG3_G       -> ((ThreeWindingsTransformer) obj).getLeg3().getG();
            case THREE_WT_LEG3_B       -> ((ThreeWindingsTransformer) obj).getLeg3().getB();
            case THREE_WT_LEG3_RATED_U -> ((ThreeWindingsTransformer) obj).getLeg3().getRatedU();
            case THREE_WT_LEG3_RATED_S -> ((ThreeWindingsTransformer) obj).getLeg3().getRatedS();
            case THREE_WT_LEG3_RTC_TARGET_V  -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().getTargetV();
            case THREE_WT_LEG3_PTC_REG_VALUE -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().getRegulationValue();
            default -> throw new IllegalArgumentException("Unknown double property: " + property);
        };
    }

    // ── Double setters ────────────────────────────────────────────────────

    public static void setDouble(long handle, int property, double value) {
        Object obj = NetworkRegistry.lookup(handle);
        switch (property) {
            case GEN_TARGET_P  -> ((Generator) obj).setTargetP(value);
            case GEN_TARGET_Q  -> ((Generator) obj).setTargetQ(value);
            case GEN_TARGET_V  -> ((Generator) obj).setTargetV(value);
            case LOAD_P0       -> ((Load) obj).setP0(value);
            case LOAD_Q0       -> ((Load) obj).setQ0(value);
            case TERMINAL_P    -> ((Terminal) obj).setP(value);
            case TERMINAL_Q    -> ((Terminal) obj).setQ(value);
            case BUS_V         -> ((Bus) obj).setV(value);
            case BUS_ANGLE     -> ((Bus) obj).setAngle(value);
            case HVDC_ACTIVE_POWER_SETPOINT -> ((HvdcLine) obj).setActivePowerSetpoint(value);
            case DL_P0 -> ((DanglingLine) obj).setP0(value);
            case DL_Q0 -> ((DanglingLine) obj).setQ0(value);
            case SVC_VOLTAGE_SETPOINT        -> ((StaticVarCompensator) obj).setVoltageSetpoint(value);
            case SVC_REACTIVE_POWER_SETPOINT -> ((StaticVarCompensator) obj).setReactivePowerSetpoint(value);
            case VSC_VOLTAGE_SETPOINT        -> ((VscConverterStation) obj).setVoltageSetpoint(value);
            case VSC_REACTIVE_POWER_SETPOINT -> ((VscConverterStation) obj).setReactivePowerSetpoint(value);
            case EXT_APC_DROOP -> ((Generator) obj).getExtension(ActivePowerControl.class).setDroop(value);
            case EXT_CRC_Q_PERCENT -> ((Generator) obj).getExtension(CoordinatedReactiveControl.class).setQPercent(value);
            case EXT_HADAPC_DROOP -> ((HvdcLine) obj).getExtension(HvdcAngleDroopActivePowerControl.class).setDroop((float) value);
            case EXT_HADAPC_P0 -> ((HvdcLine) obj).getExtension(HvdcAngleDroopActivePowerControl.class).setP0((float) value);
            case EXT_HOAR_OPR_CS1_TO_CS2 -> ((HvdcLine) obj).getExtension(HvdcOperatorActivePowerRange.class).setOprFromCS1toCS2((float) value);
            case EXT_HOAR_OPR_CS2_TO_CS1 -> ((HvdcLine) obj).getExtension(HvdcOperatorActivePowerRange.class).setOprFromCS2toCS1((float) value);
            case EXT_VPRC_SLOPE -> ((StaticVarCompensator) obj).getExtension(VoltagePerReactivePowerControl.class).setSlope(value);
            case TWO_WT_RTC_TARGET_V  -> ((TwoWindingsTransformer) obj).getRatioTapChanger().setTargetV(value);
            case TWO_WT_PTC_REG_VALUE -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().setRegulationValue(value);
            case BAT_TARGET_P -> ((Battery) obj).setTargetP(value);
            case BAT_TARGET_Q -> ((Battery) obj).setTargetQ(value);
            case THREE_WT_LEG1_RTC_TARGET_V  -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().setTargetV(value);
            case THREE_WT_LEG1_PTC_REG_VALUE -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().setRegulationValue(value);
            case THREE_WT_LEG2_RTC_TARGET_V  -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().setTargetV(value);
            case THREE_WT_LEG2_PTC_REG_VALUE -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().setRegulationValue(value);
            case THREE_WT_LEG3_RTC_TARGET_V  -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().setTargetV(value);
            case THREE_WT_LEG3_PTC_REG_VALUE -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().setRegulationValue(value);
            default -> throw new IllegalArgumentException("Unknown double property for set: " + property);
        }
    }

    // ── Int getters ───────────────────────────────────────────────────────

    public static int getInt(long handle, int property) {
        Object obj = NetworkRegistry.lookup(handle);
        return switch (property) {
            case GEN_ENERGY_SOURCE  -> ((Generator) obj).getEnergySource().ordinal();
            case LOAD_TYPE          -> ((Load) obj).getLoadType().ordinal();
            case VL_TOPOLOGY_KIND   -> ((VoltageLevel) obj).getTopologyKind().ordinal();
            case HVDC_CONVERTERS_MODE -> ((HvdcLine) obj).getConvertersMode().ordinal();
            case SVC_REGULATION_MODE  -> ((StaticVarCompensator) obj).getRegulationMode().ordinal();
            case SHUNT_SECTION_COUNT     -> ((ShuntCompensator) obj).getSectionCount();
            case SHUNT_MAX_SECTION_COUNT -> ((ShuntCompensator) obj).getMaximumSectionCount();
            case SW_KIND -> ((Switch) obj).getKind().ordinal();
            case TWO_WT_RTC_TAP_POSITION -> ((TwoWindingsTransformer) obj).getRatioTapChanger().getTapPosition();
            case TWO_WT_RTC_LOW_TAP      -> ((TwoWindingsTransformer) obj).getRatioTapChanger().getLowTapPosition();
            case TWO_WT_RTC_HIGH_TAP     -> ((TwoWindingsTransformer) obj).getRatioTapChanger().getHighTapPosition();
            case TWO_WT_PTC_TAP_POSITION -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().getTapPosition();
            case TWO_WT_PTC_LOW_TAP      -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().getLowTapPosition();
            case TWO_WT_PTC_HIGH_TAP     -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().getHighTapPosition();
            case TWO_WT_PTC_REG_MODE     -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().getRegulationMode().ordinal();
            // Generator reactive limits kind: 0=NONE, 1=MIN_MAX, 2=CURVE
            case GEN_REACTIVE_LIMITS_KIND -> {
                ReactiveLimits rl = ((Generator) obj).getReactiveLimits();
                if (rl instanceof MinMaxReactiveLimits) yield 1;
                if (rl instanceof ReactiveCapabilityCurve) yield 2;
                yield 0;
            }
            // VSC reactive limits kind
            case VSC_REACTIVE_LIMITS_KIND -> {
                ReactiveLimits rl = ((VscConverterStation) obj).getReactiveLimits();
                if (rl instanceof MinMaxReactiveLimits) yield 1;
                if (rl instanceof ReactiveCapabilityCurve) yield 2;
                yield 0;
            }
            // Shunt model kind: 0=LINEAR, 1=NON_LINEAR
            case SHUNT_MODEL_KIND -> ((ShuntCompensator) obj).getModelType().ordinal();
            // Terminal node number (node-breaker topology)
            case TERMINAL_NODE -> ((Terminal) obj).getNodeBreakerView().getNode();
            // ThreeWT leg tap changer ints
            case THREE_WT_LEG1_RTC_TAP_POS  -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().getTapPosition();
            case THREE_WT_LEG1_RTC_LOW_TAP  -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().getLowTapPosition();
            case THREE_WT_LEG1_RTC_HIGH_TAP -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().getHighTapPosition();
            case THREE_WT_LEG1_PTC_TAP_POS  -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().getTapPosition();
            case THREE_WT_LEG1_PTC_LOW_TAP  -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().getLowTapPosition();
            case THREE_WT_LEG1_PTC_HIGH_TAP -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().getHighTapPosition();
            case THREE_WT_LEG1_PTC_REG_MODE -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().getRegulationMode().ordinal();
            case THREE_WT_LEG2_RTC_TAP_POS  -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().getTapPosition();
            case THREE_WT_LEG2_RTC_LOW_TAP  -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().getLowTapPosition();
            case THREE_WT_LEG2_RTC_HIGH_TAP -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().getHighTapPosition();
            case THREE_WT_LEG2_PTC_TAP_POS  -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().getTapPosition();
            case THREE_WT_LEG2_PTC_LOW_TAP  -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().getLowTapPosition();
            case THREE_WT_LEG2_PTC_HIGH_TAP -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().getHighTapPosition();
            case THREE_WT_LEG2_PTC_REG_MODE -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().getRegulationMode().ordinal();
            case THREE_WT_LEG3_RTC_TAP_POS  -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().getTapPosition();
            case THREE_WT_LEG3_RTC_LOW_TAP  -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().getLowTapPosition();
            case THREE_WT_LEG3_RTC_HIGH_TAP -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().getHighTapPosition();
            case THREE_WT_LEG3_PTC_TAP_POS  -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().getTapPosition();
            case THREE_WT_LEG3_PTC_LOW_TAP  -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().getLowTapPosition();
            case THREE_WT_LEG3_PTC_HIGH_TAP -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().getHighTapPosition();
            case THREE_WT_LEG3_PTC_REG_MODE -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().getRegulationMode().ordinal();
            default -> throw new IllegalArgumentException("Unknown int property: " + property);
        };
    }

    // ── Int setters ───────────────────────────────────────────────────────

    public static void setInt(long handle, int property, int value) {
        Object obj = NetworkRegistry.lookup(handle);
        switch (property) {
            case HVDC_CONVERTERS_MODE ->
                ((HvdcLine) obj).setConvertersMode(HvdcLine.ConvertersMode.values()[value]);
            case SVC_REGULATION_MODE ->
                ((StaticVarCompensator) obj).setRegulationMode(
                    StaticVarCompensator.RegulationMode.values()[value]);
            case SHUNT_SECTION_COUNT ->
                ((ShuntCompensator) obj).setSectionCount(value);
            case TWO_WT_RTC_TAP_POSITION ->
                ((TwoWindingsTransformer) obj).getRatioTapChanger().setTapPosition(value);
            case TWO_WT_PTC_TAP_POSITION ->
                ((TwoWindingsTransformer) obj).getPhaseTapChanger().setTapPosition(value);
            case TWO_WT_PTC_REG_MODE ->
                ((TwoWindingsTransformer) obj).getPhaseTapChanger().setRegulationMode(
                    PhaseTapChanger.RegulationMode.values()[value]);
            case THREE_WT_LEG1_RTC_TAP_POS ->
                ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().setTapPosition(value);
            case THREE_WT_LEG1_PTC_TAP_POS ->
                ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().setTapPosition(value);
            case THREE_WT_LEG1_PTC_REG_MODE ->
                ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().setRegulationMode(
                    PhaseTapChanger.RegulationMode.values()[value]);
            case THREE_WT_LEG2_RTC_TAP_POS ->
                ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().setTapPosition(value);
            case THREE_WT_LEG2_PTC_TAP_POS ->
                ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().setTapPosition(value);
            case THREE_WT_LEG2_PTC_REG_MODE ->
                ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().setRegulationMode(
                    PhaseTapChanger.RegulationMode.values()[value]);
            case THREE_WT_LEG3_RTC_TAP_POS ->
                ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().setTapPosition(value);
            case THREE_WT_LEG3_PTC_TAP_POS ->
                ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().setTapPosition(value);
            case THREE_WT_LEG3_PTC_REG_MODE ->
                ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().setRegulationMode(
                    PhaseTapChanger.RegulationMode.values()[value]);
            default -> throw new IllegalArgumentException("Unknown int property for set: " + property);
        }
    }

    // ── Bool getters ──────────────────────────────────────────────────────

    public static int getBool(long handle, int property) {
        Object obj = NetworkRegistry.lookup(handle);
        boolean val = switch (property) {
            case GEN_VOLTAGE_REGULATOR_ON -> ((Generator) obj).isVoltageRegulatorOn();
            case TERMINAL_CONNECTED       -> ((Terminal) obj).isConnected();
            case VSC_VOLTAGE_REGULATOR_ON -> ((VscConverterStation) obj).isVoltageRegulatorOn();
            case SW_OPEN     -> ((Switch) obj).isOpen();
            case SW_RETAINED -> ((Switch) obj).isRetained();
            case TWO_WT_RTC_EXISTS     -> ((TwoWindingsTransformer) obj).getRatioTapChanger() != null;
            case TWO_WT_RTC_REGULATING -> ((TwoWindingsTransformer) obj).getRatioTapChanger().isRegulating();
            case TWO_WT_PTC_EXISTS     -> ((TwoWindingsTransformer) obj).getPhaseTapChanger() != null;
            case TWO_WT_PTC_REGULATING -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().isRegulating();
            case EXT_APC_EXISTS      -> ((Generator) obj).getExtension(ActivePowerControl.class) != null;
            case EXT_APC_PARTICIPATE -> ((Generator) obj).getExtension(ActivePowerControl.class).isParticipate();
            case EXT_CRC_EXISTS      -> ((Generator) obj).getExtension(CoordinatedReactiveControl.class) != null;
            case EXT_HADAPC_EXISTS   -> ((HvdcLine) obj).getExtension(HvdcAngleDroopActivePowerControl.class) != null;
            case EXT_HADAPC_ENABLED  -> ((HvdcLine) obj).getExtension(HvdcAngleDroopActivePowerControl.class).isEnabled();
            case EXT_HOAR_EXISTS     -> ((HvdcLine) obj).getExtension(HvdcOperatorActivePowerRange.class) != null;
            case EXT_VPRC_EXISTS     -> ((StaticVarCompensator) obj).getExtension(VoltagePerReactivePowerControl.class) != null;
            case EXT_ST_EXISTS       -> ((VoltageLevel) obj).getExtension(SlackTerminal.class) != null;
            // ThreeWT leg tap changer bools
            case THREE_WT_LEG1_RTC_EXISTS     -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger() != null;
            case THREE_WT_LEG1_RTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().isRegulating();
            case THREE_WT_LEG1_PTC_EXISTS     -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger() != null;
            case THREE_WT_LEG1_PTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().isRegulating();
            case THREE_WT_LEG2_RTC_EXISTS     -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger() != null;
            case THREE_WT_LEG2_RTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().isRegulating();
            case THREE_WT_LEG2_PTC_EXISTS     -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger() != null;
            case THREE_WT_LEG2_PTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().isRegulating();
            case THREE_WT_LEG3_RTC_EXISTS     -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger() != null;
            case THREE_WT_LEG3_RTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().isRegulating();
            case THREE_WT_LEG3_PTC_EXISTS     -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger() != null;
            case THREE_WT_LEG3_PTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().isRegulating();
            default -> throw new IllegalArgumentException("Unknown bool property: " + property);
        };
        return val ? 1 : 0;
    }

    // ── Bool setters ──────────────────────────────────────────────────────

    public static void setBool(long handle, int property, int value) {
        Object obj = NetworkRegistry.lookup(handle);
        boolean bval = value != 0;
        switch (property) {
            case GEN_VOLTAGE_REGULATOR_ON -> ((Generator) obj).setVoltageRegulatorOn(bval);
            case TERMINAL_CONNECTED -> {
                if (bval) ((Terminal) obj).connect();
                else      ((Terminal) obj).disconnect();
            }
            case VSC_VOLTAGE_REGULATOR_ON -> ((VscConverterStation) obj).setVoltageRegulatorOn(bval);
            case SW_OPEN     -> ((Switch) obj).setOpen(bval);
            case SW_RETAINED -> ((Switch) obj).setRetained(bval);
            case TWO_WT_RTC_REGULATING -> ((TwoWindingsTransformer) obj).getRatioTapChanger().setRegulating(bval);
            case TWO_WT_PTC_REGULATING -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().setRegulating(bval);
            case EXT_APC_PARTICIPATE -> ((Generator) obj).getExtension(ActivePowerControl.class).setParticipate(bval);
            case EXT_HADAPC_ENABLED  -> ((HvdcLine) obj).getExtension(HvdcAngleDroopActivePowerControl.class).setEnabled(bval);
            case THREE_WT_LEG1_RTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().setRegulating(bval);
            case THREE_WT_LEG1_PTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().setRegulating(bval);
            case THREE_WT_LEG2_RTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().setRegulating(bval);
            case THREE_WT_LEG2_PTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().setRegulating(bval);
            case THREE_WT_LEG3_RTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().setRegulating(bval);
            case THREE_WT_LEG3_PTC_REGULATING -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().setRegulating(bval);
            default -> throw new IllegalArgumentException("Unknown bool property for set: " + property);
        }
    }

    // ── String getters ────────────────────────────────────────────────────

    public static String getString(long handle, int property) {
        Object obj = NetworkRegistry.lookup(handle);
        return switch (property) {
            case ID   -> ((Identifiable<?>) obj).getId();
            case NAME -> ((Identifiable<?>) obj).getNameOrId();
            case TERMINAL_BUS_ID -> {
                Terminal t = (Terminal) obj;
                Bus bus = t.getBusView().getConnectableBus();
                yield bus != null ? bus.getId() : "";
            }
            default -> throw new IllegalArgumentException("Unknown string property: " + property);
        };
    }

    // ── String setters ────────────────────────────────────────────────────

    public static void setString(long handle, int property, String value) {
        Object obj = NetworkRegistry.lookup(handle);
        switch (property) {
            case NAME    -> ((Identifiable<?>) obj).setName(value);
            case COUNTRY -> ((Substation) obj).setCountry(Country.valueOf(value));
            default -> throw new IllegalArgumentException("Unknown string property for set: " + property);
        }
    }

    // ── Children (navigation) ─────────────────────────────────────────────

    public static long[] getChildren(long parentHandle, int childType) {
        Object obj = NetworkRegistry.lookup(parentHandle);
        Stream<?> children = switch (childType) {
            case GENERATOR   -> ((Network) obj).getGeneratorStream();
            case LOAD        -> ((Network) obj).getLoadStream();
            case LINE        -> ((Network) obj).getLineStream();
            case SUBSTATION  -> ((Network) obj).getSubstationStream();
            case VOLTAGE_LEVEL -> ((Network) obj).getVoltageLevelStream();
            case TWO_WINDINGS_TRANSFORMER   -> ((Network) obj).getTwoWindingsTransformerStream();
            case THREE_WINDINGS_TRANSFORMER -> ((Network) obj).getThreeWindingsTransformerStream();
            case HVDC_LINE           -> ((Network) obj).getHvdcLineStream();
            case DANGLING_LINE       -> ((Network) obj).getDanglingLineStream();
            case SHUNT_COMPENSATOR   -> ((Network) obj).getShuntCompensatorStream();
            case STATIC_VAR_COMPENSATOR -> ((Network) obj).getStaticVarCompensatorStream();
            case VSC_CONVERTER_STATION  -> ((Network) obj).getVscConverterStationStream();
            case LCC_CONVERTER_STATION  -> ((Network) obj).getLccConverterStationStream();
            case SWITCH -> {
                VoltageLevel vl = (VoltageLevel) obj;
                if (vl.getTopologyKind() == TopologyKind.NODE_BREAKER) {
                    yield StreamSupport.stream(
                        vl.getNodeBreakerView().getSwitches().spliterator(), false);
                } else {
                    yield StreamSupport.stream(
                        vl.getBusBreakerView().getSwitches().spliterator(), false);
                }
            }
            case BUSBAR_SECTION -> StreamSupport.stream(
                ((VoltageLevel) obj).getNodeBreakerView().getBusbarSections().spliterator(), false);
            case BUS -> StreamSupport.stream(
                ((VoltageLevel) obj).getBusBreakerView().getBuses().spliterator(), false);
            case BATTERY -> ((Network) obj).getBatteryStream();
            case REACTIVE_CURVE_POINT -> {
                Generator gen = (Generator) obj;
                Collection<ReactiveCapabilityCurve.Point> pts =
                    gen.getReactiveLimits(ReactiveCapabilityCurve.class).getPoints();
                yield pts.stream();
            }
            case TWO_WT_RTC_STEP -> {
                TwoWindingsTransformer twt = (TwoWindingsTransformer) obj;
                yield twt.getRatioTapChanger().getAllSteps().values().stream();
            }
            case TWO_WT_PTC_STEP -> {
                TwoWindingsTransformer twt = (TwoWindingsTransformer) obj;
                yield twt.getPhaseTapChanger().getAllSteps().values().stream();
            }
            case THREE_WT_LEG1_RTC_STEP -> {
                ThreeWindingsTransformer twt = (ThreeWindingsTransformer) obj;
                yield twt.getLeg1().getRatioTapChanger().getAllSteps().values().stream();
            }
            case THREE_WT_LEG2_RTC_STEP -> {
                ThreeWindingsTransformer twt = (ThreeWindingsTransformer) obj;
                yield twt.getLeg2().getRatioTapChanger().getAllSteps().values().stream();
            }
            case THREE_WT_LEG3_RTC_STEP -> {
                ThreeWindingsTransformer twt = (ThreeWindingsTransformer) obj;
                yield twt.getLeg3().getRatioTapChanger().getAllSteps().values().stream();
            }
            case THREE_WT_LEG1_PTC_STEP -> {
                ThreeWindingsTransformer twt = (ThreeWindingsTransformer) obj;
                yield twt.getLeg1().getPhaseTapChanger().getAllSteps().values().stream();
            }
            case THREE_WT_LEG2_PTC_STEP -> {
                ThreeWindingsTransformer twt = (ThreeWindingsTransformer) obj;
                yield twt.getLeg2().getPhaseTapChanger().getAllSteps().values().stream();
            }
            case THREE_WT_LEG3_PTC_STEP -> {
                ThreeWindingsTransformer twt = (ThreeWindingsTransformer) obj;
                yield twt.getLeg3().getPhaseTapChanger().getAllSteps().values().stream();
            }
            case SHUNT_SECTION -> {
                ShuntCompensator sc = (ShuntCompensator) obj;
                yield sc.getModel(ShuntCompensatorNonLinearModel.class).getAllSections().stream();
            }
            default -> throw new IllegalArgumentException("Unknown child type: " + childType);
        };
        return children.mapToLong(NetworkRegistry::register).toArray();
    }

    // ── Related object (navigation) ───────────────────────────────────────

    public static long getRelated(long handle, int relation) {
        Object obj = NetworkRegistry.lookup(handle);
        Object related = switch (relation) {
            case REL_TERMINAL  -> {
                if (obj instanceof Injection<?> inj) yield inj.getTerminal();
                throw new IllegalArgumentException("Object does not have a single terminal: " + obj);
            }
            case REL_TERMINAL1 -> {
                if (obj instanceof Branch<?> br) yield br.getTerminal1();
                throw new IllegalArgumentException("Object does not have terminal1: " + obj);
            }
            case REL_TERMINAL2 -> {
                if (obj instanceof Branch<?> br) yield br.getTerminal2();
                throw new IllegalArgumentException("Object does not have terminal2: " + obj);
            }
            case REL_BUS -> {
                Terminal t = (Terminal) obj;
                yield t.getBusView().getConnectableBus();
            }
            case REL_VOLTAGE_LEVEL -> ((Terminal) obj).getVoltageLevel();
            case REL_SUBSTATION    -> ((VoltageLevel) obj).getSubstation().orElse(null);
            case REL_SLACK_TERMINAL -> {
                VoltageLevel vl = (VoltageLevel) obj;
                SlackTerminal st = vl.getExtension(SlackTerminal.class);
                if (st == null) throw new IllegalStateException("SlackTerminal extension not present");
                yield st.getTerminal();
            }
            case REL_CONNECTABLE_BUS -> ((Terminal) obj).getBusBreakerView().getConnectableBus();
            default -> throw new IllegalArgumentException("Unknown relation: " + relation);
        };
        if (related == null) return 0L; // INVALID_HANDLE
        return NetworkRegistry.register(related);
    }

    // ── Find by ID ────────────────────────────────────────────────────────

    public static long findById(long networkHandle, int objectType, String id) {
        Object netObj = NetworkRegistry.lookup(networkHandle);
        Network network = (Network) netObj;
        Object found = switch (objectType) {
            case GENERATOR                  -> network.getGenerator(id);
            case LOAD                       -> network.getLoad(id);
            case LINE                       -> network.getLine(id);
            case TWO_WINDINGS_TRANSFORMER   -> network.getTwoWindingsTransformer(id);
            case THREE_WINDINGS_TRANSFORMER -> network.getThreeWindingsTransformer(id);
            case HVDC_LINE                  -> network.getHvdcLine(id);
            case DANGLING_LINE              -> network.getDanglingLine(id);
            case SHUNT_COMPENSATOR          -> network.getShuntCompensator(id);
            case STATIC_VAR_COMPENSATOR     -> network.getStaticVarCompensator(id);
            case VSC_CONVERTER_STATION      -> network.getVscConverterStation(id);
            case LCC_CONVERTER_STATION      -> network.getLccConverterStation(id);
            case SWITCH                     -> network.getSwitch(id);
            case BUSBAR_SECTION             -> network.getBusbarSection(id);
            case BATTERY                    -> network.getBattery(id);
            case VOLTAGE_LEVEL              -> network.getVoltageLevel(id);
            case SUBSTATION                 -> network.getSubstation(id);
            default -> throw new IllegalArgumentException("Unknown object type: " + objectType);
        };
        if (found == null) return 0L; // INVALID_HANDLE
        return NetworkRegistry.register(found);
    }
}
