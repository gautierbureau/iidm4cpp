package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.ShuntCompensatorNonLinearModel;
import com.powsybl.iidm.network.LoadingLimits.TemporaryLimit;
import com.powsybl.iidm.network.extensions.ActivePowerControl;
import com.powsybl.iidm.network.extensions.CoordinatedReactiveControl;
import com.powsybl.iidm.network.extensions.HvdcAngleDroopActivePowerControl;
import com.powsybl.iidm.network.extensions.HvdcOperatorActivePowerRange;
import com.powsybl.iidm.network.extensions.SlackTerminal;
import com.powsybl.iidm.network.extensions.DiscreteMeasurement;
import com.powsybl.iidm.network.extensions.DiscreteMeasurements;
import com.powsybl.iidm.network.extensions.Measurement;
import com.powsybl.iidm.network.extensions.Measurements;
import com.powsybl.iidm.network.extensions.ObservabilityArea;
import com.powsybl.commons.extensions.Extendable;
import com.powsybl.iidm.network.regulation.RegulationMode;
import com.powsybl.iidm.network.regulation.VoltageRegulation;
import com.powsybl.iidm.network.regulation.VoltageRegulationHolder;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import java.util.Collection;
import java.util.Objects;
import java.util.OptionalInt;

import java.util.stream.Stream;
import java.util.stream.StreamSupport;

import static com.powsybl.iidmbridge.PropertyCodes.*;

/**
 * Dispatches property get/set operations to the correct PowSyBl IIDM Java methods
 * based on integer property codes defined in {@link PropertyCodes}.
 *
 * Every entry point comes in two flavours:
 * <ul>
 *   <li>a handle-based one ({@code long handle}), used by the GraalVM entry points — handles are
 *       resolved through {@link NetworkRegistry};</li>
 *   <li>an object-based one ({@code Object obj}), called directly by the C++ JNI backend for every
 *       property code it does not hand-wire, so both backends share the same dispatch table.</li>
 * </ul>
 */
public final class PropertyDispatcher {

    private PropertyDispatcher() {}

    // ── Double getters ────────────────────────────────────────────────────

    public static double getDouble(long handle, int property) {
        return getDouble(NetworkRegistry.lookup(handle), property);
    }

    public static double getDouble(Object obj, int property) {
        if (isThreeWtLegCode(property)) return threeWtLegDouble(obj, property);
        return switch (property) {
            case GEN_TARGET_P  -> ((Generator) obj).getTargetP();
            case GEN_MIN_P     -> ((Generator) obj).getMinP();
            case GEN_MAX_P     -> ((Generator) obj).getMaxP();
            case GEN_RATED_S   -> ((Generator) obj).getRatedS();
            case LOCAL_TARGET_V      -> ((VoltageRegulationHolder<?>) obj).getLocalTargetV();
            case LOCAL_TARGET_Q      -> ((VoltageRegulationHolder<?>) obj).getLocalTargetQ();
            case REGULATING_TARGET_V -> ((VoltageRegulationHolder<?>) obj).getRegulatingTargetV();
            case REGULATING_TARGET_Q -> ((VoltageRegulationHolder<?>) obj).getRegulatingTargetQ();
            case VR_TARGET_VALUE    -> ((VoltageRegulation) obj).getTargetValue();
            case VR_TARGET_DEADBAND -> ((VoltageRegulation) obj).getTargetDeadband();
            case VR_SLOPE           -> ((VoltageRegulation) obj).getSlope();
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
            case DL_P0 -> ((BoundaryLine) obj).getP0();
            case DL_Q0 -> ((BoundaryLine) obj).getQ0();
            case DL_R  -> ((BoundaryLine) obj).getR();
            case DL_X  -> ((BoundaryLine) obj).getX();
            case DL_G  -> ((BoundaryLine) obj).getG();
            case DL_B  -> ((BoundaryLine) obj).getB();
            case SHUNT_B_PER_SECTION -> ((ShuntCompensator) obj).getModel(ShuntCompensatorLinearModel.class).getBPerSection();
            case SHUNT_G_PER_SECTION -> ((ShuntCompensator) obj).getModel(ShuntCompensatorLinearModel.class).getGPerSection();
            case SVC_B_MIN                   -> ((StaticVarCompensator) obj).getBmin();
            case SVC_B_MAX                   -> ((StaticVarCompensator) obj).getBmax();
            case SVC_VOLTAGE_SETPOINT        -> ((StaticVarCompensator) obj).getVoltageSetpoint();
            case SVC_REACTIVE_POWER_SETPOINT -> ((StaticVarCompensator) obj).getReactivePowerSetpoint();
            case VSC_VOLTAGE_SETPOINT        -> ((VscConverterStation) obj).getVoltageSetpoint();
            case VSC_REACTIVE_POWER_SETPOINT -> ((VscConverterStation) obj).getReactivePowerSetpoint();
            case CONVERTER_STATION_LOSS_FACTOR -> ((HvdcConverterStation<?>) obj).getLossFactor();
            case LCC_POWER_FACTOR -> ((LccConverterStation) obj).getPowerFactor();
            case EXT_APC_DROOP -> activePowerControl(obj).getDroop();
            case EXT_APC_PARTICIPATION_FACTOR -> activePowerControl(obj).getParticipationFactor();
            case EXT_APC_MIN_TARGET_P -> activePowerControl(obj).getMinTargetP().orElse(Double.NaN);
            case EXT_APC_MAX_TARGET_P -> activePowerControl(obj).getMaxTargetP().orElse(Double.NaN);
            case EXT_CRC_Q_PERCENT -> ext(obj, CoordinatedReactiveControl.class).getQPercent();
            case EXT_HADAPC_DROOP -> ext(obj, HvdcAngleDroopActivePowerControl.class).getDroop();
            case EXT_HADAPC_P0 -> ext(obj, HvdcAngleDroopActivePowerControl.class).getP0();
            case EXT_HOAR_OPR_CS1_TO_CS2 -> ext(obj, HvdcOperatorActivePowerRange.class).getOprFromCS1toCS2();
            case EXT_HOAR_OPR_CS2_TO_CS1 -> ext(obj, HvdcOperatorActivePowerRange.class).getOprFromCS2toCS1();
            case EXT_VPRC_SLOPE -> vprcRegulation(obj).getSlope();
            case TWO_WT_RTC_TARGET_V        -> ((TwoWindingsTransformer) obj).getRatioTapChanger().getTargetV();
            case TWO_WT_PTC_REG_VALUE       -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().getRegulationValue();
            case TWO_WT_RTC_TARGET_DEADBAND -> ((TwoWindingsTransformer) obj).getRatioTapChanger().getTargetDeadband();
            case TWO_WT_PTC_TARGET_DEADBAND -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().getTargetDeadband();
            case BBS_V     -> ((BusbarSection) obj).getV();
            case BBS_ANGLE -> ((BusbarSection) obj).getAngle();
            // Battery
            case BAT_TARGET_P -> ((Battery) obj).getTargetP();
            case BAT_TARGET_Q -> ((Battery) obj).getTargetQ();
            case BAT_MIN_P    -> ((Battery) obj).getMinP();
            case BAT_MAX_P    -> ((Battery) obj).getMaxP();
            case BAT_MIN_Q    -> ((Battery) obj).getReactiveLimits(MinMaxReactiveLimits.class).getMinQ();
            case BAT_MAX_Q    -> ((Battery) obj).getReactiveLimits(MinMaxReactiveLimits.class).getMaxQ();
            // ShuntCompensator total B for current section count
            case SHUNT_B -> ((ShuntCompensator) obj).getB();
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
            // CurrentLimits
            case CL_PERMANENT_LIMIT -> ((LoadingLimits) obj).getPermanentLimit();
            // TemporaryLimit
            case TL_VALUE -> ((TemporaryLimit) obj).getValue();
            // ThreeWT
            case THREE_WT_RATED_U0 -> ((ThreeWindingsTransformer) obj).getRatedU0();
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
            case SHUNT_TARGET_V  -> ((ShuntCompensator) obj).getTargetV();
            // ShuntCompensatorNonLinearModel.Section (non-linear model)
            case SHUNT_SECTION_B -> ((ShuntCompensatorNonLinearModel.Section) obj).getB();
            case SHUNT_SECTION_G -> ((ShuntCompensatorNonLinearModel.Section) obj).getG();
            case THREE_WT_LEG1_RTC_TARGET_DEADBAND -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().getTargetDeadband();
            case THREE_WT_LEG1_PTC_TARGET_DEADBAND -> ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().getTargetDeadband();
            case THREE_WT_LEG2_RTC_TARGET_DEADBAND -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().getTargetDeadband();
            case THREE_WT_LEG2_PTC_TARGET_DEADBAND -> ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().getTargetDeadband();
            case THREE_WT_LEG3_RTC_TARGET_DEADBAND -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().getTargetDeadband();
            case THREE_WT_LEG3_PTC_TARGET_DEADBAND -> ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().getTargetDeadband();
            // State estimation extensions
            case EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_STD_DEV_OFF,
                 EXT_INJ_OBS_QUALITY_Q + OBS_QUALITY_STD_DEV_OFF,
                 EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_STD_DEV_OFF,
                 EXT_BRANCH_OBS_QUALITY_P1 + OBS_QUALITY_STD_DEV_OFF,
                 EXT_BRANCH_OBS_QUALITY_P2 + OBS_QUALITY_STD_DEV_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q1 + OBS_QUALITY_STD_DEV_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q2 + OBS_QUALITY_STD_DEV_OFF
                -> StateEstimationDispatch.getQualityStandardDeviation(obj, property);
            case MEAS_VALUE   -> ((Measurement) obj).getValue();
            case MEAS_STD_DEV -> ((Measurement) obj).getStandardDeviation();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown double property: " + property);
        };
    }

    // ── Double setters ────────────────────────────────────────────────────

    public static void setDouble(long handle, int property, double value) {
        setDouble(NetworkRegistry.lookup(handle), property, value);
    }

    public static void setDouble(Object obj, int property, double value) {
        if (obj instanceof Creation.PendingAdd adder) {
            Creation.setDouble(adder, property, value);
            return;
        }
        if (isThreeWtLegCode(property)) {
            setThreeWtLegDouble(obj, property, value);
            return;
        }
        switch (property) {
            case GEN_TARGET_P  -> ((Generator) obj).setTargetP(value);
            case GEN_MIN_P     -> ((Generator) obj).setMinP(value);
            case GEN_MAX_P     -> ((Generator) obj).setMaxP(value);
            case GEN_RATED_S   -> ((Generator) obj).setRatedS(value);
            case LOCAL_TARGET_V -> ((VoltageRegulationHolder<?>) obj).setLocalTargetV(value);
            case LOCAL_TARGET_Q -> ((VoltageRegulationHolder<?>) obj).setLocalTargetQ(value);
            case VR_TARGET_VALUE    -> ((VoltageRegulation) obj).setTargetValue(value);
            case VR_TARGET_DEADBAND -> ((VoltageRegulation) obj).setTargetDeadband(value);
            case VR_SLOPE           -> ((VoltageRegulation) obj).setSlope(value);
            case LOAD_P0       -> ((Load) obj).setP0(value);
            case LOAD_Q0       -> ((Load) obj).setQ0(value);
            case TERMINAL_P    -> ((Terminal) obj).setP(value);
            case TERMINAL_Q    -> ((Terminal) obj).setQ(value);
            case BUS_V         -> ((Bus) obj).setV(value);
            case BUS_ANGLE     -> ((Bus) obj).setAngle(value);
            case HVDC_ACTIVE_POWER_SETPOINT -> ((HvdcLine) obj).setActivePowerSetpoint(value);
            case DL_P0 -> ((BoundaryLine) obj).setP0(value);
            case DL_Q0 -> ((BoundaryLine) obj).setQ0(value);
            case SVC_VOLTAGE_SETPOINT        -> ((StaticVarCompensator) obj).setVoltageSetpoint(value);
            case SVC_REACTIVE_POWER_SETPOINT -> ((StaticVarCompensator) obj).setReactivePowerSetpoint(value);
            case VSC_VOLTAGE_SETPOINT        -> ((VscConverterStation) obj).setVoltageSetpoint(value);
            case VSC_REACTIVE_POWER_SETPOINT -> ((VscConverterStation) obj).setReactivePowerSetpoint(value);
            case CONVERTER_STATION_LOSS_FACTOR -> ((HvdcConverterStation<?>) obj).setLossFactor((float) value);
            case EXT_APC_DROOP -> activePowerControl(obj).setDroop(value);
            case EXT_APC_PARTICIPATION_FACTOR -> activePowerControl(obj).setParticipationFactor(value);
            case EXT_APC_MIN_TARGET_P -> activePowerControl(obj).setMinTargetP(value);
            case EXT_APC_MAX_TARGET_P -> activePowerControl(obj).setMaxTargetP(value);
            case EXT_CRC_Q_PERCENT -> ext(obj, CoordinatedReactiveControl.class).setQPercent(value);
            case EXT_HADAPC_DROOP -> ext(obj, HvdcAngleDroopActivePowerControl.class).setDroop((float) value);
            case EXT_HADAPC_P0 -> ext(obj, HvdcAngleDroopActivePowerControl.class).setP0((float) value);
            case EXT_HOAR_OPR_CS1_TO_CS2 -> ext(obj, HvdcOperatorActivePowerRange.class).setOprFromCS1toCS2((float) value);
            case EXT_HOAR_OPR_CS2_TO_CS1 -> ext(obj, HvdcOperatorActivePowerRange.class).setOprFromCS2toCS1((float) value);
            case EXT_VPRC_SLOPE -> vprcRegulation(obj).setSlope(value);
            case TWO_WT_RTC_TARGET_V  -> ((TwoWindingsTransformer) obj).getRatioTapChanger().setTargetV(value);
            case TWO_WT_PTC_REG_VALUE -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().setRegulationValue(value);
            case BAT_TARGET_P -> ((Battery) obj).setTargetP(value);
            case BAT_TARGET_Q -> ((Battery) obj).setTargetQ(value);
            case EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_STD_DEV_OFF,
                 EXT_INJ_OBS_QUALITY_Q + OBS_QUALITY_STD_DEV_OFF,
                 EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_STD_DEV_OFF,
                 EXT_BRANCH_OBS_QUALITY_P1 + OBS_QUALITY_STD_DEV_OFF,
                 EXT_BRANCH_OBS_QUALITY_P2 + OBS_QUALITY_STD_DEV_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q1 + OBS_QUALITY_STD_DEV_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q2 + OBS_QUALITY_STD_DEV_OFF
                -> StateEstimationDispatch.setQualityStandardDeviation(obj, property, value);
            case MEAS_VALUE   -> ((Measurement) obj).setValue(value);
            case MEAS_STD_DEV -> ((Measurement) obj).setStandardDeviation(value);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown double property for set: " + property);
        }
    }

    // ── Int getters ───────────────────────────────────────────────────────

    public static int getInt(long handle, int property) {
        return getInt(NetworkRegistry.lookup(handle), property);
    }

    public static int getInt(Object obj, int property) {
        if (isThreeWtLegCode(property)) return threeWtLegInt(obj, property);
        return switch (property) {
            case IDENTIFIABLE_TYPE  -> ((Identifiable<?>) obj).getType().ordinal();
            case CONVERTER_STATION_HVDC_TYPE -> ((HvdcConverterStation<?>) obj).getHvdcType().ordinal();
            case TERMINAL_SIDE -> {
                ThreeSides side = ((Terminal) obj).getSide();
                yield side == null ? -1 : side.ordinal();
            }
            case GEN_ENERGY_SOURCE  -> ((Generator) obj).getEnergySource().ordinal();
            case NETWORK_FORECAST_DISTANCE -> ((Network) obj).getForecastDistance();
            case VR_MODE -> {
                RegulationMode mode = ((VoltageRegulation) obj).getMode();
                yield mode == null ? -1 : mode.ordinal();
            }
            case LOAD_TYPE          -> ((Load) obj).getLoadType().ordinal();
            case VL_TOPOLOGY_KIND   -> ((VoltageLevel) obj).getTopologyKind().ordinal();
            case HVDC_CONVERTERS_MODE -> ((HvdcLine) obj).getConvertersMode().ordinal();
            case SVC_REGULATION_MODE  -> svcRegulationModeToCode((StaticVarCompensator) obj);
            case SHUNT_SECTION_COUNT     -> ((ShuntCompensator) obj).getSectionCount();
            case SHUNT_MAX_SECTION_COUNT -> ((ShuntCompensator) obj).getMaximumSectionCount();
            case SW_KIND  -> ((Switch) obj).getKind().ordinal();
            case SW_NODE1 -> ((Switch) obj).getVoltageLevel().getNodeBreakerView().getNode1(((Switch) obj).getId());
            case SW_NODE2 -> ((Switch) obj).getVoltageLevel().getNodeBreakerView().getNode2(((Switch) obj).getId());
            case IC_NODE1 -> ((VoltageLevel.NodeBreakerView.InternalConnection) obj).getNode1();
            case IC_NODE2 -> ((VoltageLevel.NodeBreakerView.InternalConnection) obj).getNode2();
            case TL_ACCEPTABLE_DURATION -> ((TemporaryLimit) obj).getAcceptableDuration();
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
            // Battery reactive limits kind: 0=NONE, 1=MIN_MAX, 2=CURVE
            case BAT_REACTIVE_LIMITS_KIND -> {
                ReactiveLimits rl = ((Battery) obj).getReactiveLimits();
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
            // State estimation extensions
            case EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_INJ_OBS_QUALITY_Q + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_BRANCH_OBS_QUALITY_P1 + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_BRANCH_OBS_QUALITY_P2 + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q1 + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q2 + OBS_QUALITY_REDUNDANT_OFF
                -> StateEstimationDispatch.getQualityRedundant(obj, property);
            case MEAS_TYPE          -> ((Measurement) obj).getType().ordinal();
            case MEAS_SIDE          -> StateEstimationDispatch.measurementSide((Measurement) obj);
            case DMEAS_TYPE         -> ((DiscreteMeasurement) obj).getType().ordinal();
            case DMEAS_TAP_CHANGER  -> StateEstimationDispatch.discreteMeasurementTapChanger((DiscreteMeasurement) obj);
            case DMEAS_VALUE_TYPE   -> ((DiscreteMeasurement) obj).getValueType().ordinal();
            case DMEAS_VALUE_INT    -> ((DiscreteMeasurement) obj).getValueAsInt();
            case OBS_AREA_NUMBER    -> ((StateEstimationDispatch.ObservabilityAreaRef) obj).area().getAreaNumber();
            case OBS_AREA_STATUS    -> ((StateEstimationDispatch.ObservabilityAreaRef) obj).area().getStatus().ordinal();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown int property: " + property);
        };
    }

    // ── Int setters ───────────────────────────────────────────────────────

    public static void setInt(long handle, int property, int value) {
        setInt(NetworkRegistry.lookup(handle), property, value);
    }

    public static void setInt(Object obj, int property, int value) {
        if (obj instanceof Creation.PendingAdd adder) {
            Creation.setInt(adder, property, value);
            return;
        }
        if (isThreeWtLegCode(property)) {
            setThreeWtLegInt(obj, property, value);
            return;
        }
        switch (property) {
            case GEN_ENERGY_SOURCE -> ((Generator) obj).setEnergySource(Creation.enumValue(EnergySource.class, value));
            case NETWORK_FORECAST_DISTANCE -> ((Network) obj).setForecastDistance(value);
            case VR_MODE -> ((VoltageRegulation) obj).setMode(value < 0 ? null : Creation.enumValue(RegulationMode.class, value));
            case HVDC_CONVERTERS_MODE ->
                ((HvdcLine) obj).setConvertersMode(HvdcLine.ConvertersMode.values()[value]);
            case SVC_REGULATION_MODE -> setSvcRegulationModeFromCode((StaticVarCompensator) obj, value);
            case SHUNT_SECTION_COUNT ->
                ((ShuntCompensator) obj).setSectionCount(value);
            case SHUNT_SOLVED_SECTION_COUNT -> ((ShuntCompensator) obj).setSolvedSectionCount(value);
            case TWO_WT_RTC_TAP_POSITION ->
                ((TwoWindingsTransformer) obj).getRatioTapChanger().setTapPosition(value);
            case TWO_WT_PTC_TAP_POSITION ->
                ((TwoWindingsTransformer) obj).getPhaseTapChanger().setTapPosition(value);
            case TWO_WT_PTC_REG_MODE ->
                ((TwoWindingsTransformer) obj).getPhaseTapChanger().setRegulationMode(
                    PhaseTapChanger.RegulationMode.values()[value]);
            case EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_INJ_OBS_QUALITY_Q + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_BRANCH_OBS_QUALITY_P1 + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_BRANCH_OBS_QUALITY_P2 + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q1 + OBS_QUALITY_REDUNDANT_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q2 + OBS_QUALITY_REDUNDANT_OFF
                -> StateEstimationDispatch.setQualityRedundant(obj, property, value);
            case DMEAS_VALUE_INT -> ((DiscreteMeasurement) obj).setValue(value);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown int property for set: " + property);
        }
    }

    // ── Bool getters ──────────────────────────────────────────────────────

    public static int getBool(long handle, int property) {
        return getBool(NetworkRegistry.lookup(handle), property) ? 1 : 0;
    }

    public static boolean getBool(Object obj, int property) {
        if (isThreeWtLegCode(property)) return threeWtLegBool(obj, property);
        return switch (property) {
            // Identifiable type codes: whether the object is one
            case IDENTIFIABLE               -> obj instanceof Identifiable<?>;
            case CONNECTABLE                -> obj instanceof Connectable<?>;
            case INJECTION                  -> obj instanceof Injection<?>;
            case BRANCH                     -> obj instanceof Branch<?>;
            case HVDC_CONVERTER_STATION     -> obj instanceof HvdcConverterStation<?>;
            case SUBSTATION                 -> obj instanceof Substation;
            case VOLTAGE_LEVEL              -> obj instanceof VoltageLevel;
            case GENERATOR                  -> obj instanceof Generator;
            case LOAD                       -> obj instanceof Load;
            case LINE                       -> obj instanceof Line;
            case TWO_WINDINGS_TRANSFORMER   -> obj instanceof TwoWindingsTransformer;
            case THREE_WINDINGS_TRANSFORMER -> obj instanceof ThreeWindingsTransformer;
            case HVDC_LINE                  -> obj instanceof HvdcLine;
            case DANGLING_LINE              -> obj instanceof BoundaryLine;
            case SHUNT_COMPENSATOR          -> obj instanceof ShuntCompensator;
            case STATIC_VAR_COMPENSATOR     -> obj instanceof StaticVarCompensator;
            case VSC_CONVERTER_STATION      -> obj instanceof VscConverterStation;
            case LCC_CONVERTER_STATION      -> obj instanceof LccConverterStation;
            case BUS                        -> obj instanceof Bus;
            case SWITCH                     -> obj instanceof Switch;
            case BUSBAR_SECTION             -> obj instanceof BusbarSection;
            case BATTERY                    -> obj instanceof Battery;
            case FICTITIOUS                 -> ((Identifiable<?>) obj).isFictitious();
            case EQUIVALENT                 -> ((Connectable<?>) obj).isEquivalent();
            case GEN_CONDENSER              -> ((Generator) obj).isCondenser();
            case REGULATING                 -> ((VoltageRegulationHolder<?>) obj).isRegulating();
            case VR_REGULATING              -> ((VoltageRegulation) obj).isRegulating();
            case VR_WITH_TERMINAL           -> ((VoltageRegulation) obj).isWithTerminal();
            case TERMINAL_CONNECTED         -> ((Terminal) obj).isConnected();
            case VSC_VOLTAGE_REGULATOR_ON   -> ((VscConverterStation) obj).isVoltageRegulatorOn();
            case SHUNT_VOLTAGE_REGULATOR_ON -> ((ShuntCompensator) obj).isVoltageRegulatorOn();
            case TL_FICTITIOUS -> ((TemporaryLimit) obj).isFictitious();
            case SW_OPEN     -> ((Switch) obj).isOpen();
            case SW_RETAINED -> ((Switch) obj).isRetained();
            case TWO_WT_RTC_EXISTS     -> ((TwoWindingsTransformer) obj).getRatioTapChanger() != null;
            case TWO_WT_RTC_REGULATING -> ((TwoWindingsTransformer) obj).getRatioTapChanger().isRegulating();
            case TWO_WT_PTC_EXISTS     -> ((TwoWindingsTransformer) obj).getPhaseTapChanger() != null;
            case TWO_WT_PTC_REGULATING -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().isRegulating();
            case EXT_APC_PARTICIPATE -> activePowerControl(obj).isParticipate();
            case EXT_CRC_EXISTS      -> ((Generator) obj).getExtension(CoordinatedReactiveControl.class) != null;
            case EXT_HADAPC_EXISTS   -> ((HvdcLine) obj).getExtension(HvdcAngleDroopActivePowerControl.class) != null;
            case EXT_HADAPC_ENABLED  -> ext(obj, HvdcAngleDroopActivePowerControl.class).isEnabled();
            case EXT_HOAR_EXISTS     -> ((HvdcLine) obj).getExtension(HvdcOperatorActivePowerRange.class) != null;
            case EXT_VPRC_EXISTS     -> isVoltagePerReactivePowerControl(obj);
            case EXT_ST_EXISTS       -> ((VoltageLevel) obj).getExtension(SlackTerminal.class) != null;
            case EXT_ST_EMPTY        -> slackTerminal(obj).isEmpty();
            case OLG_EMPTY -> ((OperationalLimitsGroup) obj).isEmpty();
            case TWO_WT_RTC_LOAD_TAP_CAP -> ((TwoWindingsTransformer) obj).getRatioTapChanger().hasLoadTapChangingCapabilities();
            // State estimation extensions
            case EXT_TWO_WT_TBE_EXISTS, EXT_TWO_WT_TBE_RTC, EXT_TWO_WT_TBE_PTC,
                 EXT_THREE_WT_TBE_EXISTS, EXT_THREE_WT_TBE_RTC1, EXT_THREE_WT_TBE_RTC2, EXT_THREE_WT_TBE_RTC3,
                 EXT_THREE_WT_TBE_PTC1, EXT_THREE_WT_TBE_PTC2, EXT_THREE_WT_TBE_PTC3
                -> StateEstimationDispatch.getToBeEstimated(obj, property);
            case EXT_INJ_OBS_EXISTS, EXT_INJ_OBS_OBSERVABLE, EXT_BRANCH_OBS_EXISTS, EXT_BRANCH_OBS_OBSERVABLE
                -> StateEstimationDispatch.getObservabilityFlag(obj, property);
            case EXT_INJ_OBS_QUALITY_P + OBS_QUALITY_EXISTS_OFF,
                 EXT_INJ_OBS_QUALITY_Q + OBS_QUALITY_EXISTS_OFF,
                 EXT_INJ_OBS_QUALITY_V + OBS_QUALITY_EXISTS_OFF,
                 EXT_BRANCH_OBS_QUALITY_P1 + OBS_QUALITY_EXISTS_OFF,
                 EXT_BRANCH_OBS_QUALITY_P2 + OBS_QUALITY_EXISTS_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q1 + OBS_QUALITY_EXISTS_OFF,
                 EXT_BRANCH_OBS_QUALITY_Q2 + OBS_QUALITY_EXISTS_OFF
                -> StateEstimationDispatch.qualityExists(obj, property);
            case EXT_MEAS_EXISTS  -> StateEstimationDispatch.findExtension(obj, Measurements.class) != null;
            case MEAS_VALID       -> ((Measurement) obj).isValid();
            case EXT_DMEAS_EXISTS -> StateEstimationDispatch.findExtension(obj, DiscreteMeasurements.class) != null;
            case DMEAS_VALUE_BOOL -> ((DiscreteMeasurement) obj).getValueAsBoolean();
            case DMEAS_VALID      -> ((DiscreteMeasurement) obj).isValid();
            case EXT_OBS_AREA_EXISTS     -> ((VoltageLevel) obj).getExtension(ObservabilityArea.class) != null;
            case EXT_OBS_AREA_CONSISTENT -> StateEstimationDispatch.extension(obj, ObservabilityArea.class) instanceof ObservabilityArea oa
                                            && oa.isConsistentWithTopology();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown bool property: " + property);
        };
    }

    // ── Bool setters ──────────────────────────────────────────────────────

    public static void setBool(long handle, int property, int value) {
        setBool(NetworkRegistry.lookup(handle), property, value != 0);
    }

    public static void setBool(Object obj, int property, boolean bval) {
        if (obj instanceof Creation.PendingAdd adder) {
            Creation.setBool(adder, property, bval);
            return;
        }
        if (isThreeWtLegCode(property)) {
            setThreeWtLegBool(obj, property, bval);
            return;
        }
        switch (property) {
            case FICTITIOUS -> ((Identifiable<?>) obj).setFictitious(bval);
            case EQUIVALENT -> ((Connectable<?>) obj).setEquivalent(bval);
            case VR_REGULATING -> ((VoltageRegulation) obj).setRegulating(bval);
            case VSC_VOLTAGE_REGULATOR_ON -> ((VscConverterStation) obj).setVoltageRegulatorOn(bval);
            case SW_OPEN     -> ((Switch) obj).setOpen(bval);
            case SW_RETAINED -> ((Switch) obj).setRetained(bval);
            case TWO_WT_RTC_REGULATING -> ((TwoWindingsTransformer) obj).getRatioTapChanger().setRegulating(bval);
            case TWO_WT_PTC_REGULATING -> ((TwoWindingsTransformer) obj).getPhaseTapChanger().setRegulating(bval);
            case EXT_APC_PARTICIPATE     -> activePowerControl(obj).setParticipate(bval);
            case EXT_HADAPC_ENABLED  -> ext(obj, HvdcAngleDroopActivePowerControl.class).setEnabled(bval);
            case EXT_TWO_WT_TBE_RTC, EXT_TWO_WT_TBE_PTC,
                 EXT_THREE_WT_TBE_RTC1, EXT_THREE_WT_TBE_RTC2, EXT_THREE_WT_TBE_RTC3,
                 EXT_THREE_WT_TBE_PTC1, EXT_THREE_WT_TBE_PTC2, EXT_THREE_WT_TBE_PTC3
                -> StateEstimationDispatch.setToBeEstimated(obj, property, bval);
            case EXT_INJ_OBS_OBSERVABLE, EXT_BRANCH_OBS_OBSERVABLE -> StateEstimationDispatch.setObservable(obj, property, bval);
            case MEAS_VALID       -> ((Measurement) obj).setValid(bval);
            case DMEAS_VALUE_BOOL -> ((DiscreteMeasurement) obj).setValue(bval);
            case DMEAS_VALID      -> ((DiscreteMeasurement) obj).setValid(bval);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown bool property for set: " + property);
        }
    }

    // ── String getters ────────────────────────────────────────────────────

    public static String getString(long handle, int property) {
        return getString(NetworkRegistry.lookup(handle), property);
    }

    public static String getString(Object obj, int property) {
        return switch (property) {
            case ID   -> ((Identifiable<?>) obj).getId();
            case NAME -> ((Identifiable<?>) obj).getNameOrId();
            case TL_NAME -> ((TemporaryLimit) obj).getName();
            case TERMINAL_BUS_ID -> {
                Terminal t = (Terminal) obj;
                Bus bus = t.getBusView().getConnectableBus();
                yield bus != null ? bus.getId() : "";
            }
            case TERMINAL_BUS_BREAKER_BUS_ID -> {
                Bus bus = ((Terminal) obj).getBusBreakerView().getBus();
                yield bus != null ? bus.getId() : "";
            }
            case TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID -> {
                Bus bus = ((Terminal) obj).getBusBreakerView().getConnectableBus();
                yield bus != null ? bus.getId() : "";
            }
            case SUBSTATION_TSO -> {
                String tso = ((Substation) obj).getTso();
                yield tso != null ? tso : "";
            }
            case NETWORK_CASE_DATE -> ((Network) obj).getCaseDate().toString();
            case NETWORK_SOURCE_FORMAT -> ((Network) obj).getSourceFormat();
            case SHUNT_REGULATING_TERMINAL_ID -> {
                Terminal rt = ((ShuntCompensator) obj).getRegulatingTerminal();
                yield rt != null ? rt.getConnectable().getId() : "";
            }
            case TWO_WT_RTC_REG_TERMINAL_ID -> {
                Terminal rt = ((TwoWindingsTransformer) obj).getRatioTapChanger().getRegulationTerminal();
                yield rt != null ? rt.getConnectable().getId() : "";
            }
            case TWO_WT_PTC_REG_TERMINAL_ID -> {
                Terminal rt = ((TwoWindingsTransformer) obj).getPhaseTapChanger().getRegulationTerminal();
                yield rt != null ? rt.getConnectable().getId() : "";
            }
            case THREE_WT_LEG1_RTC_REG_TERMINAL_ID -> {
                Terminal rt = ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().getRegulationTerminal();
                yield rt != null ? rt.getConnectable().getId() : "";
            }
            case THREE_WT_LEG1_PTC_REG_TERMINAL_ID -> {
                Terminal rt = ((ThreeWindingsTransformer) obj).getLeg1().getPhaseTapChanger().getRegulationTerminal();
                yield rt != null ? rt.getConnectable().getId() : "";
            }
            case THREE_WT_LEG2_RTC_REG_TERMINAL_ID -> {
                Terminal rt = ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().getRegulationTerminal();
                yield rt != null ? rt.getConnectable().getId() : "";
            }
            case THREE_WT_LEG2_PTC_REG_TERMINAL_ID -> {
                Terminal rt = ((ThreeWindingsTransformer) obj).getLeg2().getPhaseTapChanger().getRegulationTerminal();
                yield rt != null ? rt.getConnectable().getId() : "";
            }
            case THREE_WT_LEG3_RTC_REG_TERMINAL_ID -> {
                Terminal rt = ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().getRegulationTerminal();
                yield rt != null ? rt.getConnectable().getId() : "";
            }
            case THREE_WT_LEG3_PTC_REG_TERMINAL_ID -> {
                Terminal rt = ((ThreeWindingsTransformer) obj).getLeg3().getPhaseTapChanger().getRegulationTerminal();
                yield rt != null ? rt.getConnectable().getId() : "";
            }
            case OLG_ID -> ((OperationalLimitsGroup) obj).getId();
            case COUNTRY -> ((Substation) obj).getCountry().map(Country::name).orElse("");
            case MEAS_ID            -> ((Measurement) obj).getId();
            case DMEAS_ID           -> ((DiscreteMeasurement) obj).getId();
            case DMEAS_VALUE_STRING -> ((DiscreteMeasurement) obj).getValueAsString();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown string property: " + property);
        };
    }

    // ── String setters ────────────────────────────────────────────────────

    public static void setString(long handle, int property, String value) {
        setString(NetworkRegistry.lookup(handle), property, value);
    }

    public static void setString(Object obj, int property, String value) {
        if (obj instanceof Creation.PendingAdd adder) {
            Creation.setString(adder, property, value);
            return;
        }
        switch (property) {
            case NAME    -> ((Identifiable<?>) obj).setName(value);
            case COUNTRY -> ((Substation) obj).setCountry(Country.valueOf(value));
            case SUBSTATION_TSO -> ((Substation) obj).setTso(value);
            case NETWORK_CASE_DATE -> ((Network) obj).setCaseDate(java.time.ZonedDateTime.parse(value));
            case DMEAS_VALUE_STRING -> ((DiscreteMeasurement) obj).setValue(value);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown string property for set: " + property);
        }
    }

    // ── Optional values ───────────────────────────────────────────────────
    //
    // Some properties may have no value. getOptInt / getOptString give it as it is (no value:
    // NO_INT, null), while getInt / getString give a value or fail (getString(NAME) is the name,
    // or else the id). Any int or string property can be read as an optional one.

    /** getOptInt: the property has no value (outside the int range). */
    public static final long NO_INT = Long.MIN_VALUE;

    public static long getOptInt(long handle, int property) {
        return getOptInt(NetworkRegistry.lookup(handle), property);
    }

    public static long getOptInt(Object obj, int property) {
        OptionalInt value = switch (property) {
            case SHUNT_SECTION_COUNT        -> ((ShuntCompensator) obj).findSectionCount();
            case SHUNT_SOLVED_SECTION_COUNT -> ((ShuntCompensator) obj).findSolvedSectionCount();
            default -> OptionalInt.of(getInt(obj, property));
        };
        return value.isPresent() ? value.getAsInt() : NO_INT;
    }

    public static String getOptString(long handle, int property) {
        return getOptString(NetworkRegistry.lookup(handle), property);
    }

    public static String getOptString(Object obj, int property) {
        return switch (property) {
            case NAME           -> ((Identifiable<?>) obj).getOptionalName().orElse(null);
            case SUBSTATION_TSO -> ((Substation) obj).getTso();
            case COUNTRY        -> ((Substation) obj).getCountry().map(Country::name).orElse(null);
            default -> getString(obj, property);
        };
    }

    // ── String lists ──────────────────────────────────────────────────────

    public static String[] getStringList(long handle, int listCode) {
        return getStringList(NetworkRegistry.lookup(handle), listCode);
    }

    /** Sets are sorted, so that both backends list them in the same order. */
    public static String[] getStringList(Object obj, int listCode) {
        Collection<String> values = switch (listCode) {
            case PROPERTY_NAMES -> ((BasePropertiesHolder) obj).getPropertyNames();
            case ALIASES        -> ((Identifiable<?>) obj).getAliases();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown string list code: " + listCode);
        };
        return values.stream().sorted().toArray(String[]::new);
    }

    // ── Keyed strings (properties, aliases) ───────────────────────────────

    public static String getKeyedString(long handle, int property, String key) {
        return getKeyedString(NetworkRegistry.lookup(handle), property, key);
    }

    /** The value under a key, null when there is none. */
    public static String getKeyedString(Object obj, int property, String key) {
        return switch (property) {
            case PROPERTY        -> ((BasePropertiesHolder) obj).getProperty(key);
            case ALIAS_TYPE      -> ((Identifiable<?>) obj).getAliasType(key).orElse(null);
            case ALIAS_FROM_TYPE -> ((Identifiable<?>) obj).getAliasFromType(key).orElse(null);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown keyed property: " + property);
        };
    }

    public static void setKeyedString(long handle, int property, String key, String value) {
        setKeyedString(NetworkRegistry.lookup(handle), property, key, value);
    }

    /** Sets the value under a key; a null value removes the key. */
    public static void setKeyedString(Object obj, int property, String key, String value) {
        if (obj instanceof Creation.PendingAdd adder) {
            Creation.setKeyedString(adder, property, key, value);
            return;
        }
        switch (property) {
            case PROPERTY -> {
                if (value == null) ((PropertiesHolder) obj).removeProperty(key);
                else ((BasePropertiesHolder) obj).setProperty(key, value);
            }
            case ALIAS_TYPE -> {
                if (value == null) ((Identifiable<?>) obj).removeAlias(key);
                else ((Identifiable<?>) obj).addAlias(key, value);
            }
            case UNIQUE_ALIAS_TYPE -> ((Identifiable<?>) obj).addAlias(key, Objects.requireNonNull(value), true);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown keyed property for set: " + property);
        }
    }

    // ── Identity ──────────────────────────────────────────────────────────

    /** Whether two handles stand for the same object (handles are not unique: each lookup makes one). */
    public static int isSameObject(long handle, long otherHandle) {
        return isSameObject(NetworkRegistry.lookup(handle), NetworkRegistry.lookup(otherHandle)) ? 1 : 0;
    }

    public static boolean isSameObject(Object obj, Object other) {
        return Objects.equals(obj, other);
    }

    // ── Children (navigation) ─────────────────────────────────────────────

    public static long[] getChildren(long parentHandle, int childType) {
        return childStream(NetworkRegistry.lookup(parentHandle), childType)
            .mapToLong(NetworkRegistry::register).toArray();
    }

    public static Object[] getChildObjects(Object obj, int childType) {
        return childStream(obj, childType).toArray();
    }

    private static Stream<?> childStream(Object obj, int childType) {
        return switch (childType) {
            case GENERATOR -> {
                if (obj instanceof VoltageLevel vl) yield StreamSupport.stream(vl.getConnectables(Generator.class).spliterator(), false);
                yield ((Network) obj).getGeneratorStream();
            }
            case LOAD -> {
                if (obj instanceof VoltageLevel vl) yield StreamSupport.stream(vl.getConnectables(Load.class).spliterator(), false);
                yield ((Network) obj).getLoadStream();
            }
            case BATTERY -> {
                if (obj instanceof VoltageLevel vl) yield StreamSupport.stream(vl.getConnectables(Battery.class).spliterator(), false);
                yield ((Network) obj).getBatteryStream();
            }
            case SHUNT_COMPENSATOR -> {
                if (obj instanceof VoltageLevel vl) yield StreamSupport.stream(vl.getConnectables(ShuntCompensator.class).spliterator(), false);
                yield ((Network) obj).getShuntCompensatorStream();
            }
            case STATIC_VAR_COMPENSATOR -> {
                if (obj instanceof VoltageLevel vl) yield StreamSupport.stream(vl.getConnectables(StaticVarCompensator.class).spliterator(), false);
                yield ((Network) obj).getStaticVarCompensatorStream();
            }
            case DANGLING_LINE -> {
                if (obj instanceof VoltageLevel vl) yield StreamSupport.stream(vl.getConnectables(BoundaryLine.class).spliterator(), false);
                yield ((Network) obj).getBoundaryLineStream();
            }
            case LCC_CONVERTER_STATION -> {
                if (obj instanceof VoltageLevel vl) yield StreamSupport.stream(vl.getConnectables(LccConverterStation.class).spliterator(), false);
                yield ((Network) obj).getLccConverterStationStream();
            }
            case VSC_CONVERTER_STATION -> {
                if (obj instanceof VoltageLevel vl) yield StreamSupport.stream(vl.getConnectables(VscConverterStation.class).spliterator(), false);
                yield ((Network) obj).getVscConverterStationStream();
            }
            case LINE        -> ((Network) obj).getLineStream();
            case IDENTIFIABLE -> ((Network) obj).getIdentifiables().stream();
            case CONNECTABLE -> {
                if (obj instanceof VoltageLevel vl) yield vl.getConnectableStream();
                yield ((Network) obj).getConnectableStream();
            }
            case BRANCH -> ((Network) obj).getBranchStream();
            case HVDC_CONVERTER_STATION -> ((Network) obj).getHvdcConverterStationStream();
            case TERMINAL -> ((Connectable<?>) obj).getTerminals().stream();
            case SUBSTATION  -> ((Network) obj).getSubstationStream();
            case VOLTAGE_LEVEL -> {
                if (obj instanceof Substation sub)
                    yield StreamSupport.stream(sub.getVoltageLevels().spliterator(), false);
                yield ((Network) obj).getVoltageLevelStream();
            }
            case TWO_WINDINGS_TRANSFORMER -> {
                if (obj instanceof Substation sub)
                    yield StreamSupport.stream(sub.getTwoWindingsTransformers().spliterator(), false);
                yield ((Network) obj).getTwoWindingsTransformerStream();
            }
            case THREE_WINDINGS_TRANSFORMER -> {
                if (obj instanceof Substation sub)
                    yield StreamSupport.stream(sub.getThreeWindingsTransformers().spliterator(), false);
                yield ((Network) obj).getThreeWindingsTransformerStream();
            }
            case HVDC_LINE           -> ((Network) obj).getHvdcLineStream();
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
            case INTERNAL_CONNECTION -> StreamSupport.stream(
                ((VoltageLevel) obj).getNodeBreakerView().getInternalConnections().spliterator(), false);
            case BUS -> StreamSupport.stream(
                ((VoltageLevel) obj).getBusBreakerView().getBuses().spliterator(), false);
            case REACTIVE_CURVE_POINT -> {
                ReactiveLimitsHolder holder = (ReactiveLimitsHolder) obj;
                Collection<ReactiveCapabilityCurve.Point> pts =
                    holder.getReactiveLimits(ReactiveCapabilityCurve.class).getPoints();
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
            case TEMPORARY_LIMIT -> ((LoadingLimits) obj).getTemporaryLimits().stream();
            case OLG_SIDE1 -> ((Branch<?>) obj).getOperationalLimitsGroups1().stream();
            case OLG_SIDE2 -> ((Branch<?>) obj).getOperationalLimitsGroups2().stream();
            case OLG_LEG1  -> ((ThreeWindingsTransformer) obj).getLeg1().getOperationalLimitsGroups().stream();
            case OLG_LEG2  -> ((ThreeWindingsTransformer) obj).getLeg2().getOperationalLimitsGroups().stream();
            case OLG_LEG3  -> ((ThreeWindingsTransformer) obj).getLeg3().getOperationalLimitsGroups().stream();
            case MEASUREMENT          -> StateEstimationDispatch.measurements(obj);
            case DISCRETE_MEASUREMENT -> StateEstimationDispatch.discreteMeasurements(obj);
            case OBSERVABILITY_AREA   -> StateEstimationDispatch.observabilityAreas(obj);
            case OBS_AREA_TERMINALS   -> StateEstimationDispatch.observabilityAreaTerminals(obj);
            case OBS_AREA_BUSES       -> StateEstimationDispatch.observabilityAreaBuses(obj);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown child type: " + childType);
        };
    }

    // ── Related object (navigation) ───────────────────────────────────────

    public static long getRelated(long handle, int relation) {
        return registerOrZero(getRelatedObject(NetworkRegistry.lookup(handle), relation));
    }

    public static Object getRelatedObject(Object obj, int relation) {
        return switch (relation) {
            case REL_TERMINAL  -> {
                if (obj instanceof Injection<?> inj) yield inj.getTerminal();
                if (obj instanceof VoltageRegulation vr) yield vr.getTerminal();
                throw new BridgeErrors.PropertyNotFoundException("Object does not have a single terminal: " + obj);
            }
            case REL_TERMINAL1 -> {
                if (obj instanceof Branch<?> br) yield br.getTerminal1();
                throw new BridgeErrors.PropertyNotFoundException("Object does not have terminal1: " + obj);
            }
            case REL_TERMINAL2 -> {
                if (obj instanceof Branch<?> br) yield br.getTerminal2();
                throw new BridgeErrors.PropertyNotFoundException("Object does not have terminal2: " + obj);
            }
            case REL_BUS -> {
                Terminal t = (Terminal) obj;
                yield t.getBusView().getConnectableBus();
            }
            case REL_VOLTAGE_LEVEL -> {
                if (obj instanceof Bus b) yield b.getVoltageLevel();
                yield ((Terminal) obj).getVoltageLevel();
            }
            case REL_HVDC_LINE -> ((HvdcConverterStation<?>) obj).getHvdcLine();
            case REL_CONVERTER_STATION1 -> ((HvdcLine) obj).getConverterStation1();
            case REL_CONVERTER_STATION2 -> ((HvdcLine) obj).getConverterStation2();
            case REL_OTHER_CONVERTER_STATION -> ((HvdcConverterStation<?>) obj).getOtherConverterStation().orElse(null);
            case REL_TWO_WT_RTC_REG_TERMINAL        -> ((TwoWindingsTransformer) obj).getRatioTapChanger().getRegulationTerminal();
            case REL_THREE_WT_LEG1_RTC_REG_TERMINAL -> ((ThreeWindingsTransformer) obj).getLeg1().getRatioTapChanger().getRegulationTerminal();
            case REL_THREE_WT_LEG2_RTC_REG_TERMINAL -> ((ThreeWindingsTransformer) obj).getLeg2().getRatioTapChanger().getRegulationTerminal();
            case REL_THREE_WT_LEG3_RTC_REG_TERMINAL -> ((ThreeWindingsTransformer) obj).getLeg3().getRatioTapChanger().getRegulationTerminal();
            case REL_SUBSTATION -> {
                if (obj instanceof ThreeWindingsTransformer twt)
                    yield twt.getLeg1().getTerminal().getVoltageLevel().getSubstation().orElse(null);
                yield ((VoltageLevel) obj).getSubstation().orElse(null);
            }
            case REL_SLACK_TERMINAL -> {
                VoltageLevel vl = (VoltageLevel) obj;
                SlackTerminal st = vl.getExtension(SlackTerminal.class);
                if (st == null) throw new BridgeErrors.ExtensionNotFoundException("SlackTerminal extension not present on " + vl.getId());
                yield st.getTerminal();
            }
            case REL_CONNECTABLE_BUS -> ((Terminal) obj).getBusBreakerView().getConnectableBus();
            case REL_REGULATING_TERMINAL -> {
                if (obj instanceof ShuntCompensator sc) yield sc.getRegulatingTerminal();
                if (obj instanceof StaticVarCompensator svc) yield svc.getRegulatingTerminal();
                if (obj instanceof Generator gen) yield gen.getRegulatingTerminal();
                if (obj instanceof VscConverterStation vsc) yield vsc.getRegulatingTerminal();
                throw new BridgeErrors.PropertyNotFoundException("Object does not have a regulating terminal: " + obj);
            }
            case REL_CURRENT_LIMITS1 -> {
                if (obj instanceof ThreeWindingsTransformer twt)
                    yield twt.getLeg1().getCurrentLimits().orElse(null);
                if (obj instanceof BoundaryLine dl)
                    yield dl.getCurrentLimits().orElse(null);
                yield ((Branch<?>) obj).getCurrentLimits1().orElse(null);
            }
            case REL_CURRENT_LIMITS2 -> {
                if (obj instanceof ThreeWindingsTransformer twt)
                    yield twt.getLeg2().getCurrentLimits().orElse(null);
                yield ((Branch<?>) obj).getCurrentLimits2().orElse(null);
            }
            case REL_CURRENT_LIMITS3 -> ((ThreeWindingsTransformer) obj).getLeg3().getCurrentLimits().orElse(null);
            case REL_SELECTED_OLG1 -> {
                if (obj instanceof ThreeWindingsTransformer twt)
                    yield twt.getLeg1().getSelectedOperationalLimitsGroup().orElse(null);
                yield ((Branch<?>) obj).getSelectedOperationalLimitsGroup1().orElse(null);
            }
            case REL_SELECTED_OLG2 -> {
                if (obj instanceof ThreeWindingsTransformer twt)
                    yield twt.getLeg2().getSelectedOperationalLimitsGroup().orElse(null);
                yield ((Branch<?>) obj).getSelectedOperationalLimitsGroup2().orElse(null);
            }
            case REL_SELECTED_OLG3 -> ((ThreeWindingsTransformer) obj).getLeg3().getSelectedOperationalLimitsGroup().orElse(null);
            case REL_OLG_CURRENT_LIMITS -> ((OperationalLimitsGroup) obj).getCurrentLimits().orElse(null);
            case REL_CONNECTABLE -> ((Terminal) obj).getConnectable();
            case VOLTAGE_REGULATION -> ((VoltageRegulationHolder<?>) obj).getVoltageRegulation();
            case ACTIVE_POWER_CONTROL -> ((Extendable<?>) obj).getExtension(ActivePowerControl.class);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown relation: " + relation);
        };
    }

    // ── Related object setter (navigation) ────────────────────────────────

    /** {@code valueHandle} 0 (INVALID_HANDLE) stands for null. */
    public static void setRelated(long handle, int relation, long valueHandle) {
        Object value = valueHandle == 0L ? null : NetworkRegistry.lookup(valueHandle);
        setRelatedObject(NetworkRegistry.lookup(handle), relation, value);
    }

    public static void setRelatedObject(Object obj, int relation, Object value) {
        if (obj instanceof Creation.PendingAdd adder) {
            Creation.setRelated(adder, relation, value);
            return;
        }
        switch (relation) {
            case REL_SLACK_TERMINAL -> slackTerminal(obj).setTerminal((Terminal) value);
            // Only clearing (INVALID_HANDLE): new ones are created with invoke
            case VOLTAGE_REGULATION -> Creation.clear(obj, relation, value);
            case ACTIVE_POWER_CONTROL -> Creation.clear(obj, relation, value);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown relation for set: " + relation);
        }
    }

    // ── Creation and removal ──────────────────────────────────────────────

    public static long invoke(long handle, int code) {
        return registerOrZero(invokeObject(NetworkRegistry.lookup(handle), code));
    }

    /**
     * {@code code} is a type code: starts a new object of that type and returns its adder (the network
     * does not change); {@link PropertyCodes#ACT_ADD} on an adder: adds the object and returns it;
     * {@link PropertyCodes#ACT_REMOVE} on an identifiable: removes it (returns null). Also starts
     * changes that take several values ({@code invoke(regulation, REL_TERMINAL)}). See {@link Creation}.
     * {@link PropertyCodes#ACT_CONNECT} / {@link PropertyCodes#ACT_DISCONNECT} on a terminal or a
     * connectable: returns it when this changed it, else null.
     */
    public static Object invokeObject(Object obj, int code) {
        return switch (code) {
            case ACT_CONNECT -> (obj instanceof Terminal t ? t.connect() : ((Connectable<?>) obj).connect()) ? obj : null;
            case ACT_DISCONNECT -> (obj instanceof Terminal t ? t.disconnect() : ((Connectable<?>) obj).disconnect()) ? obj : null;
            case ACT_ADD -> Creation.add(obj);
            case ACT_REMOVE -> {
                Creation.remove(obj);
                yield null;
            }
            default -> Creation.newAdder(obj, code);
        };
    }

    private static long registerOrZero(Object obj) {
        return obj == null ? 0L : NetworkRegistry.register(obj); // 0 = INVALID_HANDLE
    }

    // ── Int list (navigation) ─────────────────────────────────────────────

    public static int[] getIntList(long handle, int listCode) {
        return getIntList(NetworkRegistry.lookup(handle), listCode);
    }

    public static int[] getIntList(Object obj, int listCode) {
        return switch (listCode) {
            case VL_NBV_NODES -> ((VoltageLevel) obj).getNodeBreakerView().getNodes();
            case OBS_AREA_NODES -> StateEstimationDispatch.observabilityAreaNodes(obj);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown int list code: " + listCode);
        };
    }

    // ── Related by string key (navigation) ───────────────────────────────────

    public static long getRelatedByString(long handle, int relation, String key) {
        return registerOrZero(getRelatedObjectByString(NetworkRegistry.lookup(handle), relation, key));
    }

    public static Object getRelatedObjectByString(Object obj, int relation, String key) {
        return switch (relation) {
            case REL_TERMINAL -> {
                if (obj instanceof Branch<?> branch) yield branch.getTerminal(key);
                yield ((ThreeWindingsTransformer) obj).getTerminal(key);
            }
            case REL_BBV_BUS -> ((VoltageLevel) obj).getBusBreakerView().getBus(key);
            case REL_VL_BBV_BUS1 -> ((VoltageLevel) obj).getBusBreakerView().getBus1(key);
            case REL_VL_BBV_BUS2 -> ((VoltageLevel) obj).getBusBreakerView().getBus2(key);
            case REL_MEASUREMENT_BY_ID          -> StateEstimationDispatch.measurementById(obj, key);
            case REL_DISCRETE_MEASUREMENT_BY_ID -> StateEstimationDispatch.discreteMeasurementById(obj, key);
            case REL_OBS_AREA_BY_BUS, REL_OBS_AREA_BY_BBV_BUS -> StateEstimationDispatch.observabilityAreaByBus(obj, relation, key);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown keyed relation: " + relation);
        };
    }

    // ── Related by index (navigation) ─────────────────────────────────────

    public static long getRelatedByIndex(long handle, int relation, int index) {
        return registerOrZero(getRelatedObjectByIndex(NetworkRegistry.lookup(handle), relation, index));
    }

    public static Object getRelatedObjectByIndex(Object obj, int relation, int index) {
        return switch (relation) {
            case REL_NBV_TERMINAL_AT_NODE -> ((VoltageLevel) obj).getNodeBreakerView().getTerminal(index);
            case REL_OBS_AREA_BY_NODE -> StateEstimationDispatch.observabilityAreaByNode(obj, index);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown indexed relation: " + relation);
        };
    }

    // ── Find by ID ────────────────────────────────────────────────────────

    public static long findById(long networkHandle, int objectType, String id) {
        return registerOrZero(findObjectById(NetworkRegistry.lookup(networkHandle), objectType, id));
    }

    public static Object findObjectById(Object netObj, int objectType, String id) {
        Network network = (Network) netObj;
        return switch (objectType) {
            case GENERATOR                  -> network.getGenerator(id);
            case LOAD                       -> network.getLoad(id);
            case LINE                       -> network.getLine(id);
            case TWO_WINDINGS_TRANSFORMER   -> network.getTwoWindingsTransformer(id);
            case THREE_WINDINGS_TRANSFORMER -> network.getThreeWindingsTransformer(id);
            case HVDC_LINE                  -> network.getHvdcLine(id);
            case DANGLING_LINE              -> network.getBoundaryLine(id);
            case SHUNT_COMPENSATOR          -> network.getShuntCompensator(id);
            case STATIC_VAR_COMPENSATOR     -> network.getStaticVarCompensator(id);
            case VSC_CONVERTER_STATION      -> network.getVscConverterStation(id);
            case LCC_CONVERTER_STATION      -> network.getLccConverterStation(id);
            case SWITCH                     -> network.getSwitch(id);
            case BUSBAR_SECTION             -> network.getBusbarSection(id);
            case BATTERY                    -> network.getBattery(id);
            case VOLTAGE_LEVEL              -> network.getVoltageLevel(id);
            case SUBSTATION                 -> network.getSubstation(id);
            case IDENTIFIABLE               -> network.getIdentifiable(id);
            case CONNECTABLE                -> network.getConnectable(id);
            case INJECTION                  -> network.getConnectable(id) instanceof Injection<?> injection ? injection : null;
            case BRANCH                     -> network.getBranch(id);
            case HVDC_CONVERTER_STATION     -> network.getHvdcConverterStation(id);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown object type: " + objectType);
        };
    }

    // ── ThreeWindingsTransformer legs: THREE_WT_LEGn_BASE + THREE_WT_LEG_*_OFF ──

    static boolean isThreeWtLegCode(int property) {
        return property >= THREE_WT_LEG1_BASE && property < THREE_WT_LEG3_BASE + THREE_WT_LEG_BLOCK_SIZE;
    }

    private static ThreeWindingsTransformer.Leg threeWtLeg(Object obj, int property) {
        ThreeWindingsTransformer twt = (ThreeWindingsTransformer) obj;
        return switch ((property - THREE_WT_LEG1_BASE) / THREE_WT_LEG_BLOCK_SIZE) {
            case 0 -> twt.getLeg1();
            case 1 -> twt.getLeg2();
            default -> twt.getLeg3();
        };
    }

    private static int threeWtLegOffset(int property) {
        return (property - THREE_WT_LEG1_BASE) % THREE_WT_LEG_BLOCK_SIZE;
    }

    private static BridgeErrors.PropertyNotFoundException unknownLegProperty(String type, int property) {
        return new BridgeErrors.PropertyNotFoundException("Unknown " + type + " property: " + property);
    }

    static double threeWtLegDouble(Object obj, int property) {
        ThreeWindingsTransformer.Leg leg = threeWtLeg(obj, property);
        return switch (threeWtLegOffset(property)) {
            case THREE_WT_LEG_R_OFF             -> leg.getR();
            case THREE_WT_LEG_X_OFF             -> leg.getX();
            case THREE_WT_LEG_G_OFF             -> leg.getG();
            case THREE_WT_LEG_B_OFF             -> leg.getB();
            case THREE_WT_LEG_RATED_U_OFF       -> leg.getRatedU();
            case THREE_WT_LEG_RATED_S_OFF       -> leg.getRatedS();
            case THREE_WT_LEG_RTC_TARGET_V_OFF  -> leg.getRatioTapChanger().getTargetV();
            case THREE_WT_LEG_PTC_REG_VALUE_OFF -> leg.getPhaseTapChanger().getRegulationValue();
            default -> throw unknownLegProperty("double", property);
        };
    }

    static void setThreeWtLegDouble(Object obj, int property, double value) {
        ThreeWindingsTransformer.Leg leg = threeWtLeg(obj, property);
        switch (threeWtLegOffset(property)) {
            case THREE_WT_LEG_RTC_TARGET_V_OFF  -> leg.getRatioTapChanger().setTargetV(value);
            case THREE_WT_LEG_PTC_REG_VALUE_OFF -> leg.getPhaseTapChanger().setRegulationValue(value);
            default -> throw unknownLegProperty("double", property);
        }
    }

    static int threeWtLegInt(Object obj, int property) {
        ThreeWindingsTransformer.Leg leg = threeWtLeg(obj, property);
        return switch (threeWtLegOffset(property)) {
            case THREE_WT_LEG_RTC_TAP_POS_OFF   -> leg.getRatioTapChanger().getTapPosition();
            case THREE_WT_LEG_RTC_LOW_TAP_OFF   -> leg.getRatioTapChanger().getLowTapPosition();
            case THREE_WT_LEG_RTC_HIGH_TAP_OFF  -> leg.getRatioTapChanger().getHighTapPosition();
            case THREE_WT_LEG_PTC_TAP_POS_OFF   -> leg.getPhaseTapChanger().getTapPosition();
            case THREE_WT_LEG_PTC_LOW_TAP_OFF   -> leg.getPhaseTapChanger().getLowTapPosition();
            case THREE_WT_LEG_PTC_HIGH_TAP_OFF  -> leg.getPhaseTapChanger().getHighTapPosition();
            case THREE_WT_LEG_PTC_REG_MODE_OFF  -> leg.getPhaseTapChanger().getRegulationMode().ordinal();
            default -> throw unknownLegProperty("int", property);
        };
    }

    static void setThreeWtLegInt(Object obj, int property, int value) {
        ThreeWindingsTransformer.Leg leg = threeWtLeg(obj, property);
        switch (threeWtLegOffset(property)) {
            case THREE_WT_LEG_RTC_TAP_POS_OFF  -> leg.getRatioTapChanger().setTapPosition(value);
            case THREE_WT_LEG_PTC_TAP_POS_OFF  -> leg.getPhaseTapChanger().setTapPosition(value);
            case THREE_WT_LEG_PTC_REG_MODE_OFF ->
                leg.getPhaseTapChanger().setRegulationMode(PhaseTapChanger.RegulationMode.values()[value]);
            default -> throw unknownLegProperty("int", property);
        }
    }

    static boolean threeWtLegBool(Object obj, int property) {
        ThreeWindingsTransformer.Leg leg = threeWtLeg(obj, property);
        return switch (threeWtLegOffset(property)) {
            case THREE_WT_LEG_RTC_EXISTS_OFF        -> leg.getRatioTapChanger() != null;
            case THREE_WT_LEG_RTC_REGULATING_OFF    -> leg.getRatioTapChanger().isRegulating();
            case THREE_WT_LEG_PTC_EXISTS_OFF        -> leg.getPhaseTapChanger() != null;
            case THREE_WT_LEG_PTC_REGULATING_OFF    -> leg.getPhaseTapChanger().isRegulating();
            case THREE_WT_LEG_RTC_LOAD_TAP_CAP_OFF  -> leg.getRatioTapChanger().hasLoadTapChangingCapabilities();
            default -> throw unknownLegProperty("bool", property);
        };
    }

    static void setThreeWtLegBool(Object obj, int property, boolean value) {
        ThreeWindingsTransformer.Leg leg = threeWtLeg(obj, property);
        switch (threeWtLegOffset(property)) {
            case THREE_WT_LEG_RTC_REGULATING_OFF -> leg.getRatioTapChanger().setRegulating(value);
            case THREE_WT_LEG_PTC_REGULATING_OFF -> leg.getPhaseTapChanger().setRegulating(value);
            default -> throw unknownLegProperty("bool", property);
        }
    }

    // ── Extension helpers ─────────────────────────────────────────────────

    /** Extension of any extendable object; ExtensionNotFoundException when absent. */
    static <E extends com.powsybl.commons.extensions.Extension<?>> E ext(Object obj, Class<? super E> type) {
        return StateEstimationDispatch.extension(obj, type);
    }

    /** The ActivePowerControl extension object (from getRelated(injection, ACTIVE_POWER_CONTROL)). */
    static ActivePowerControl<?> activePowerControl(Object obj) {
        return (ActivePowerControl<?>) obj;
    }

    static SlackTerminal slackTerminal(Object obj) {
        return StateEstimationDispatch.extension(obj, SlackTerminal.class);
    }

    // ── StaticVarCompensator regulation helpers ───────────────────────────
    //
    // iidm4cpp's StaticVarCompensatorRegulationMode is
    //   {VOLTAGE = 0, REACTIVE_POWER = 1, OFF = 2, VOLTAGE_PER_REACTIVE_POWER = 3}.
    // powsybl-core (>= 7.4) carries the mode on the core VoltageRegulation object and
    // the "off" state through isRegulating(); OFF is reported when there is no
    // regulation or when it is not regulating.

    static final int SVC_MODE_OFF = 2;
    static final int SVC_MODE_VOLTAGE_PER_REACTIVE_POWER = 3;

    static int svcRegulationModeToCode(StaticVarCompensator svc) {
        VoltageRegulation vr = svc.getVoltageRegulation();
        if (vr == null || !vr.isRegulating() || vr.getMode() == null) {
            return SVC_MODE_OFF;
        }
        return switch (vr.getMode()) {
            case VOLTAGE -> 0;
            case REACTIVE_POWER -> 1;
            case VOLTAGE_PER_REACTIVE_POWER -> SVC_MODE_VOLTAGE_PER_REACTIVE_POWER;
        };
    }

    static void setSvcRegulationModeFromCode(StaticVarCompensator svc, int code) {
        RegulationMode mode = switch (code) {
            case 0 -> RegulationMode.VOLTAGE;
            case 1 -> RegulationMode.REACTIVE_POWER;
            case SVC_MODE_OFF -> null;
            case SVC_MODE_VOLTAGE_PER_REACTIVE_POWER -> RegulationMode.VOLTAGE_PER_REACTIVE_POWER;
            default -> throw new IllegalArgumentException("Unknown SVC regulation mode code: " + code);
        };
        if (mode == null) {
            svc.setRegulating(false);
        } else {
            svc.setRegulationMode(mode);
            svc.setRegulating(true);
        }
    }

    // The VoltagePerReactivePowerControl extension was removed in powsybl-core 7.4: the slope now
    // lives on the core VoltageRegulation, in VOLTAGE_PER_REACTIVE_POWER mode. iidm4cpp keeps its
    // VoltagePerReactivePowerControl wrapper and maps it onto that regulation.
    static boolean isVoltagePerReactivePowerControl(Object obj) {
        VoltageRegulation vr = ((StaticVarCompensator) obj).getVoltageRegulation();
        return vr != null && vr.getMode() == RegulationMode.VOLTAGE_PER_REACTIVE_POWER;
    }

    static VoltageRegulation vprcRegulation(Object obj) {
        if (!isVoltagePerReactivePowerControl(obj)) {
            throw new BridgeErrors.ExtensionNotFoundException("VoltagePerReactivePowerControl not present on " + ((StaticVarCompensator) obj).getId());
        }
        return ((StaticVarCompensator) obj).getVoltageRegulation();
    }
}
