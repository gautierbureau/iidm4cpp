package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.LoadingLimits.TemporaryLimit;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.commons.extensions.Extendable;
import com.powsybl.iidm.network.regulation.RegulationMode;
import com.powsybl.iidm.network.regulation.VoltageRegulation;
import com.powsybl.iidm.network.regulation.VoltageRegulationHolder;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import java.util.Arrays;
import java.util.Collection;
import java.util.Comparator;
import java.util.Objects;
import java.util.OptionalInt;

import java.util.stream.IntStream;
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
            case ZIP_C0P -> ((ZipLoadModel) obj).getC0p();
            case ZIP_C1P -> ((ZipLoadModel) obj).getC1p();
            case ZIP_C2P -> ((ZipLoadModel) obj).getC2p();
            case ZIP_C0Q -> ((ZipLoadModel) obj).getC0q();
            case ZIP_C1Q -> ((ZipLoadModel) obj).getC1q();
            case ZIP_C2Q -> ((ZipLoadModel) obj).getC2q();
            case EXP_NP  -> ((ExponentialLoadModel) obj).getNp();
            case EXP_NQ  -> ((ExponentialLoadModel) obj).getNq();
            case TERMINAL_P    -> ((Terminal) obj).getP();
            case TERMINAL_Q    -> ((Terminal) obj).getQ();
            case TERMINAL_I    -> ((Terminal) obj).getI();
            case BUS_V         -> ((Bus) obj).getV();
            case BUS_ANGLE     -> ((Bus) obj).getAngle();
            case BUS_P         -> ((Bus) obj).getP();
            case BUS_Q         -> ((Bus) obj).getQ();
            case FICTITIOUS_P0 -> ((Bus) obj).getFictitiousP0();
            case FICTITIOUS_Q0 -> ((Bus) obj).getFictitiousQ0();
            case VL_NOMINAL_V  -> ((VoltageLevel) obj).getNominalV();
            case VL_LOW_VOLTAGE_LIMIT  -> ((VoltageLevel) obj).getLowVoltageLimit();
            case VL_HIGH_VOLTAGE_LIMIT -> ((VoltageLevel) obj).getHighVoltageLimit();
            case LINE_R  -> ((LineCharacteristics) obj).getR();
            case LINE_X  -> ((LineCharacteristics) obj).getX();
            case LINE_G1 -> ((LineCharacteristics) obj).getG1();
            case LINE_B1 -> ((LineCharacteristics) obj).getB1();
            case LINE_G2 -> ((LineCharacteristics) obj).getG2();
            case LINE_B2 -> ((LineCharacteristics) obj).getB2();
            case TWO_WT_R        -> ((TwoWindingsTransformer) obj).getR();
            case TWO_WT_X        -> ((TwoWindingsTransformer) obj).getX();
            case TWO_WT_G        -> ((TwoWindingsTransformer) obj).getG();
            case TWO_WT_B        -> ((TwoWindingsTransformer) obj).getB();
            case TWO_WT_RATED_U1 -> ((TwoWindingsTransformer) obj).getRatedU1();
            case TWO_WT_RATED_U2 -> ((TwoWindingsTransformer) obj).getRatedU2();
            case TWO_WT_RATED_S  -> ((TwoWindingsTransformer) obj).getRatedS();
            case LEG_R       -> ((ThreeWindingsTransformer.Leg) obj).getR();
            case LEG_X       -> ((ThreeWindingsTransformer.Leg) obj).getX();
            case LEG_G       -> ((ThreeWindingsTransformer.Leg) obj).getG();
            case LEG_B       -> ((ThreeWindingsTransformer.Leg) obj).getB();
            case LEG_RATED_U -> ((ThreeWindingsTransformer.Leg) obj).getRatedU();
            case LEG_RATED_S -> ((ThreeWindingsTransformer.Leg) obj).getRatedS();
            case PTC_REGULATION_VALUE -> ((PhaseTapChanger) obj).getRegulationValue();
            case PTC_TARGET_DEADBAND  -> ((PhaseTapChanger) obj).getTargetDeadband();
            case STEP_RHO       -> ((TapChangerStep<?>) obj).getRho();
            case STEP_R         -> ((TapChangerStep<?>) obj).getR();
            case STEP_X         -> ((TapChangerStep<?>) obj).getX();
            case STEP_G         -> ((TapChangerStep<?>) obj).getG();
            case STEP_B         -> ((TapChangerStep<?>) obj).getB();
            case PTC_STEP_ALPHA -> ((PhaseTapChangerStep) obj).getAlpha();
            case HVDC_R                     -> ((HvdcLine) obj).getR();
            case HVDC_NOMINAL_V             -> ((HvdcLine) obj).getNominalV();
            case HVDC_ACTIVE_POWER_SETPOINT -> ((HvdcLine) obj).getActivePowerSetpoint();
            case HVDC_MAX_P                 -> ((HvdcLine) obj).getMaxP();
            case BL_P0 -> ((BoundaryLine) obj).getP0();
            case BL_Q0 -> ((BoundaryLine) obj).getQ0();
            case BL_R  -> ((BoundaryLine) obj).getR();
            case BL_X  -> ((BoundaryLine) obj).getX();
            case BL_G  -> ((BoundaryLine) obj).getG();
            case BL_B  -> ((BoundaryLine) obj).getB();
            case GENERATION_TARGET_P -> ((BoundaryLine.Generation) obj).getTargetP();
            case GENERATION_MIN_P    -> ((BoundaryLine.Generation) obj).getMinP();
            case GENERATION_MAX_P    -> ((BoundaryLine.Generation) obj).getMaxP();
            case GENERATION_TARGET_Q -> ((BoundaryLine.Generation) obj).getTargetQ();
            case GENERATION_TARGET_V -> ((BoundaryLine.Generation) obj).getTargetV();
            case BOUNDARY_V     -> ((Boundary) obj).getV();
            case BOUNDARY_ANGLE -> ((Boundary) obj).getAngle();
            case BOUNDARY_P     -> ((Boundary) obj).getP();
            case BOUNDARY_Q     -> ((Boundary) obj).getQ();
            case BOUNDARY_I     -> ((Boundary) obj).getI();
            case SHUNT_B -> ((ShuntCompensator) obj).getB();
            case SHUNT_G -> ((ShuntCompensator) obj).getG();
            case SHUNT_B_PER_SECTION -> ((ShuntCompensatorLinearModel) obj).getBPerSection();
            case SHUNT_G_PER_SECTION -> ((ShuntCompensatorLinearModel) obj).getGPerSection();
            case SHUNT_SECTION_B -> ((ShuntCompensatorNonLinearModel.Section) obj).getB();
            case SHUNT_SECTION_G -> ((ShuntCompensatorNonLinearModel.Section) obj).getG();
            case SVC_B_MIN                   -> ((StaticVarCompensator) obj).getBmin();
            case SVC_B_MAX                   -> ((StaticVarCompensator) obj).getBmax();
            case CONVERTER_STATION_LOSS_FACTOR -> ((HvdcConverterStation<?>) obj).getLossFactor();
            case LCC_POWER_FACTOR -> ((LccConverterStation) obj).getPowerFactor();
            case EXT_APC_DROOP -> activePowerControl(obj).getDroop();
            case EXT_APC_PARTICIPATION_FACTOR -> activePowerControl(obj).getParticipationFactor();
            case EXT_APC_MIN_TARGET_P -> activePowerControl(obj).getMinTargetP().orElse(Double.NaN);
            case EXT_APC_MAX_TARGET_P -> activePowerControl(obj).getMaxTargetP().orElse(Double.NaN);
            case EXT_CRC_Q_PERCENT -> ((CoordinatedReactiveControl) obj).getQPercent();
            case EXT_HADAPC_DROOP -> ((HvdcAngleDroopActivePowerControl) obj).getDroop();
            case EXT_HADAPC_P0 -> ((HvdcAngleDroopActivePowerControl) obj).getP0();
            case EXT_HOAR_OPR_CS1_TO_CS2 -> ((HvdcOperatorActivePowerRange) obj).getOprFromCS1toCS2();
            case EXT_HOAR_OPR_CS2_TO_CS1 -> ((HvdcOperatorActivePowerRange) obj).getOprFromCS2toCS1();
            case BBS_V     -> ((BusbarSection) obj).getV();
            case BBS_ANGLE -> ((BusbarSection) obj).getAngle();
            // Battery
            case BAT_TARGET_P -> ((Battery) obj).getTargetP();
            case BAT_MIN_P    -> ((Battery) obj).getMinP();
            case BAT_MAX_P    -> ((Battery) obj).getMaxP();
            // Generator reactive limits (min-max kind)
            // Reactive limits
            case RL_MIN_Q    -> ((MinMaxReactiveLimits) obj).getMinQ();
            case RL_MAX_Q    -> ((MinMaxReactiveLimits) obj).getMaxQ();
            case CURVE_MIN_P -> ((ReactiveCapabilityCurve) obj).getMinP();
            case CURVE_MAX_P -> ((ReactiveCapabilityCurve) obj).getMaxP();
            case PLANE_ALPHA -> ((ReactiveCapabilityShapePlane) obj).getAlpha();
            case PLANE_BETA  -> ((ReactiveCapabilityShapePlane) obj).getBeta();
            case PLANE_GAMMA -> ((ReactiveCapabilityShapePlane) obj).getGamma();
            case PLANE_DELTA -> ((ReactiveCapabilityShapePlane) obj).getDelta();
            case POINT_P     -> ((ReactiveCapabilityCurve.Point) obj).getP();
            case POINT_MIN_Q -> ((ReactiveCapabilityCurve.Point) obj).getMinQ();
            case POINT_MAX_Q -> ((ReactiveCapabilityCurve.Point) obj).getMaxQ();
            // CurrentLimits
            case LL_PERMANENT_LIMIT -> ((LoadingLimits) obj).getPermanentLimit();
            // TemporaryLimit
            case TL_VALUE -> ((TemporaryLimit) obj).getValue();
            // Areas, voltage angle limits
            case AREA_INTERCHANGE_TARGET -> ((Area) obj).getInterchangeTarget().orElse(Double.NaN);
            case AREA_AC_INTERCHANGE     -> ((Area) obj).getAcInterchange();
            case AREA_DC_INTERCHANGE     -> ((Area) obj).getDcInterchange();
            case AREA_INTERCHANGE        -> ((Area) obj).getInterchange();
            case AREA_BOUNDARY_P         -> ((AreaBoundary) obj).getP();
            case AREA_BOUNDARY_Q         -> ((AreaBoundary) obj).getQ();
            case VAL_LOW_LIMIT  -> ((VoltageAngleLimit) obj).getLowLimit().orElse(Double.NaN);
            case VAL_HIGH_LIMIT -> ((VoltageAngleLimit) obj).getHighLimit().orElse(Double.NaN);
            case TRIPPING_CURRENT_LIMIT -> ((OverloadManagementSystem.Tripping) obj).getCurrentLimit();
            // Overload
            case OVERLOAD_PREVIOUS_LIMIT -> ((Overload) obj).getPreviousLimit();
            case OVERLOAD_LIMIT_SCALING  -> ((Overload) obj).getLimitScalingCoefficient();
            // ThreeWT
            case THREE_WT_RATED_U0 -> ((ThreeWindingsTransformer) obj).getRatedU0();
            // State estimation extensions
            case OBS_QUALITY_STANDARD_DEVIATION -> ((ObservabilityQuality<?>) obj).getStandardDeviation();
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
            case ZIP_C0P -> ((ZipLoadModel) obj).setC0p(value);
            case ZIP_C1P -> ((ZipLoadModel) obj).setC1p(value);
            case ZIP_C2P -> ((ZipLoadModel) obj).setC2p(value);
            case ZIP_C0Q -> ((ZipLoadModel) obj).setC0q(value);
            case ZIP_C1Q -> ((ZipLoadModel) obj).setC1q(value);
            case ZIP_C2Q -> ((ZipLoadModel) obj).setC2q(value);
            case EXP_NP  -> ((ExponentialLoadModel) obj).setNp(value);
            case EXP_NQ  -> ((ExponentialLoadModel) obj).setNq(value);
            case TERMINAL_P    -> ((Terminal) obj).setP(value);
            case TERMINAL_Q    -> ((Terminal) obj).setQ(value);
            case BUS_V         -> ((Bus) obj).setV(value);
            case BUS_ANGLE     -> ((Bus) obj).setAngle(value);
            case FICTITIOUS_P0 -> ((Bus) obj).setFictitiousP0(value);
            case AREA_INTERCHANGE_TARGET -> ((Area) obj).setInterchangeTarget(value);
            case TRIPPING_CURRENT_LIMIT -> ((OverloadManagementSystem.Tripping) obj).setCurrentLimit(value);
            case FICTITIOUS_Q0 -> ((Bus) obj).setFictitiousQ0(value);
            case VL_NOMINAL_V          -> ((VoltageLevel) obj).setNominalV(value);
            case VL_LOW_VOLTAGE_LIMIT  -> ((VoltageLevel) obj).setLowVoltageLimit(value);
            case VL_HIGH_VOLTAGE_LIMIT -> ((VoltageLevel) obj).setHighVoltageLimit(value);
            case HVDC_R                     -> ((HvdcLine) obj).setR(value);
            case HVDC_NOMINAL_V             -> ((HvdcLine) obj).setNominalV(value);
            case HVDC_ACTIVE_POWER_SETPOINT -> ((HvdcLine) obj).setActivePowerSetpoint(value);
            case HVDC_MAX_P                 -> ((HvdcLine) obj).setMaxP(value);
            case LINE_R  -> ((Line) obj).setR(value);
            case LINE_X  -> ((Line) obj).setX(value);
            case LINE_G1 -> ((Line) obj).setG1(value);
            case LINE_B1 -> ((Line) obj).setB1(value);
            case LINE_G2 -> ((Line) obj).setG2(value);
            case LINE_B2 -> ((Line) obj).setB2(value);
            case TWO_WT_R        -> ((TwoWindingsTransformer) obj).setR(value);
            case TWO_WT_X        -> ((TwoWindingsTransformer) obj).setX(value);
            case TWO_WT_G        -> ((TwoWindingsTransformer) obj).setG(value);
            case TWO_WT_B        -> ((TwoWindingsTransformer) obj).setB(value);
            case TWO_WT_RATED_U1 -> ((TwoWindingsTransformer) obj).setRatedU1(value);
            case TWO_WT_RATED_U2 -> ((TwoWindingsTransformer) obj).setRatedU2(value);
            case TWO_WT_RATED_S  -> ((TwoWindingsTransformer) obj).setRatedS(value);
            case THREE_WT_RATED_U0 -> ((ThreeWindingsTransformer) obj).setRatedU0(value);
            case BL_P0 -> ((BoundaryLine) obj).setP0(value);
            case BL_Q0 -> ((BoundaryLine) obj).setQ0(value);
            case BL_R  -> ((BoundaryLine) obj).setR(value);
            case BL_X  -> ((BoundaryLine) obj).setX(value);
            case BL_G  -> ((BoundaryLine) obj).setG(value);
            case BL_B  -> ((BoundaryLine) obj).setB(value);
            case GENERATION_TARGET_P -> ((BoundaryLine.Generation) obj).setTargetP(value);
            case GENERATION_MIN_P    -> ((BoundaryLine.Generation) obj).setMinP(value);
            case GENERATION_MAX_P    -> ((BoundaryLine.Generation) obj).setMaxP(value);
            case GENERATION_TARGET_Q -> ((BoundaryLine.Generation) obj).setTargetQ(value);
            case GENERATION_TARGET_V -> ((BoundaryLine.Generation) obj).setTargetV(value);
            case CONVERTER_STATION_LOSS_FACTOR -> ((HvdcConverterStation<?>) obj).setLossFactor((float) value);
            case SHUNT_B_PER_SECTION -> ((ShuntCompensatorLinearModel) obj).setBPerSection(value);
            case SHUNT_G_PER_SECTION -> ((ShuntCompensatorLinearModel) obj).setGPerSection(value);
            case SHUNT_SECTION_B -> ((ShuntCompensatorNonLinearModel.Section) obj).setB(value);
            case SHUNT_SECTION_G -> ((ShuntCompensatorNonLinearModel.Section) obj).setG(value);
            case SVC_B_MIN -> ((StaticVarCompensator) obj).setBmin(value);
            case SVC_B_MAX -> ((StaticVarCompensator) obj).setBmax(value);
            case EXT_APC_DROOP -> activePowerControl(obj).setDroop(value);
            case EXT_APC_PARTICIPATION_FACTOR -> activePowerControl(obj).setParticipationFactor(value);
            case EXT_APC_MIN_TARGET_P -> activePowerControl(obj).setMinTargetP(value);
            case EXT_APC_MAX_TARGET_P -> activePowerControl(obj).setMaxTargetP(value);
            case EXT_CRC_Q_PERCENT -> ((CoordinatedReactiveControl) obj).setQPercent(value);
            case EXT_HADAPC_DROOP -> ((HvdcAngleDroopActivePowerControl) obj).setDroop((float) value);
            case EXT_HADAPC_P0 -> ((HvdcAngleDroopActivePowerControl) obj).setP0((float) value);
            case EXT_HOAR_OPR_CS1_TO_CS2 -> ((HvdcOperatorActivePowerRange) obj).setOprFromCS1toCS2((float) value);
            case EXT_HOAR_OPR_CS2_TO_CS1 -> ((HvdcOperatorActivePowerRange) obj).setOprFromCS2toCS1((float) value);
            case LEG_R       -> ((ThreeWindingsTransformer.Leg) obj).setR(value);
            case LEG_X       -> ((ThreeWindingsTransformer.Leg) obj).setX(value);
            case LEG_G       -> ((ThreeWindingsTransformer.Leg) obj).setG(value);
            case LEG_B       -> ((ThreeWindingsTransformer.Leg) obj).setB(value);
            case LEG_RATED_U -> ((ThreeWindingsTransformer.Leg) obj).setRatedU(value);
            case LEG_RATED_S -> ((ThreeWindingsTransformer.Leg) obj).setRatedS(value);
            case PTC_REGULATION_VALUE -> ((PhaseTapChanger) obj).setRegulationValue(value);
            case PTC_TARGET_DEADBAND  -> ((PhaseTapChanger) obj).setTargetDeadband(value);
            case STEP_RHO       -> ((TapChangerStep<?>) obj).setRho(value);
            case STEP_R         -> ((TapChangerStep<?>) obj).setR(value);
            case STEP_X         -> ((TapChangerStep<?>) obj).setX(value);
            case STEP_G         -> ((TapChangerStep<?>) obj).setG(value);
            case STEP_B         -> ((TapChangerStep<?>) obj).setB(value);
            case PTC_STEP_ALPHA -> ((PhaseTapChangerStep) obj).setAlpha(value);
            case BAT_TARGET_P -> ((Battery) obj).setTargetP(value);
            case BAT_MIN_P    -> ((Battery) obj).setMinP(value);
            case BAT_MAX_P    -> ((Battery) obj).setMaxP(value);
            case LCC_POWER_FACTOR -> ((LccConverterStation) obj).setPowerFactor((float) value);
            case LL_PERMANENT_LIMIT -> ((LoadingLimits) obj).setPermanentLimit(value);
            case OBS_QUALITY_STANDARD_DEVIATION -> ((ObservabilityQuality<?>) obj).setStandardDeviation(value);
            // Creates the quality if needed
            case OBS_QUALITY_P  -> ((InjectionObservability<?>) obj).setQualityP(value);
            case OBS_QUALITY_Q  -> ((InjectionObservability<?>) obj).setQualityQ(value);
            case OBS_QUALITY_V  -> ((InjectionObservability<?>) obj).setQualityV(value);
            case OBS_QUALITY_P1 -> ((BranchObservability<?>) obj).setQualityP1(value);
            case OBS_QUALITY_P2 -> ((BranchObservability<?>) obj).setQualityP2(value);
            case OBS_QUALITY_Q1 -> ((BranchObservability<?>) obj).setQualityQ1(value);
            case OBS_QUALITY_Q2 -> ((BranchObservability<?>) obj).setQualityQ2(value);
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
        return switch (property) {
            case IDENTIFIABLE_TYPE  -> ((Identifiable<?>) obj).getType().ordinal();
            case CONTAINER_TYPE     -> ((Container<?>) obj).getContainerType().ordinal();
            case NETWORK_VALIDATION_LEVEL -> ((Network) obj).getValidationLevel().ordinal();
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
            case LOAD_MODEL_TYPE    -> ((LoadModel) obj).getType().ordinal();
            case VL_TOPOLOGY_KIND   -> ((VoltageLevel) obj).getTopologyKind().ordinal();
            case VL_NBV_MAXIMUM_NODE_INDEX -> ((VoltageLevel) obj).getNodeBreakerView().getMaximumNodeIndex();
            case HVDC_CONVERTERS_MODE -> ((HvdcLine) obj).getConvertersMode().ordinal();
            case SHUNT_SECTION_COUNT     -> ((ShuntCompensator) obj).getSectionCount(); // fails when undefined
            case SHUNT_MAX_SECTION_COUNT -> ((ShuntCompensator) obj).getMaximumSectionCount();
            case SHUNT_MODEL_TYPE        -> ((ShuntCompensator) obj).getModelType().ordinal();
            case SW_KIND  -> ((Switch) obj).getKind().ordinal();
            case SW_NODE1 -> ((Switch) obj).getVoltageLevel().getNodeBreakerView().getNode1(((Switch) obj).getId());
            case SW_NODE2 -> ((Switch) obj).getVoltageLevel().getNodeBreakerView().getNode2(((Switch) obj).getId());
            case IC_NODE1 -> ((VoltageLevel.NodeBreakerView.InternalConnection) obj).getNode1();
            case IC_NODE2 -> ((VoltageLevel.NodeBreakerView.InternalConnection) obj).getNode2();
            case TL_ACCEPTABLE_DURATION -> ((TemporaryLimit) obj).getAcceptableDuration();
            case OMS_MONITORED_SIDE -> ((OverloadManagementSystem) obj).getMonitoredSide().ordinal();
            case COMPONENT_NUM  -> ((Component) obj).getNum();
            case COMPONENT_SIZE -> ((Component) obj).getSize();
            case TRIPPING_TYPE -> ((OverloadManagementSystem.Tripping) obj).getType().ordinal();
            case TRIPPING_SIDE -> obj instanceof OverloadManagementSystem.BranchTripping tripping
                ? tripping.getSideToOperate().ordinal()
                : ((OverloadManagementSystem.ThreeWindingsTransformerTripping) obj).getSideToOperate().ordinal();
            case OVERLOAD_DURATION -> overloadChecked(obj) instanceof ThreeWindingsTransformer twt
                ? twt.getOverloadDuration()
                : ((Branch<?>) obj).getOverloadDuration();
            case LL_DETECTION_KIND -> ((LoadingLimits) obj).getDetectionKind().ordinal();
            case LL_LIMIT_TYPE -> ((OperationalLimits) obj).getLimitType().ordinal();
            case TC_LOW_TAP_POSITION  -> tapChanger(obj).getLowTapPosition();
            case TC_HIGH_TAP_POSITION -> tapChanger(obj).getHighTapPosition();
            case TC_TAP_POSITION      -> tapChanger(obj).getTapPosition(); // fails when unset
            case TC_STEP_COUNT        -> tapChanger(obj).getStepCount();
            case PTC_REGULATION_MODE  -> ((PhaseTapChanger) obj).getRegulationMode().ordinal();
            case LEG_SIDE             -> ((ThreeWindingsTransformer.Leg) obj).getSide().ordinal();
            case RL_KIND           -> ((ReactiveLimits) obj).getKind().ordinal();
            case CURVE_POINT_COUNT -> ((ReactiveCapabilityCurve) obj).getPointCount();
            // Terminal node number (node-breaker topology)
            case TERMINAL_NODE -> ((Terminal) obj).getNodeBreakerView().getNode();
            // State estimation extensions
            case OBS_QUALITY_REDUNDANT -> ((ObservabilityQuality<?>) obj).isRedundant().map(r -> r ? 1 : 0).orElse(-1);
            case MEAS_TYPE          -> ((Measurement) obj).getType().ordinal();
            case MEAS_SIDE          -> StateEstimationDispatch.measurementSide((Measurement) obj);
            case DMEAS_TYPE         -> ((DiscreteMeasurement) obj).getType().ordinal();
            case DMEAS_TAP_CHANGER  -> StateEstimationDispatch.discreteMeasurementTapChanger((DiscreteMeasurement) obj);
            case DMEAS_VALUE_TYPE   -> ((DiscreteMeasurement) obj).getValueType().ordinal();
            case DMEAS_VALUE_INT    -> ((DiscreteMeasurement) obj).getValueAsInt();
            case OBS_AREA_NUMBER    -> ((StateEstimationDispatch.AreaRef) obj).area().getAreaNumber();
            case OBS_AREA_STATUS    -> ((StateEstimationDispatch.AreaRef) obj).area().getStatus().ordinal();
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
        switch (property) {
            case GEN_ENERGY_SOURCE -> ((Generator) obj).setEnergySource(Creation.enumValue(EnergySource.class, value));
            case LOAD_TYPE -> ((Load) obj).setLoadType(Creation.enumValue(LoadType.class, value));
            case NETWORK_FORECAST_DISTANCE -> ((Network) obj).setForecastDistance(value);
            case NETWORK_MIN_VALIDATION_LEVEL ->
                ((Network) obj).setMinimumAcceptableValidationLevel(Creation.enumValue(ValidationLevel.class, value));
            case VL_TOPOLOGY_KIND -> ((VoltageLevel) obj).convertToTopology(Creation.enumValue(TopologyKind.class, value));
            case VR_MODE -> ((VoltageRegulation) obj).setMode(value < 0 ? null : Creation.enumValue(RegulationMode.class, value));
            case HVDC_CONVERTERS_MODE ->
                ((HvdcLine) obj).setConvertersMode(HvdcLine.ConvertersMode.values()[value]);
            case SHUNT_SECTION_COUNT        -> ((ShuntCompensator) obj).setSectionCount(value);
            case SHUNT_SOLVED_SECTION_COUNT -> ((ShuntCompensator) obj).setSolvedSectionCount(value);
            case SHUNT_MAX_SECTION_COUNT    -> ((ShuntCompensatorLinearModel) obj).setMaximumSectionCount(value);
            case TC_LOW_TAP_POSITION    -> tapChanger(obj).setLowTapPosition(value);
            case TC_TAP_POSITION        -> tapChanger(obj).setTapPosition(value);
            case TC_SOLVED_TAP_POSITION -> tapChanger(obj).setSolvedTapPosition(value);
            case PTC_REGULATION_MODE ->
                ((PhaseTapChanger) obj).setRegulationMode(Creation.enumValue(PhaseTapChanger.RegulationMode.class, value));
            case TRIPPING_SIDE -> {
                if (obj instanceof OverloadManagementSystem.BranchTripping tripping) {
                    tripping.setSideToOperate(Creation.enumValue(TwoSides.class, value));
                } else {
                    ((OverloadManagementSystem.ThreeWindingsTransformerTripping) obj)
                        .setSideToOperate(Creation.enumValue(ThreeSides.class, value));
                }
            }
            case OBS_QUALITY_REDUNDANT -> ((ObservabilityQuality<?>) obj).setRedundant(value < 0 ? null : value != 0);
            case DMEAS_VALUE_INT -> ((DiscreteMeasurement) obj).setValue(value);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown int property for set: " + property);
        }
    }

    // ── Bool getters ──────────────────────────────────────────────────────

    public static int getBool(long handle, int property) {
        return getBool(NetworkRegistry.lookup(handle), property) ? 1 : 0;
    }

    public static boolean getBool(Object obj, int property) {
        return switch (property) {
            // Identifiable type codes: whether the object is one
            case IDENTIFIABLE               -> obj instanceof Identifiable<?>;
            case NETWORK                    -> obj instanceof Network;
            case CONNECTABLE                -> obj instanceof Connectable<?>;
            case INJECTION                  -> obj instanceof Injection<?>;
            case BRANCH                     -> obj instanceof Branch<?>;
            case HVDC_CONVERTER_STATION     -> obj instanceof HvdcConverterStation<?>;
            case SUBSTATION                 -> obj instanceof Substation;
            case VOLTAGE_LEVEL              -> obj instanceof VoltageLevel;
            case GENERATOR                  -> obj instanceof Generator;
            case LOAD                       -> obj instanceof Load;
            case LOAD_MODEL                 -> obj instanceof LoadModel;
            case ZIP_LOAD_MODEL             -> obj instanceof ZipLoadModel;
            case EXPONENTIAL_LOAD_MODEL     -> obj instanceof ExponentialLoadModel;
            case LINE                       -> obj instanceof Line;
            case TWO_WINDINGS_TRANSFORMER   -> obj instanceof TwoWindingsTransformer;
            case THREE_WINDINGS_TRANSFORMER -> obj instanceof ThreeWindingsTransformer;
            case HVDC_LINE                  -> obj instanceof HvdcLine;
            case BOUNDARY_LINE              -> obj instanceof BoundaryLine;
            case BOUNDARY_LINE_GENERATION   -> obj instanceof BoundaryLine.Generation;
            case BOUNDARY                   -> obj instanceof Boundary;
            case TIE_LINE                   -> obj instanceof TieLine;
            case AREA                       -> obj instanceof Area;
            case AREA_BOUNDARY              -> obj instanceof AreaBoundary;
            case AREA_BOUNDARY_AC           -> ((AreaBoundary) obj).isAc();
            case VOLTAGE_ANGLE_LIMIT        -> obj instanceof VoltageAngleLimit;
            case OVERLOAD_MANAGEMENT_SYSTEM -> obj instanceof OverloadManagementSystem;
            case COMPONENT                  -> obj instanceof Component;
            case VARIANT_MANAGER            -> obj instanceof VariantManager;
            case VARIANT_MULTI_THREAD_ACCESS -> ((VariantManager) obj).isVariantMultiThreadAccessAllowed();
            case NETWORK_DETACHABLE         -> ((Network) obj).isDetachable();
            case TRIPPING                   -> obj instanceof OverloadManagementSystem.Tripping;
            case SWITCH_TRIPPING            -> obj instanceof OverloadManagementSystem.SwitchTripping;
            case BRANCH_TRIPPING            -> obj instanceof OverloadManagementSystem.BranchTripping;
            case THREE_WINDINGS_TRANSFORMER_TRIPPING -> obj instanceof OverloadManagementSystem.ThreeWindingsTransformerTripping;
            case AUTOMATION_SYSTEM_ENABLED  -> ((AutomationSystem<?>) obj).isEnabled();
            case TRIPPING_OPEN_ACTION       -> ((OverloadManagementSystem.Tripping) obj).isOpenAction();
            case GROUND                     -> obj instanceof Ground;
            case BL_PAIRED                  -> ((BoundaryLine) obj).isPaired();
            case GENERATION_VOLTAGE_REGULATION_ON -> ((BoundaryLine.Generation) obj).isVoltageRegulationOn();
            case SHUNT_COMPENSATOR          -> obj instanceof ShuntCompensator;
            case SHUNT_COMPENSATOR_MODEL    -> obj instanceof ShuntCompensatorModel;
            case SHUNT_COMPENSATOR_LINEAR_MODEL -> obj instanceof ShuntCompensatorLinearModel;
            case SHUNT_COMPENSATOR_NON_LINEAR_MODEL -> obj instanceof ShuntCompensatorNonLinearModel;
            case SHUNT_SECTION              -> obj instanceof ShuntCompensatorNonLinearModel.Section;
            case STATIC_VAR_COMPENSATOR     -> obj instanceof StaticVarCompensator;
            case VSC_CONVERTER_STATION      -> obj instanceof VscConverterStation;
            case LCC_CONVERTER_STATION      -> obj instanceof LccConverterStation;
            case BUS                        -> obj instanceof Bus;
            case SWITCH                     -> obj instanceof Switch;
            case BUSBAR_SECTION             -> obj instanceof BusbarSection;
            case BATTERY                    -> obj instanceof Battery;
            case THREE_WINDINGS_TRANSFORMER_LEG -> obj instanceof ThreeWindingsTransformer.Leg;
            case RATIO_TAP_CHANGER          -> obj instanceof RatioTapChanger;
            case PHASE_TAP_CHANGER          -> obj instanceof PhaseTapChanger;
            case RATIO_TAP_CHANGER_STEP     -> obj instanceof RatioTapChangerStep;
            case PHASE_TAP_CHANGER_STEP     -> obj instanceof PhaseTapChangerStep;
            case REACTIVE_LIMITS            -> obj instanceof ReactiveLimits;
            case MIN_MAX_REACTIVE_LIMITS    -> obj instanceof MinMaxReactiveLimits;
            case REACTIVE_CAPABILITY_CURVE  -> obj instanceof ReactiveCapabilityCurve;
            case REACTIVE_CAPABILITY_SHAPE  -> obj instanceof ReactiveCapabilityShape;
            case REACTIVE_CURVE_POINT       -> obj instanceof ReactiveCapabilityCurve.Point;
            case REACTIVE_CAPABILITY_SHAPE_PLANE -> obj instanceof ReactiveCapabilityShapePlane;
            case LOADING_LIMITS             -> obj instanceof LoadingLimits;
            case CURRENT_LIMITS             -> obj instanceof CurrentLimits;
            case ACTIVE_POWER_LIMITS        -> obj instanceof ActivePowerLimits;
            case APPARENT_POWER_LIMITS      -> obj instanceof ApparentPowerLimits;
            case TEMPORARY_LIMIT            -> obj instanceof TemporaryLimit;
            case OPERATIONAL_LIMITS_GROUP   -> obj instanceof OperationalLimitsGroup;
            case OVERLOAD                   -> obj instanceof Overload;
            case OVERLOADED                 -> overloadChecked(obj) instanceof ThreeWindingsTransformer twt
                                               ? twt.isOverloaded()
                                               : ((Branch<?>) obj).isOverloaded();
            case FLOWS_LIMITS_HOLDER        -> obj instanceof FlowsLimitsHolder || obj instanceof LimitsDispatch.BranchSide;
            case ACTIVE_POWER_CONTROL       -> obj instanceof ActivePowerControl<?>;
            case COORDINATED_REACTIVE_CONTROL -> obj instanceof CoordinatedReactiveControl;
            case HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL -> obj instanceof HvdcAngleDroopActivePowerControl;
            case HVDC_OPERATOR_ACTIVE_POWER_RANGE -> obj instanceof HvdcOperatorActivePowerRange;
            case SLACK_TERMINAL             -> obj instanceof SlackTerminal;
            case TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED -> obj instanceof TwoWindingsTransformerToBeEstimated;
            case THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED -> obj instanceof ThreeWindingsTransformerToBeEstimated;
            case INJECTION_OBSERVABILITY    -> obj instanceof InjectionObservability<?>;
            case BRANCH_OBSERVABILITY       -> obj instanceof BranchObservability<?>;
            case MEASUREMENTS               -> obj instanceof Measurements<?>;
            case MEASUREMENT                -> obj instanceof Measurement;
            case DISCRETE_MEASUREMENTS      -> obj instanceof DiscreteMeasurements<?>;
            case DISCRETE_MEASUREMENT       -> obj instanceof DiscreteMeasurement;
            case OBSERVABILITY_AREA         -> obj instanceof ObservabilityArea;
            case AREA_CHARACTERISTICS       -> obj instanceof StateEstimationDispatch.AreaRef;
            case PLANE_LESS_OR_EQUAL        -> ((ReactiveCapabilityShapePlane) obj).isLessOrEqual();
            case PLANE_GREATER_OR_EQUAL     -> ((ReactiveCapabilityShapePlane) obj).isGreaterOrEqual();
            case FICTITIOUS                 -> ((Identifiable<?>) obj).isFictitious();
            case EQUIVALENT                 -> ((Connectable<?>) obj).isEquivalent();
            case GEN_CONDENSER              -> ((Generator) obj).isCondenser();
            case REGULATING                 -> obj instanceof TapChanger<?, ?, ?, ?> tapChanger
                                               ? tapChanger.isRegulating()
                                               : ((VoltageRegulationHolder<?>) obj).isRegulating();
            case TC_LOAD_TAP_CHANGING_CAPABILITIES -> tapChanger(obj).hasLoadTapChangingCapabilities();
            case HAS_REGULATING_TERMINAL    -> ((VoltageRegulationHolder<?>) obj).hasRegulatingTerminal();
            case VR_REGULATING              -> ((VoltageRegulation) obj).isRegulating();
            case VR_WITH_TERMINAL           -> ((VoltageRegulation) obj).isWithTerminal();
            case TERMINAL_CONNECTED         -> ((Terminal) obj).isConnected();
            case BUS_IN_MAIN_CONNECTED_COMPONENT   -> ((Bus) obj).isInMainConnectedComponent();
            case BUS_IN_MAIN_SYNCHRONOUS_COMPONENT -> ((Bus) obj).isInMainSynchronousComponent();
            case TL_FICTITIOUS -> ((TemporaryLimit) obj).isFictitious();
            case SW_OPEN     -> ((Switch) obj).isOpen();
            case SW_RETAINED -> ((Switch) obj).isRetained();
            case EXT_APC_PARTICIPATE -> activePowerControl(obj).isParticipate();
            case EXT_HADAPC_ENABLED  -> ((HvdcAngleDroopActivePowerControl) obj).isEnabled();
            case EXT_ST_EMPTY        -> ((SlackTerminal) obj).isEmpty();
            case EXT_TWO_WT_TBE_RTC  -> ((TwoWindingsTransformerToBeEstimated) obj).shouldEstimateRatioTapChanger();
            case EXT_TWO_WT_TBE_PTC  -> ((TwoWindingsTransformerToBeEstimated) obj).shouldEstimatePhaseTapChanger();
            case EXT_THREE_WT_TBE_RTC1 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimateRatioTapChanger1();
            case EXT_THREE_WT_TBE_RTC2 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimateRatioTapChanger2();
            case EXT_THREE_WT_TBE_RTC3 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimateRatioTapChanger3();
            case EXT_THREE_WT_TBE_PTC1 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimatePhaseTapChanger1();
            case EXT_THREE_WT_TBE_PTC2 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimatePhaseTapChanger2();
            case EXT_THREE_WT_TBE_PTC3 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimatePhaseTapChanger3();
            case EXT_OBS_OBSERVABLE  -> ((Observability<?>) obj).isObservable();
            case OLG_EMPTY -> ((OperationalLimitsGroup) obj).isEmpty();
            // State estimation extensions
            case MEAS_VALID       -> ((Measurement) obj).isValid();
            case DMEAS_VALUE_BOOL -> ((DiscreteMeasurement) obj).getValueAsBoolean();
            case DMEAS_VALID      -> ((DiscreteMeasurement) obj).isValid();
            case EXT_OBS_AREA_CONSISTENT -> ((ObservabilityArea) obj).isConsistentWithTopology();
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
        switch (property) {
            case FICTITIOUS -> ((Identifiable<?>) obj).setFictitious(bval);
            case EQUIVALENT -> ((Connectable<?>) obj).setEquivalent(bval);
            case VR_REGULATING -> ((VoltageRegulation) obj).setRegulating(bval);
            case SW_OPEN     -> ((Switch) obj).setOpen(bval);
            case SW_RETAINED -> ((Switch) obj).setRetained(bval);
            case REGULATING -> ((PhaseTapChanger) obj).setRegulating(bval);
            case TC_LOAD_TAP_CHANGING_CAPABILITIES -> tapChanger(obj).setLoadTapChangingCapabilities(bval);
            case GENERATION_VOLTAGE_REGULATION_ON -> ((BoundaryLine.Generation) obj).setVoltageRegulationOn(bval);
            case AUTOMATION_SYSTEM_ENABLED -> ((AutomationSystem<?>) obj).setEnabled(bval);
            case VARIANT_MULTI_THREAD_ACCESS -> ((VariantManager) obj).allowVariantMultiThreadAccess(bval);
            case TRIPPING_OPEN_ACTION -> ((OverloadManagementSystem.Tripping) obj).setOpenAction(bval);
            case EXT_APC_PARTICIPATE     -> activePowerControl(obj).setParticipate(bval);
            case EXT_HADAPC_ENABLED  -> ((HvdcAngleDroopActivePowerControl) obj).setEnabled(bval);
            case EXT_TWO_WT_TBE_RTC  -> ((TwoWindingsTransformerToBeEstimated) obj).shouldEstimateRatioTapChanger(bval);
            case EXT_TWO_WT_TBE_PTC  -> ((TwoWindingsTransformerToBeEstimated) obj).shouldEstimatePhaseTapChanger(bval);
            case EXT_THREE_WT_TBE_RTC1 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimateRatioTapChanger1(bval);
            case EXT_THREE_WT_TBE_RTC2 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimateRatioTapChanger2(bval);
            case EXT_THREE_WT_TBE_RTC3 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimateRatioTapChanger3(bval);
            case EXT_THREE_WT_TBE_PTC1 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimatePhaseTapChanger1(bval);
            case EXT_THREE_WT_TBE_PTC2 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimatePhaseTapChanger2(bval);
            case EXT_THREE_WT_TBE_PTC3 -> ((ThreeWindingsTransformerToBeEstimated) obj).shouldEstimatePhaseTapChanger3(bval);
            case EXT_OBS_OBSERVABLE  -> ((Observability<?>) obj).setObservable(bval);
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

    /** A value, or fails: a property that may have none is read with {@link #getOptString}. */
    public static String getString(Object obj, int property) {
        String value = switch (property) {
            case ID   -> ((Identifiable<?>) obj).getId();
            case NAME -> ((Identifiable<?>) obj).getNameOrId();
            case TL_NAME -> ((TemporaryLimit) obj).getName();
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
            case VL_TOPOLOGY_TEXT -> {
                java.io.ByteArrayOutputStream text = new java.io.ByteArrayOutputStream();
                ((VoltageLevel) obj).printTopology(new java.io.PrintStream(text, true, java.nio.charset.StandardCharsets.UTF_8), null);
                yield text.toString(java.nio.charset.StandardCharsets.UTF_8);
            }
            case OLG_ID -> ((OperationalLimitsGroup) obj).getId();
            case OVERLOAD_PREVIOUS_LIMIT_NAME -> ((Overload) obj).getPreviousLimitName();
            case AREA_TYPE -> ((Area) obj).getAreaType();
            case VAL_ID -> ((VoltageAngleLimit) obj).getId();
            case OMS_MONITORED_ELEMENT_ID -> ((OverloadManagementSystem) obj).getMonitoredElementId();
            case WORKING_VARIANT_ID -> ((VariantManager) obj).getWorkingVariantId();
            case TRIPPING_KEY -> ((OverloadManagementSystem.Tripping) obj).getKey();
            case TRIPPING_NAME -> ((OverloadManagementSystem.Tripping) obj).getNameOrKey();
            case TRIPPING_ELEMENT_ID -> switch (obj) {
                case OverloadManagementSystem.SwitchTripping tripping -> tripping.getSwitchToOperateId();
                case OverloadManagementSystem.BranchTripping tripping -> tripping.getBranchToOperateId();
                default -> ((OverloadManagementSystem.ThreeWindingsTransformerTripping) obj).getThreeWindingsTransformerToOperateId();
            };
            case OVERLOAD_OLG_ID -> ((Overload) obj).getOperationalLimitsGroupId();
            case LL_PERMANENT_LIMIT_NAME -> ((LoadingLimits) obj).getPermanentLimitName();
            case BL_PAIRING_KEY -> pairingKey(obj);
            case SELECTED_OLG_ID -> LimitsDispatch.holder(obj).getSelectedOperationalLimitsGroupId().orElse(null);
            case COUNTRY -> ((Substation) obj).getCountry().map(Country::name).orElse("");
            case MEAS_ID            -> ((Measurement) obj).getId();
            case DMEAS_ID           -> ((DiscreteMeasurement) obj).getId();
            case DMEAS_VALUE_STRING -> ((DiscreteMeasurement) obj).getValueAsString();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown string property: " + property);
        };
        if (value == null) {
            // Both backends fail the same way (JNI would read null as "", GraalVM would fail)
            throw new BridgeErrors.PropertyNotFoundException("No value for string property " + property);
        }
        return value;
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
            case ID      -> ((Identifiable<?>) obj).setId(value);
            case ADD_GEOGRAPHICAL_TAG -> ((Substation) obj).addGeographicalTag(value);
            case TRIPPING_NAME -> ((OverloadManagementSystem.Tripping) obj).setName(value);
            case WORKING_VARIANT_ID -> ((VariantManager) obj).setWorkingVariant(value);
            case REMOVE_VARIANT -> ((VariantManager) obj).removeVariant(value);
            case TRIPPING_ELEMENT_ID -> {
                switch (obj) {
                    case OverloadManagementSystem.SwitchTripping tripping -> tripping.setSwitchToOperateId(value);
                    case OverloadManagementSystem.BranchTripping tripping -> tripping.setBranchToOperateId(value);
                    default -> ((OverloadManagementSystem.ThreeWindingsTransformerTripping) obj).setThreeWindingsTransformerToOperateId(value);
                }
            }
            case TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID -> ((Terminal) obj).getBusBreakerView().setConnectableBus(value);
            case VL_NBV_REMOVE_SWITCH -> ((VoltageLevel) obj).getNodeBreakerView().removeSwitch(value);
            case VL_BBV_REMOVE_SWITCH -> ((VoltageLevel) obj).getBusBreakerView().removeSwitch(value);
            case BL_PAIRING_KEY -> ((BoundaryLine) obj).setPairingKey(value);
            case COUNTRY -> ((Substation) obj).setCountry(Country.valueOf(value));
            case SUBSTATION_TSO -> ((Substation) obj).setTso(value);
            case NETWORK_CASE_DATE -> ((Network) obj).setCaseDate(java.time.ZonedDateTime.parse(value));
            case DMEAS_VALUE_STRING -> ((DiscreteMeasurement) obj).setValue(value);
            case LL_PERMANENT_LIMIT_NAME -> ((LoadingLimits) obj).setPermanentLimitName(value);
            case SELECTED_OLG_ID  -> LimitsDispatch.holder(obj).setSelectedOperationalLimitsGroup(value);
            case ADD_SELECTED_OLG -> LimitsDispatch.holder(obj).addSelectedOperationalLimitsGroup(value);
            case DESELECT_OLG     -> LimitsDispatch.holder(obj).deselectOperationalLimitsGroup(value);
            case REMOVE_OLG       -> LimitsDispatch.holder(obj).removeOperationalLimitsGroup(value);
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
            case TC_TAP_POSITION            -> tapChanger(obj).findTapPosition();
            case TC_SOLVED_TAP_POSITION     -> tapChanger(obj).findSolvedTapPosition();
            case TC_NEUTRAL_POSITION        -> tapChanger(obj).getNeutralPosition();
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
            case MEAS_ID        -> ((Measurement) obj).getId();
            case DMEAS_ID       -> ((DiscreteMeasurement) obj).getId();
            case SELECTED_OLG_ID -> LimitsDispatch.holder(obj).getSelectedOperationalLimitsGroupId().orElse(null);
            case BL_PAIRING_KEY -> pairingKey(obj);
            default -> getString(obj, property);
        };
    }

    // ── String lists ──────────────────────────────────────────────────────

    public static String[] getStringList(long handle, int listCode) {
        return getStringList(NetworkRegistry.lookup(handle), listCode);
    }

    /** Sets are sorted, so that both backends list them in the same order; lists keep theirs. */
    public static String[] getStringList(Object obj, int listCode) {
        if (listCode == ALL_SELECTED_OLG_IDS_ORDERED) {
            return LimitsDispatch.holder(obj).getAllSelectedOperationalLimitsGroupIdsOrdered().toArray(String[]::new);
        }
        Collection<String> values = switch (listCode) {
            case PROPERTY_NAMES -> switch (obj) {
                case Measurement m -> m.getPropertyNames();
                case DiscreteMeasurement m -> m.getPropertyNames();
                default -> ((BasePropertiesHolder) obj).getPropertyNames();
            };
            case ALIASES        -> ((Identifiable<?>) obj).getAliases();
            case NETWORK_COUNTRIES -> ((Network) obj).getCountries().stream().map(Country::name).toList();
            case AREA_TYPE -> StreamSupport.stream(((Network) obj).getAreaTypes().spliterator(), false).toList();
            case VARIANT_IDS -> ((VariantManager) obj).getVariantIds();
            case SUBSTATION_GEOGRAPHICAL_TAGS -> ((Substation) obj).getGeographicalTags();
            case ALL_SELECTED_OLG_IDS -> LimitsDispatch.holder(obj).getAllSelectedOperationalLimitsGroupIds();
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
            case PROPERTY        -> switch (obj) {
                case Measurement m -> m.getProperty(key);
                case DiscreteMeasurement m -> m.getProperty(key);
                default -> ((BasePropertiesHolder) obj).getProperty(key);
            };
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
                switch (obj) {
                    case Measurement m -> {
                        if (value == null) m.removeProperty(key);
                        else m.putProperty(key, value);
                    }
                    case DiscreteMeasurement m -> {
                        if (value == null) m.removeProperty(key);
                        else m.putProperty(key, value);
                    }
                    default -> {
                        if (value == null) ((PropertiesHolder) obj).removeProperty(key);
                        else ((BasePropertiesHolder) obj).setProperty(key, value);
                    }
                }
            }
            case ALIAS_TYPE -> {
                if (value == null) ((Identifiable<?>) obj).removeAlias(key);
                else ((Identifiable<?>) obj).addAlias(key, value);
            }
            case UNIQUE_ALIAS_TYPE -> ((Identifiable<?>) obj).addAlias(key, Objects.requireNonNull(value), true);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown keyed property for set: " + property);
        }
    }

    // ── Methods that take arguments ───────────────────────────────────────

    public static double evaluate(long handle, int code, double[] args) {
        return evaluate(NetworkRegistry.lookup(handle), code, args);
    }

    /**
     * The result of a method that takes numeric arguments (ints, bools and enum ordinals come as
     * doubles); a boolean result is 1 or 0. The number of arguments chooses between overloads.
     */
    public static double evaluate(Object obj, int code, double[] args) {
        return switch (code) {
            case RL_MIN_Q -> {
                ReactiveLimits limits = (ReactiveLimits) obj;
                yield switch (args.length) {
                    case 1 -> limits.getMinQ(args[0]);
                    case 2 -> limits instanceof ReactiveCapabilityCurve curve
                        ? curve.getMinQ(args[0], args[1] != 0)
                        : ((ReactiveCapabilityShape) limits).getMinQ(args[0], args[1]);
                    default -> throw wrongArgumentCount(code, args);
                };
            }
            case RL_MAX_Q -> {
                ReactiveLimits limits = (ReactiveLimits) obj;
                yield switch (args.length) {
                    case 1 -> limits.getMaxQ(args[0]);
                    case 2 -> limits instanceof ReactiveCapabilityCurve curve
                        ? curve.getMaxQ(args[0], args[1] != 0)
                        : ((ReactiveCapabilityShape) limits).getMaxQ(args[0], args[1]);
                    default -> throw wrongArgumentCount(code, args);
                };
            }
            case LL_TEMPORARY_LIMIT_VALUE -> {
                if (args.length != 1) throw wrongArgumentCount(code, args);
                yield ((LoadingLimits) obj).getTemporaryLimitValue((int) args[0]);
            }
            case SHUNT_B -> {
                if (args.length != 1) throw wrongArgumentCount(code, args);
                yield ((ShuntCompensator) obj).getB((int) args[0]);
            }
            case SHUNT_G -> {
                if (args.length != 1) throw wrongArgumentCount(code, args);
                yield ((ShuntCompensator) obj).getG((int) args[0]);
            }
            case CHILD_COUNT -> {
                if (args.length != 1) throw wrongArgumentCount(code, args);
                yield childStream(obj, (int) args[0]).count();
            }
            // At a node of a node-breaker view
            case FICTITIOUS_P0 -> {
                if (args.length != 1) throw wrongArgumentCount(code, args);
                yield ((VoltageLevel) obj).getNodeBreakerView().getFictitiousP0((int) args[0]);
            }
            case FICTITIOUS_Q0 -> {
                if (args.length != 1) throw wrongArgumentCount(code, args);
                yield ((VoltageLevel) obj).getNodeBreakerView().getFictitiousQ0((int) args[0]);
            }
            case VL_NBV_ATTACHED_EQUIPMENT -> {
                if (args.length != 1) throw wrongArgumentCount(code, args);
                yield ((VoltageLevel) obj).getNodeBreakerView().hasAttachedEquipment((int) args[0]) ? 1 : 0;
            }
            case NETWORK_VALIDATION_LEVEL -> {
                if (args.length != 1) throw wrongArgumentCount(code, args);
                yield ((Network) obj).runValidationChecks(args[0] != 0).ordinal();
            }
            case OVERLOADED -> {
                Object checked = overloadChecked(obj);
                if (args.length != 1) throw wrongArgumentCount(code, args);
                yield (checked instanceof ThreeWindingsTransformer twt
                    ? twt.isOverloaded(args[0])
                    : ((Branch<?>) checked).isOverloaded(args[0])) ? 1 : 0;
            }
            case CHECK_PERMANENT_LIMIT -> {
                // {side, type} or {side, limit reduction, type}
                Object checked = overloadChecked(obj);
                if (args.length != 2 && args.length != 3) throw wrongArgumentCount(code, args);
                LimitType type = Creation.enumValue(LimitType.class, (int) args[args.length - 1]);
                boolean over;
                if (checked instanceof ThreeWindingsTransformer twt) {
                    ThreeSides side = Creation.enumValue(ThreeSides.class, (int) args[0]);
                    over = args.length == 2 ? twt.checkPermanentLimit(side, type) : twt.checkPermanentLimit(side, args[1], type);
                } else {
                    TwoSides side = Creation.enumValue(TwoSides.class, (int) args[0]);
                    Branch<?> branch = (Branch<?>) checked;
                    over = args.length == 2 ? branch.checkPermanentLimit(side, type) : branch.checkPermanentLimit(side, args[1], type);
                }
                yield over ? 1 : 0;
            }
            case SHAPE_IS_INSIDE -> {
                ReactiveCapabilityShape shape = (ReactiveCapabilityShape) obj;
                if (args.length != 3) throw wrongArgumentCount(code, args);
                yield shape.isInside(args[0], args[1], args[2]) ? 1 : 0;
            }
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown code to evaluate: " + code);
        };
    }

    private static BridgeErrors.PropertyNotFoundException wrongArgumentCount(int code, double[] args) {
        return new BridgeErrors.PropertyNotFoundException("Code " + code + " does not take " + args.length + " arguments");
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
            case GENERATOR -> switch (obj) {
                case VoltageLevel vl -> StreamSupport.stream(vl.getConnectables(Generator.class).spliterator(), false);
                case Bus bus -> bus.getGeneratorStream();
                default -> ((Network) obj).getGeneratorStream();
            };
            case LOAD -> switch (obj) {
                case VoltageLevel vl -> StreamSupport.stream(vl.getConnectables(Load.class).spliterator(), false);
                case Bus bus -> bus.getLoadStream();
                default -> ((Network) obj).getLoadStream();
            };
            case BATTERY -> switch (obj) {
                case VoltageLevel vl -> StreamSupport.stream(vl.getConnectables(Battery.class).spliterator(), false);
                case Bus bus -> bus.getBatteryStream();
                default -> ((Network) obj).getBatteryStream();
            };
            case SHUNT_COMPENSATOR -> switch (obj) {
                case VoltageLevel vl -> StreamSupport.stream(vl.getConnectables(ShuntCompensator.class).spliterator(), false);
                case Bus bus -> bus.getShuntCompensatorStream();
                default -> ((Network) obj).getShuntCompensatorStream();
            };
            case STATIC_VAR_COMPENSATOR -> switch (obj) {
                case VoltageLevel vl -> StreamSupport.stream(vl.getConnectables(StaticVarCompensator.class).spliterator(), false);
                case Bus bus -> bus.getStaticVarCompensatorStream();
                default -> ((Network) obj).getStaticVarCompensatorStream();
            };
            case BOUNDARY_LINE -> switch (obj) {
                case VoltageLevel vl -> StreamSupport.stream(vl.getConnectables(BoundaryLine.class).spliterator(), false);
                case Bus bus -> bus.getBoundaryLineStream();
                default -> ((Network) obj).getBoundaryLineStream();
            };
            case LCC_CONVERTER_STATION -> switch (obj) {
                case VoltageLevel vl -> StreamSupport.stream(vl.getConnectables(LccConverterStation.class).spliterator(), false);
                case Bus bus -> bus.getLccConverterStationStream();
                default -> ((Network) obj).getLccConverterStationStream();
            };
            case VSC_CONVERTER_STATION -> switch (obj) {
                case VoltageLevel vl -> StreamSupport.stream(vl.getConnectables(VscConverterStation.class).spliterator(), false);
                case Bus bus -> bus.getVscConverterStationStream();
                default -> ((Network) obj).getVscConverterStationStream();
            };
            case LINE -> switch (obj) {
                case VoltageLevel vl -> vl.getLineStream();
                case Bus bus -> bus.getLineStream();
                default -> ((Network) obj).getLineStream();
            };
            case IDENTIFIABLE -> ((Network) obj).getIdentifiables().stream();
            case CONNECTABLE -> {
                if (obj instanceof VoltageLevel vl) yield vl.getConnectableStream();
                yield ((Network) obj).getConnectableStream();
            }
            case BRANCH -> ((Network) obj).getBranchStream();
            case HVDC_CONVERTER_STATION -> ((Network) obj).getHvdcConverterStationStream();
            case TERMINAL -> obj instanceof Bus bus
                ? bus.getConnectedTerminalStream()
                : ((Connectable<?>) obj).getTerminals().stream();
            case SUBSTATION  -> ((Network) obj).getSubstationStream();
            case VOLTAGE_LEVEL -> switch (obj) {
                case Substation sub -> StreamSupport.stream(sub.getVoltageLevels().spliterator(), false);
                // A set: sorted, so that both backends list it in the same order
                case Area area -> StreamSupport.stream(area.getVoltageLevels().spliterator(), false)
                    .sorted(Comparator.comparing(VoltageLevel::getId));
                default -> ((Network) obj).getVoltageLevelStream();
            };
            case AREA -> {
                if (obj instanceof VoltageLevel vl) yield vl.getAreasStream();
                yield ((Network) obj).getAreaStream();
            }
            case AREA_BOUNDARY -> StreamSupport.stream(((Area) obj).getAreaBoundaries().spliterator(), false);
            case OVERLOAD_MANAGEMENT_SYSTEM -> {
                if (obj instanceof Substation substation) {
                    yield StreamSupport.stream(substation.getOverloadManagementSystems().spliterator(), false);
                }
                yield ((Network) obj).getOverloadManagementSystemStream();
            }
            case TRIPPING -> ((OverloadManagementSystem) obj).getTrippings().stream();
            case VOLTAGE_ANGLE_LIMIT -> ((Network) obj).getVoltageAngleLimitsStream();
            case TWO_WINDINGS_TRANSFORMER -> switch (obj) {
                case Substation sub -> StreamSupport.stream(sub.getTwoWindingsTransformers().spliterator(), false);
                case VoltageLevel vl -> vl.getTwoWindingsTransformerStream();
                case Bus bus -> bus.getTwoWindingsTransformerStream();
                default -> ((Network) obj).getTwoWindingsTransformerStream();
            };
            case THREE_WINDINGS_TRANSFORMER -> switch (obj) {
                case Substation sub -> StreamSupport.stream(sub.getThreeWindingsTransformers().spliterator(), false);
                case VoltageLevel vl -> vl.getThreeWindingsTransformerStream();
                case Bus bus -> bus.getThreeWindingsTransformerStream();
                default -> ((Network) obj).getThreeWindingsTransformerStream();
            };
            case HVDC_LINE           -> ((Network) obj).getHvdcLineStream();
            case TIE_LINE            -> ((Network) obj).getTieLineStream();
            case GROUND -> {
                if (obj instanceof VoltageLevel vl) yield vl.getGroundStream();
                yield ((Network) obj).getGroundStream();
            }
            case SWITCH -> {
                if (obj instanceof Network network) yield network.getSwitchStream();
                yield StreamSupport.stream(((VoltageLevel) obj).getSwitches().spliterator(), false);
            }
            case REL_BBV_SWITCH -> {
                if (obj instanceof Network network) yield network.getBusBreakerView().getSwitchStream();
                yield ((VoltageLevel) obj).getBusBreakerView().getSwitchStream();
            }
            case REL_NBV_SWITCH -> ((VoltageLevel) obj).getNodeBreakerView().getSwitchStream();
            case BUSBAR_SECTION -> {
                if (obj instanceof Network network) yield network.getBusbarSectionStream();
                yield StreamSupport.stream(((VoltageLevel) obj).getNodeBreakerView().getBusbarSections().spliterator(), false);
            }
            case INTERNAL_CONNECTION -> StreamSupport.stream(
                ((VoltageLevel) obj).getNodeBreakerView().getInternalConnections().spliterator(), false);
            case BUS -> switch (obj) {
                case Network network -> network.getBusBreakerView().getBusStream();
                case Component component -> StreamSupport.stream(component.getBuses().spliterator(), false);
                default -> StreamSupport.stream(((VoltageLevel) obj).getBusBreakerView().getBuses().spliterator(), false);
            };
            case REL_CONNECTED_COMPONENT -> ((Network) obj).getBusView().getConnectedComponents().stream();
            case NETWORK -> ((Network) obj).getSubnetworks().stream();
            // A set: sorted, so that both backends list it in the same order
            case BOUNDARY_ELEMENT -> ((Network) obj).getBoundaryElements().stream()
                .sorted(Comparator.comparing(Identifiable::getId));
            case REL_SYNCHRONOUS_COMPONENT -> ((Network) obj).getBusView().getSynchronousComponents().stream();
            case REL_BV_BUS -> {
                if (obj instanceof Network network) yield network.getBusView().getBusStream();
                yield ((VoltageLevel) obj).getBusView().getBusStream();
            }
            case REACTIVE_CURVE_POINT -> ((ReactiveCapabilityCurve) obj).getPoints().stream();
            case REACTIVE_CAPABILITY_SHAPE_PLANE -> ((ReactiveCapabilityShape) obj).getPlanes().stream();
            case THREE_WINDINGS_TRANSFORMER_LEG -> ((ThreeWindingsTransformer) obj).getLegs().stream();
            case RATIO_TAP_CHANGER_STEP -> steps((RatioTapChanger) obj);
            case PHASE_TAP_CHANGER_STEP -> steps((PhaseTapChanger) obj);
            case SHUNT_SECTION -> ((ShuntCompensatorNonLinearModel) obj).getAllSections().stream();
            case TEMPORARY_LIMIT -> ((LoadingLimits) obj).getTemporaryLimits().stream();
            case OPERATIONAL_LIMITS_GROUP -> LimitsDispatch.holder(obj).getOperationalLimitsGroups().stream();
            case REL_SELECTED_OLG -> LimitsDispatch.holder(obj).getAllSelectedOperationalLimitsGroups().stream();
            case CURRENT_LIMITS, ACTIVE_POWER_LIMITS, APPARENT_POWER_LIMITS
                -> LimitsDispatch.holder(obj).getAllSelectedLimits(LimitsDispatch.limitType(childType)).stream();
            case ELEMENTS -> ((Collection<?>) obj).stream();
            case MEASUREMENT          -> ((Measurements<?>) obj).getMeasurements().stream();
            case DISCRETE_MEASUREMENT -> ((DiscreteMeasurements<?>) obj).getDiscreteMeasurements().stream();
            case AREA_CHARACTERISTICS -> StateEstimationDispatch.observabilityAreas((ObservabilityArea) obj);
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
                if (obj instanceof ThreeWindingsTransformer.Leg leg) yield leg.getTerminal();
                if (obj instanceof SlackTerminal st) yield st.getTerminal();
                if (obj instanceof AreaBoundary boundary) yield boundary.getTerminal().orElse(null);
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
            case REL_BUS -> ((Terminal) obj).getBusBreakerView().getBus();
            case REL_BV_BUS -> ((Terminal) obj).getBusView().getBus();
            case REL_BV_CONNECTABLE_BUS -> ((Terminal) obj).getBusView().getConnectableBus();
            case REL_VOLTAGE_LEVEL -> switch (obj) {
                case Bus b -> b.getVoltageLevel();
                case Switch sw -> sw.getVoltageLevel();
                case Boundary boundary -> boundary.getNetworkSideVoltageLevel();
                default -> ((Terminal) obj).getVoltageLevel();
            };
            case BOUNDARY_LINE_GENERATION -> ((BoundaryLine) obj).getGeneration();
            case LOAD_MODEL -> ((Load) obj).getModel().orElse(null);
            case BOUNDARY -> obj instanceof AreaBoundary boundary
                ? boundary.getBoundary().orElse(null)
                : ((BoundaryLine) obj).getBoundary();
            case AREA -> ((AreaBoundary) obj).getArea();
            case REL_TERMINAL_FROM -> ((VoltageAngleLimit) obj).getTerminalFrom();
            case REL_TERMINAL_TO -> ((VoltageAngleLimit) obj).getTerminalTo();
            case BOUNDARY_LINE -> ((Boundary) obj).getBoundaryLine();
            case TIE_LINE -> ((BoundaryLine) obj).getTieLine().orElse(null);
            case REL_HVDC_LINE -> ((HvdcConverterStation<?>) obj).getHvdcLine();
            case NETWORK -> ((Identifiable<?>) obj).getNetwork();
            case VARIANT_MANAGER -> ((Network) obj).getVariantManager();
            case REL_CONNECTED_COMPONENT -> ((Bus) obj).getConnectedComponent();
            case REL_SYNCHRONOUS_COMPONENT -> ((Bus) obj).getSynchronousComponent();
            case REL_PARENT_NETWORK -> ((Identifiable<?>) obj).getParentNetwork();
            case REL_CONVERTER_STATION1 -> ((HvdcLine) obj).getConverterStation1();
            case REL_CONVERTER_STATION2 -> ((HvdcLine) obj).getConverterStation2();
            case REL_OTHER_CONVERTER_STATION -> ((HvdcConverterStation<?>) obj).getOtherConverterStation().orElse(null);
            case RATIO_TAP_CHANGER -> ((RatioTapChangerHolder) obj).getOptionalRatioTapChanger().orElse(null);
            case PHASE_TAP_CHANGER -> ((PhaseTapChangerHolder) obj).getOptionalPhaseTapChanger().orElse(null);
            case THREE_WINDINGS_TRANSFORMER -> ((ThreeWindingsTransformer.Leg) obj).getTransformer();
            case REL_CURRENT_STEP        -> tapChanger(obj).getCurrentStep();
            case REL_SOLVED_CURRENT_STEP -> tapChanger(obj).getSolvedCurrentStep();
            case REL_NEUTRAL_STEP        -> tapChanger(obj).getNeutralStep().orElse(null);
            case REL_REGULATION_TERMINAL -> ((PhaseTapChanger) obj).getRegulationTerminal();
            case REL_SUBSTATION -> switch (obj) {
                case OverloadManagementSystem system -> system.getSubstation();
                case TwoWindingsTransformer twt -> twt.getSubstation().orElse(null);
                case ThreeWindingsTransformer twt -> twt.getSubstation().orElse(null);
                default -> ((VoltageLevel) obj).getSubstation().orElse(null);
            };
            case REL_CONNECTABLE_BUS -> ((Terminal) obj).getBusBreakerView().getConnectableBus();
            case REL_REGULATING_TERMINAL -> ((VoltageRegulationHolder<?>) obj).getRegulatingTerminal();
            case REL_SELECTED_OLG -> LimitsDispatch.holder(obj).getSelectedOperationalLimitsGroup().orElse(null);
            case CURRENT_LIMITS, ACTIVE_POWER_LIMITS, APPARENT_POWER_LIMITS
                -> LimitsDispatch.limits(obj, LimitsDispatch.limitType(relation));
            case REL_CONNECTABLE -> ((Terminal) obj).getConnectable();
            case TEMPORARY_LIMIT -> ((Overload) obj).getTemporaryLimit();
            case VOLTAGE_REGULATION -> ((VoltageRegulationHolder<?>) obj).getVoltageRegulation();
            case ACTIVE_POWER_CONTROL -> extension(obj, ActivePowerControl.class);
            case COORDINATED_REACTIVE_CONTROL -> extension(obj, CoordinatedReactiveControl.class);
            case HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL -> extension(obj, HvdcAngleDroopActivePowerControl.class);
            case HVDC_OPERATOR_ACTIVE_POWER_RANGE -> extension(obj, HvdcOperatorActivePowerRange.class);
            case SLACK_TERMINAL -> extension(obj, SlackTerminal.class);
            case TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED -> extension(obj, TwoWindingsTransformerToBeEstimated.class);
            case THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED -> extension(obj, ThreeWindingsTransformerToBeEstimated.class);
            case INJECTION_OBSERVABILITY -> extension(obj, InjectionObservability.class);
            case BRANCH_OBSERVABILITY -> extension(obj, BranchObservability.class);
            case MEASUREMENTS -> extension(obj, Measurements.class);
            case DISCRETE_MEASUREMENTS -> extension(obj, DiscreteMeasurements.class);
            case OBSERVABILITY_AREA -> extension(obj, ObservabilityArea.class);
            case OBS_QUALITY_P  -> ((InjectionObservability<?>) obj).getNullableQualityP();
            case OBS_QUALITY_Q  -> ((InjectionObservability<?>) obj).getNullableQualityQ();
            case OBS_QUALITY_V  -> ((InjectionObservability<?>) obj).getNullableQualityV();
            case OBS_QUALITY_P1 -> ((BranchObservability<?>) obj).getNullableQualityP1();
            case OBS_QUALITY_P2 -> ((BranchObservability<?>) obj).getNullableQualityP2();
            case OBS_QUALITY_Q1 -> ((BranchObservability<?>) obj).getNullableQualityQ1();
            case OBS_QUALITY_Q2 -> ((BranchObservability<?>) obj).getNullableQualityQ2();
            case SHUNT_COMPENSATOR_MODEL   -> ((ShuntCompensator) obj).getModel();
            case SHUNT_COMPENSATOR_LINEAR_MODEL -> ((ShuntCompensator) obj).getModel(ShuntCompensatorLinearModel.class);
            case SHUNT_COMPENSATOR_NON_LINEAR_MODEL -> ((ShuntCompensator) obj).getModel(ShuntCompensatorNonLinearModel.class);
            case REACTIVE_LIMITS           -> ((ReactiveLimitsHolder) obj).getReactiveLimits();
            case MIN_MAX_REACTIVE_LIMITS   -> ((ReactiveLimitsHolder) obj).getReactiveLimits(MinMaxReactiveLimits.class);
            case REACTIVE_CAPABILITY_CURVE -> ((ReactiveLimitsHolder) obj).getReactiveLimits(ReactiveCapabilityCurve.class);
            case REACTIVE_CAPABILITY_SHAPE -> ((ReactiveLimitsHolder) obj).getReactiveLimits(ReactiveCapabilityShape.class);
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
            case REL_TERMINAL -> ((SlackTerminal) obj).setTerminal((Terminal) value);
            case REL_REGULATION_TERMINAL -> ((PhaseTapChanger) obj).setRegulationTerminal((Terminal) value);
            case ADD_VOLTAGE_LEVEL    -> ((Area) obj).addVoltageLevel((VoltageLevel) value);
            case REMOVE_VOLTAGE_LEVEL -> ((Area) obj).removeVoltageLevel((VoltageLevel) value);
            case ADD_AREA             -> ((VoltageLevel) obj).addArea((Area) value);
            case REMOVE_AREA          -> ((VoltageLevel) obj).removeArea((Area) value);
            case ADD_TRIPPING -> ((OverloadManagementSystem) obj).addTripping((OverloadManagementSystem.Tripping) value);
            case REMOVE_AREA_BOUNDARY -> {
                if (value instanceof Terminal terminal) ((Area) obj).removeAreaBoundary(terminal);
                else ((Area) obj).removeAreaBoundary((Boundary) value);
            }
            // Only clearing (INVALID_HANDLE): new ones are created with invoke
            case VOLTAGE_REGULATION, ACTIVE_POWER_CONTROL, COORDINATED_REACTIVE_CONTROL,
                 HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL, HVDC_OPERATOR_ACTIVE_POWER_RANGE, SLACK_TERMINAL,
                 TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED, THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED,
                 INJECTION_OBSERVABILITY, BRANCH_OBSERVABILITY, MEASUREMENTS, DISCRETE_MEASUREMENTS,
                 OBSERVABILITY_AREA, REL_SELECTED_OLG, CURRENT_LIMITS, ACTIVE_POWER_LIMITS,
                 APPARENT_POWER_LIMITS -> Creation.clear(obj, relation, value);
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
            case ACT_CONNECT -> connect(obj) ? obj : null;
            case ACT_DISCONNECT -> disconnect(obj) ? obj : null;
            case ACT_SET_TARGET_P_TO_P -> {
                switch (obj) {
                    case Generator generator -> generator.setTargetPToP();
                    case Battery battery -> battery.setTargetPToP();
                    case Load load -> load.setP0ToP();
                    default -> ((BoundaryLine) obj).setGenerationTargetPToP();
                }
                yield null;
            }
            case ACT_SET_TARGET_Q_TO_Q -> {
                switch (obj) {
                    case BoundaryLine line -> line.setGenerationTargetQToQ();
                    case Load load -> load.setQ0ToQ();
                    default -> ((VoltageRegulationHolder<?>) obj).setTargetQToQ();
                }
                yield null;
            }
            case ACT_SET_TARGET_V_TO_V -> {
                if (obj instanceof BoundaryLine line) line.setGenerationTargetVToV();
                else ((VoltageRegulationHolder<?>) obj).setTargetVToV();
                yield null;
            }
            case ACT_UNSET_TAP_POSITION -> {
                tapChanger(obj).unsetTapPosition();
                yield null;
            }
            case ACT_UNSET_SOLVED_TAP_POSITION -> {
                tapChanger(obj).unsetSolvedTapPosition();
                yield null;
            }
            case ACT_APPLY_SOLVED_VALUES -> {
                switch (obj) {
                    case TapChanger<?, ?, ?, ?> tapChanger -> tapChanger.applySolvedValues();
                    case TwoWindingsTransformer transformer -> transformer.applySolvedValues();
                    case ThreeWindingsTransformer transformer -> transformer.applySolvedValues();
                    case Generator generator -> generator.applySolvedValues();
                    case Battery battery -> battery.applySolvedValues();
                    case Load load -> load.applySolvedValues();
                    case ShuntCompensator shunt -> shunt.applySolvedValues();
                    case BoundaryLine line -> line.applySolvedValues();
                    default -> throw new BridgeErrors.PropertyNotFoundException("No solved values to apply: " + obj);
                }
                yield null;
            }
            case ACT_UNSET_SOLVED_VALUES -> {
                switch (obj) {
                    case Connectable<?> connectable -> connectable.unsetSolvedValues();
                    case Terminal terminal -> terminal.unsetSolvedValues();
                    case Bus bus -> bus.unsetSolvedValues();
                    default -> throw new BridgeErrors.PropertyNotFoundException("No solved values to unset: " + obj);
                }
                yield null;
            }
            case ACT_UNSET_SECTION_COUNT -> {
                ((ShuntCompensator) obj).unsetSectionCount();
                yield null;
            }
            case ACT_UNSET_SOLVED_SECTION_COUNT -> {
                ((ShuntCompensator) obj).unsetSolvedSectionCount();
                yield null;
            }
            case ACT_SET_SECTION_COUNT_TO_SOLVED_SECTION_COUNT -> {
                ((ShuntCompensator) obj).setSectionCountToSolvedSectionCount();
                yield null;
            }
            case ACT_REMOVE_ALL_BUSES -> {
                ((VoltageLevel) obj).getBusBreakerView().removeAllBuses();
                yield null;
            }
            case ACT_REMOVE_ALL_SWITCHES -> {
                ((VoltageLevel) obj).getBusBreakerView().removeAllSwitches();
                yield null;
            }
            case ACT_REMOVE_UPDATING_BOUNDARY_LINES -> {
                ((TieLine) obj).remove(true);
                yield null;
            }
            case ACT_DETACH -> ((Network) obj).detach();
            case ACT_FLATTEN -> {
                ((Network) obj).flatten();
                yield null;
            }
            case ACT_CLEAN_IF_EMPTY -> {
                switch (obj) {
                    case Measurements<?> measurements -> measurements.cleanIfEmpty();
                    case DiscreteMeasurements<?> measurements -> measurements.cleanIfEmpty();
                    case SlackTerminal slackTerminal -> slackTerminal.setTerminal(slackTerminal.getTerminal(), true);
                    default -> throw new BridgeErrors.PropertyNotFoundException("Cannot clean " + obj);
                }
                yield null;
            }
            case ACT_SET_TAP_POSITION_TO_SOLVED_TAP_POSITION -> {
                tapChanger(obj).setTapPositionToSolvedTapPosition();
                yield null;
            }
            case ACT_SET_RATIO_TAP_POSITION_TO_SOLVED_TAP_POSITION -> {
                if (obj instanceof ThreeWindingsTransformer twt) twt.setRatioTapPositionToSolvedTapPosition();
                else ((TwoWindingsTransformer) obj).setRatioTapPositionToSolvedTapPosition();
                yield null;
            }
            case ACT_SET_PHASE_TAP_POSITION_TO_SOLVED_TAP_POSITION -> {
                if (obj instanceof ThreeWindingsTransformer twt) twt.setPhaseTapPositionToSolvedTapPosition();
                else ((TwoWindingsTransformer) obj).setPhaseTapPositionToSolvedTapPosition();
                yield null;
            }
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

    /** Connects a terminal, a connectable, the converter stations of an HVDC line: whether this changed it. */
    private static boolean connect(Object obj) {
        return switch (obj) {
            case Terminal terminal -> terminal.connect();
            case HvdcLine line -> line.connectConverterStations();
            case TieLine line -> line.connectBoundaryLines();
            default -> ((Connectable<?>) obj).connect();
        };
    }

    private static boolean disconnect(Object obj) {
        return switch (obj) {
            case Terminal terminal -> terminal.disconnect();
            case HvdcLine line -> line.disconnectConverterStations();
            case TieLine line -> line.disconnectBoundaryLines();
            default -> ((Connectable<?>) obj).disconnect();
        };
    }

    // ── Int list (navigation) ─────────────────────────────────────────────

    public static int[] getIntList(long handle, int listCode) {
        return getIntList(NetworkRegistry.lookup(handle), listCode);
    }

    public static int[] getIntList(Object obj, int listCode) {
        return switch (listCode) {
            case VL_NBV_NODES -> ((VoltageLevel) obj).getNodeBreakerView().getNodes();
            case ELEMENTS -> ((Collection<?>) obj).stream().mapToInt(element -> (Integer) element).toArray();
            case REGULATING_MODES -> modes(((VoltageRegulationHolder<?>) obj)::isRegulatingWithMode);
            case WITH_MODES       -> modes(((VoltageRegulationHolder<?>) obj)::isWithMode);
            case OBS_AREA_NODES -> StateEstimationDispatch.observabilityAreaNodes(obj);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown int list code: " + listCode);
        };
    }

    // ── Related by string key (navigation) ───────────────────────────────────

    public static long getRelatedByString(long handle, int relation, String key) {
        return registerOrZero(getRelatedObjectByString(NetworkRegistry.lookup(handle), relation, key));
    }

    public static Object getRelatedObjectByString(Object obj, int relation, String key) {
        if (obj instanceof Network network) {
            return switch (relation) {
                case REL_BBV_BUS -> network.getBusBreakerView().getBus(key);
                case REL_BV_BUS -> network.getBusView().getBus(key);
                default -> findObjectById(network, relation, key); // in this network only
            };
        }
        return switch (relation) {
            case REL_TERMINAL -> {
                if (obj instanceof Branch<?> branch) yield branch.getTerminal(key);
                yield ((ThreeWindingsTransformer) obj).getTerminal(key);
            }
            case REL_BBV_BUS -> ((VoltageLevel) obj).getBusBreakerView().getBus(key);
            case REL_BV_BUS -> ((VoltageLevel) obj).getBusView().getBus(key);
            case REL_MERGED_BUS -> ((VoltageLevel) obj).getBusView().getMergedBus(key);
            // A set: sorted, so that both backends list it in the same order
            case REL_BBV_BUSES_OF_BV_BUS -> ((VoltageLevel) obj).getBusBreakerView().getBusesFromBusViewBusId(key).stream()
                .sorted(Comparator.comparing(Bus::getId)).toList();
            case BUSBAR_SECTION -> ((VoltageLevel) obj).getNodeBreakerView().getBusbarSection(key);
            case BOUNDARY_LINE -> ((TieLine) obj).getBoundaryLine(key);
            case AREA -> ((VoltageLevel) obj).getArea(key).orElse(null);
            case REL_BBV_SWITCH -> ((VoltageLevel) obj).getBusBreakerView().getSwitch(key);
            case REL_NBV_SWITCH -> ((VoltageLevel) obj).getNodeBreakerView().getSwitch(key);
            case CONNECTABLE -> ((VoltageLevel) obj).getConnectable(key, Connectable.class);
            case REL_VL_BBV_BUS1 -> ((VoltageLevel) obj).getBusBreakerView().getBus1(key);
            case REL_VL_BBV_BUS2 -> ((VoltageLevel) obj).getBusBreakerView().getBus2(key);
            case MEASUREMENT          -> ((Measurements<?>) obj).getMeasurement(key);
            case DISCRETE_MEASUREMENT -> ((DiscreteMeasurements<?>) obj).getDiscreteMeasurement(key);
            case REL_OBS_AREA_BY_BUS, REL_OBS_AREA_BY_BBV_BUS
                -> StateEstimationDispatch.observabilityAreaByBus((ObservabilityArea) obj, relation, key);
            case OPERATIONAL_LIMITS_GROUP -> LimitsDispatch.holder(obj).getOperationalLimitsGroup(key).orElse(null);
            case CURRENT_LIMITS, ACTIVE_POWER_LIMITS, APPARENT_POWER_LIMITS
                -> LimitsDispatch.holder(obj).getLimitsFromId(LimitsDispatch.limitType(relation), key).orElse(null);
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
            case REL_NBV_SWITCH -> ((VoltageLevel) obj).getNodeBreakerView().getSwitches(index);
            case BOUNDARY_LINE -> ((TieLine) obj).getBoundaryLine(Creation.enumValue(TwoSides.class, index));
            case INTERNAL_CONNECTION -> ((VoltageLevel) obj).getNodeBreakerView().getNodesInternalConnectedTo(index);
            case REL_TERMINAL -> ((ThreeWindingsTransformer) obj).getTerminal(Creation.enumValue(ThreeSides.class, index));
            case THREE_WINDINGS_TRANSFORMER_LEG -> ((ThreeWindingsTransformer) obj).getLeg(Creation.enumValue(ThreeSides.class, index));
            case RATIO_TAP_CHANGER_STEP -> ((RatioTapChanger) obj).getStep(index);
            case PHASE_TAP_CHANGER_STEP -> ((PhaseTapChanger) obj).getStep(index);
            case REL_OBS_AREA_BY_NODE -> StateEstimationDispatch.observabilityAreaByNode((ObservabilityArea) obj, index);
            case FLOWS_LIMITS_HOLDER -> LimitsDispatch.branchSide(obj, index);
            case LOADING_LIMITS -> LimitsDispatch.limits(obj, Creation.enumValue(LimitType.class, index));
            case TEMPORARY_LIMIT -> ((LoadingLimits) obj).getTemporaryLimit(index);
            case HVDC_CONVERTER_STATION -> ((HvdcLine) obj).getConverterStation(Creation.enumValue(TwoSides.class, index));
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
            case TIE_LINE                   -> network.getTieLine(id);
            case AREA                       -> network.getArea(id);
            case VOLTAGE_ANGLE_LIMIT        -> network.getVoltageAngleLimit(id);
            case OVERLOAD_MANAGEMENT_SYSTEM -> network.getOverloadManagementSystem(id);
            case GROUND                     -> network.getGround(id);
            case BOUNDARY_LINE              -> network.getBoundaryLine(id);
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
            case NETWORK                    -> network.getSubnetwork(id);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown object type: " + objectType);
        };
    }

    // ── Extensions ────────────────────────────────────────────────────────

    /** The extension of the given class of an extendable object, or null. */
    @SuppressWarnings({"unchecked", "rawtypes"})
    private static Object extension(Object extendable, Class type) {
        // Raw: the extension classes are generic in their extendable, which Class<X> cannot express
        return ((Extendable) extendable).getExtension(type);
    }

    /** The pairing key of a boundary line or of a tie line (null when it has none). */
    private static String pairingKey(Object obj) {
        return obj instanceof TieLine tieLine ? tieLine.getPairingKey() : ((BoundaryLine) obj).getPairingKey();
    }

    // ── Overloads ─────────────────────────────────────────────────────────

    /** What the overload checks apply to: a branch or a three windings transformer (else a ClassCastException). */
    static Object overloadChecked(Object obj) {
        return obj instanceof ThreeWindingsTransformer ? obj : (Branch<?>) obj;
    }

    // ── Tap changers ──────────────────────────────────────────────────────

    static TapChanger<?, ?, ?, ?> tapChanger(Object obj) {
        return (TapChanger<?, ?, ?, ?>) obj;
    }

    /** The steps of a tap changer, from its low to its high tap position. */
    private static Stream<?> steps(TapChanger<?, ?, ?, ?> tapChanger) {
        return IntStream.rangeClosed(tapChanger.getLowTapPosition(), tapChanger.getHighTapPosition())
            .mapToObj(tapChanger::getStep);
    }

    // ── Extension helpers ─────────────────────────────────────────────────

    /** The ActivePowerControl extension object (from getRelated(injection, ACTIVE_POWER_CONTROL)). */
    static ActivePowerControl<?> activePowerControl(Object obj) {
        return (ActivePowerControl<?>) obj;
    }

    /** The ordinals of the regulation modes that satisfy a test. */
    private static int[] modes(java.util.function.Predicate<RegulationMode> test) {
        return Arrays.stream(RegulationMode.values()).filter(test).mapToInt(Enum::ordinal).toArray();
    }
}
