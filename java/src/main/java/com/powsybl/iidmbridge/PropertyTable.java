package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.LoadingLimits.TemporaryLimit;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidm.network.regulation.VoltageRegulation;
import com.powsybl.iidm.network.regulation.VoltageRegulationAdderOrBuilder;
import com.powsybl.iidm.network.regulation.VoltageRegulationHolder;
import com.powsybl.iidm.network.regulation.VoltageRegulationHolderAdder;

import java.util.function.BiConsumer;
import java.util.function.Function;
import java.util.function.ObjDoubleConsumer;
import java.util.function.ObjIntConsumer;
import java.util.function.Predicate;
import java.util.function.ToDoubleFunction;
import java.util.function.ToIntFunction;

import static com.powsybl.iidmbridge.PropertyCodes.*;

/**
 * The properties that are one call on a powsybl object, by code: their getter, setter and adder
 * setter on one line (null when there is none, or when it needs more than one call: it is then a
 * case of {@link PropertyDispatcher} or {@link Creation}, which look a code up here first). Given an
 * object of another type, they throw a ClassCastException, as a cast does.
 */
final class PropertyTable {

    /** A setter of a boolean (java.util.function has none). */
    interface BoolSetter<T> {
        void accept(T object, boolean value);
    }

    record DoubleProperty(ToDoubleFunction<Object> get, ObjDoubleConsumer<Object> set, ObjDoubleConsumer<Object> add) {
    }

    record IntProperty(ToIntFunction<Object> get, ObjIntConsumer<Object> set, ObjIntConsumer<Object> add) {
    }

    record BoolProperty(Predicate<Object> get, BoolSetter<Object> set, BoolSetter<Object> add) {
    }

    record StringProperty(Function<Object, String> get, BiConsumer<Object, String> set, BiConsumer<Object, String> add) {
    }

    /** Above every code (a code at or above it fails the loading of this class, and so every test). */
    private static final int SIZE = 10_000;

    private static final DoubleProperty[] DOUBLES = new DoubleProperty[SIZE];
    private static final IntProperty[] INTS = new IntProperty[SIZE];
    private static final BoolProperty[] BOOLS = new BoolProperty[SIZE];
    private static final StringProperty[] STRINGS = new StringProperty[SIZE];

    private PropertyTable() {
    }

    static DoubleProperty doubleProperty(int code) {
        return code >= 0 && code < SIZE ? DOUBLES[code] : null;
    }

    static IntProperty intProperty(int code) {
        return code >= 0 && code < SIZE ? INTS[code] : null;
    }

    static BoolProperty boolProperty(int code) {
        return code >= 0 && code < SIZE ? BOOLS[code] : null;
    }

    static StringProperty stringProperty(int code) {
        return code >= 0 && code < SIZE ? STRINGS[code] : null;
    }

    static {
        // ── Doubles ─────────────────────────────────────────────────────────────────────────────────
        doubles(GEN_TARGET_P, Generator::getTargetP, Generator::setTargetP, GeneratorAdder::setTargetP);
        doubles(GEN_MIN_P, Generator::getMinP, Generator::setMinP, GeneratorAdder::setMinP);
        doubles(GEN_MAX_P, Generator::getMaxP, Generator::setMaxP, GeneratorAdder::setMaxP);
        doubles(GEN_RATED_S, Generator::getRatedS, Generator::setRatedS, GeneratorAdder::setRatedS);
        doubles(LOCAL_TARGET_V, (VoltageRegulationHolder<?> x) -> x.getLocalTargetV(),
            (VoltageRegulationHolder<?> x, double v) -> x.setLocalTargetV(v),
            (VoltageRegulationHolderAdder<?> x, double v) -> x.setLocalTargetV(v));
        doubles(LOCAL_TARGET_Q, (VoltageRegulationHolder<?> x) -> x.getLocalTargetQ(),
            (VoltageRegulationHolder<?> x, double v) -> x.setLocalTargetQ(v),
            (VoltageRegulationHolderAdder<?> x, double v) -> x.setLocalTargetQ(v));
        doubles(REGULATING_TARGET_V, (VoltageRegulationHolder<?> x) -> x.getRegulatingTargetV(), null, null);
        doubles(REGULATING_TARGET_Q, (VoltageRegulationHolder<?> x) -> x.getRegulatingTargetQ(), null, null);
        doubles(VR_TARGET_VALUE, VoltageRegulation::getTargetValue, VoltageRegulation::setTargetValue,
            (VoltageRegulationAdderOrBuilder<?> x, double v) -> x.withTargetValue(v));
        doubles(VR_TARGET_DEADBAND, VoltageRegulation::getTargetDeadband, VoltageRegulation::setTargetDeadband,
            (VoltageRegulationAdderOrBuilder<?> x, double v) -> x.withTargetDeadband(v));
        doubles(VR_SLOPE, VoltageRegulation::getSlope, VoltageRegulation::setSlope,
            (VoltageRegulationAdderOrBuilder<?> x, double v) -> x.withSlope(v));
        doubles(LOAD_P0, Load::getP0, Load::setP0, LoadAdder::setP0);
        doubles(LOAD_Q0, Load::getQ0, Load::setQ0, LoadAdder::setQ0);
        doubles(ZIP_C0P, ZipLoadModel::getC0p, ZipLoadModel::setC0p, ZipLoadModelAdder::setC0p);
        doubles(ZIP_C1P, ZipLoadModel::getC1p, ZipLoadModel::setC1p, ZipLoadModelAdder::setC1p);
        doubles(ZIP_C2P, ZipLoadModel::getC2p, ZipLoadModel::setC2p, ZipLoadModelAdder::setC2p);
        doubles(ZIP_C0Q, ZipLoadModel::getC0q, ZipLoadModel::setC0q, ZipLoadModelAdder::setC0q);
        doubles(ZIP_C1Q, ZipLoadModel::getC1q, ZipLoadModel::setC1q, ZipLoadModelAdder::setC1q);
        doubles(ZIP_C2Q, ZipLoadModel::getC2q, ZipLoadModel::setC2q, ZipLoadModelAdder::setC2q);
        doubles(EXP_NP, ExponentialLoadModel::getNp, ExponentialLoadModel::setNp, ExponentialLoadModelAdder::setNp);
        doubles(EXP_NQ, ExponentialLoadModel::getNq, ExponentialLoadModel::setNq, ExponentialLoadModelAdder::setNq);
        doubles(TERMINAL_P, Terminal::getP, Terminal::setP, null);
        doubles(TERMINAL_Q, Terminal::getQ, Terminal::setQ, null);
        doubles(TERMINAL_I, Terminal::getI, null, null);
        doubles(BUS_V, Bus::getV, Bus::setV, null);
        doubles(BUS_ANGLE, Bus::getAngle, Bus::setAngle, null);
        doubles(BUS_P, Bus::getP, null, null);
        doubles(BUS_Q, Bus::getQ, null, null);
        doubles(FICTITIOUS_P0, Bus::getFictitiousP0, Bus::setFictitiousP0, null);
        doubles(FICTITIOUS_Q0, Bus::getFictitiousQ0, Bus::setFictitiousQ0, null);
        doubles(VL_NOMINAL_V, VoltageLevel::getNominalV, VoltageLevel::setNominalV, VoltageLevelAdder::setNominalV);
        doubles(VL_LOW_VOLTAGE_LIMIT, VoltageLevel::getLowVoltageLimit, VoltageLevel::setLowVoltageLimit,
            VoltageLevelAdder::setLowVoltageLimit);
        doubles(VL_HIGH_VOLTAGE_LIMIT, VoltageLevel::getHighVoltageLimit, VoltageLevel::setHighVoltageLimit,
            VoltageLevelAdder::setHighVoltageLimit);
        doubles(LINE_R, LineCharacteristics::getR, Line::setR, LineAdder::setR);
        doubles(LINE_X, LineCharacteristics::getX, Line::setX, LineAdder::setX);
        doubles(LINE_G1, LineCharacteristics::getG1, Line::setG1, LineAdder::setG1);
        doubles(LINE_B1, LineCharacteristics::getB1, Line::setB1, LineAdder::setB1);
        doubles(LINE_G2, LineCharacteristics::getG2, Line::setG2, LineAdder::setG2);
        doubles(LINE_B2, LineCharacteristics::getB2, Line::setB2, LineAdder::setB2);
        doubles(TWO_WT_R, TwoWindingsTransformer::getR, TwoWindingsTransformer::setR, TwoWindingsTransformerAdder::setR);
        doubles(TWO_WT_X, TwoWindingsTransformer::getX, TwoWindingsTransformer::setX, TwoWindingsTransformerAdder::setX);
        doubles(TWO_WT_G, TwoWindingsTransformer::getG, TwoWindingsTransformer::setG, TwoWindingsTransformerAdder::setG);
        doubles(TWO_WT_B, TwoWindingsTransformer::getB, TwoWindingsTransformer::setB, TwoWindingsTransformerAdder::setB);
        doubles(TWO_WT_RATED_U1, TwoWindingsTransformer::getRatedU1, TwoWindingsTransformer::setRatedU1,
            TwoWindingsTransformerAdder::setRatedU1);
        doubles(TWO_WT_RATED_U2, TwoWindingsTransformer::getRatedU2, TwoWindingsTransformer::setRatedU2,
            TwoWindingsTransformerAdder::setRatedU2);
        doubles(TWO_WT_RATED_S, TwoWindingsTransformer::getRatedS, TwoWindingsTransformer::setRatedS,
            TwoWindingsTransformerAdder::setRatedS);
        doubles(LEG_R, ThreeWindingsTransformer.Leg::getR, ThreeWindingsTransformer.Leg::setR, null);
        doubles(LEG_X, ThreeWindingsTransformer.Leg::getX, ThreeWindingsTransformer.Leg::setX, null);
        doubles(LEG_G, ThreeWindingsTransformer.Leg::getG, ThreeWindingsTransformer.Leg::setG, null);
        doubles(LEG_B, ThreeWindingsTransformer.Leg::getB, ThreeWindingsTransformer.Leg::setB, null);
        doubles(LEG_RATED_U, ThreeWindingsTransformer.Leg::getRatedU, ThreeWindingsTransformer.Leg::setRatedU, null);
        doubles(LEG_RATED_S, ThreeWindingsTransformer.Leg::getRatedS, ThreeWindingsTransformer.Leg::setRatedS, null);
        doubles(PTC_REGULATION_VALUE, PhaseTapChanger::getRegulationValue, PhaseTapChanger::setRegulationValue,
            PhaseTapChangerAdder::setRegulationValue);
        doubles(PTC_TARGET_DEADBAND, PhaseTapChanger::getTargetDeadband, PhaseTapChanger::setTargetDeadband,
            PhaseTapChangerAdder::setTargetDeadband);
        doubles(STEP_RHO, (TapChangerStep<?> x) -> x.getRho(), (TapChangerStep<?> x, double v) -> x.setRho(v),
            (TapChangerStepAdder<?, ?> x, double v) -> x.setRho(v));
        doubles(STEP_R, (TapChangerStep<?> x) -> x.getR(), (TapChangerStep<?> x, double v) -> x.setR(v),
            (TapChangerStepAdder<?, ?> x, double v) -> x.setR(v));
        doubles(STEP_X, (TapChangerStep<?> x) -> x.getX(), (TapChangerStep<?> x, double v) -> x.setX(v),
            (TapChangerStepAdder<?, ?> x, double v) -> x.setX(v));
        doubles(STEP_G, (TapChangerStep<?> x) -> x.getG(), (TapChangerStep<?> x, double v) -> x.setG(v),
            (TapChangerStepAdder<?, ?> x, double v) -> x.setG(v));
        doubles(STEP_B, (TapChangerStep<?> x) -> x.getB(), (TapChangerStep<?> x, double v) -> x.setB(v),
            (TapChangerStepAdder<?, ?> x, double v) -> x.setB(v));
        doubles(PTC_STEP_ALPHA, PhaseTapChangerStep::getAlpha, PhaseTapChangerStep::setAlpha,
            (PhaseTapChangerStepAdder<?, ?> x, double v) -> x.setAlpha(v));
        doubles(HVDC_R, HvdcLine::getR, HvdcLine::setR, HvdcLineAdder::setR);
        doubles(HVDC_NOMINAL_V, HvdcLine::getNominalV, HvdcLine::setNominalV, HvdcLineAdder::setNominalV);
        doubles(HVDC_ACTIVE_POWER_SETPOINT, HvdcLine::getActivePowerSetpoint, HvdcLine::setActivePowerSetpoint,
            HvdcLineAdder::setActivePowerSetpoint);
        doubles(HVDC_MAX_P, HvdcLine::getMaxP, HvdcLine::setMaxP, HvdcLineAdder::setMaxP);
        doubles(BL_P0, BoundaryLine::getP0, BoundaryLine::setP0, BoundaryLineAdder::setP0);
        doubles(BL_Q0, BoundaryLine::getQ0, BoundaryLine::setQ0, BoundaryLineAdder::setQ0);
        doubles(BL_R, BoundaryLine::getR, BoundaryLine::setR, BoundaryLineAdder::setR);
        doubles(BL_X, BoundaryLine::getX, BoundaryLine::setX, BoundaryLineAdder::setX);
        doubles(BL_G, BoundaryLine::getG, BoundaryLine::setG, BoundaryLineAdder::setG);
        doubles(BL_B, BoundaryLine::getB, BoundaryLine::setB, BoundaryLineAdder::setB);
        doubles(GENERATION_TARGET_P, BoundaryLine.Generation::getTargetP, BoundaryLine.Generation::setTargetP,
            BoundaryLineAdder.GenerationAdder::setTargetP);
        doubles(GENERATION_MIN_P, BoundaryLine.Generation::getMinP, BoundaryLine.Generation::setMinP,
            BoundaryLineAdder.GenerationAdder::setMinP);
        doubles(GENERATION_MAX_P, BoundaryLine.Generation::getMaxP, BoundaryLine.Generation::setMaxP,
            BoundaryLineAdder.GenerationAdder::setMaxP);
        doubles(GENERATION_TARGET_Q, BoundaryLine.Generation::getTargetQ, BoundaryLine.Generation::setTargetQ,
            BoundaryLineAdder.GenerationAdder::setTargetQ);
        doubles(GENERATION_TARGET_V, BoundaryLine.Generation::getTargetV, BoundaryLine.Generation::setTargetV,
            BoundaryLineAdder.GenerationAdder::setTargetV);
        doubles(BOUNDARY_V, Boundary::getV, null, null);
        doubles(BOUNDARY_ANGLE, Boundary::getAngle, null, null);
        doubles(BOUNDARY_P, Boundary::getP, null, null);
        doubles(BOUNDARY_Q, Boundary::getQ, null, null);
        doubles(BOUNDARY_I, Boundary::getI, null, null);
        doubles(SHUNT_B, (ShuntCompensator x) -> x.getB(), null, null);
        doubles(SHUNT_G, (ShuntCompensator x) -> x.getG(), null, null);
        doubles(SHUNT_B_PER_SECTION, ShuntCompensatorLinearModel::getBPerSection, ShuntCompensatorLinearModel::setBPerSection,
            ShuntCompensatorLinearModelAdder::setBPerSection);
        doubles(SHUNT_G_PER_SECTION, ShuntCompensatorLinearModel::getGPerSection, ShuntCompensatorLinearModel::setGPerSection,
            ShuntCompensatorLinearModelAdder::setGPerSection);
        doubles(SHUNT_SECTION_B, ShuntCompensatorNonLinearModel.Section::getB, ShuntCompensatorNonLinearModel.Section::setB,
            ShuntCompensatorNonLinearModelAdder.SectionAdder::setB);
        doubles(SHUNT_SECTION_G, ShuntCompensatorNonLinearModel.Section::getG, ShuntCompensatorNonLinearModel.Section::setG,
            ShuntCompensatorNonLinearModelAdder.SectionAdder::setG);
        doubles(SVC_B_MIN, StaticVarCompensator::getBmin, StaticVarCompensator::setBmin, StaticVarCompensatorAdder::setBmin);
        doubles(SVC_B_MAX, StaticVarCompensator::getBmax, StaticVarCompensator::setBmax, StaticVarCompensatorAdder::setBmax);
        doubles(CONVERTER_STATION_LOSS_FACTOR, (HvdcConverterStation<?> x) -> x.getLossFactor(),
            (HvdcConverterStation<?> x, double v) -> x.setLossFactor((float) v),
            (HvdcConverterStationAdder<?, ?> x, double v) -> x.setLossFactor((float) v));
        doubles(LCC_POWER_FACTOR, LccConverterStation::getPowerFactor, (LccConverterStation x, double v) -> x.setPowerFactor((float) v),
            (LccConverterStationAdder x, double v) -> x.setPowerFactor((float) v));
        doubles(EXT_APC_DROOP, (ActivePowerControl<?> x) -> x.getDroop(), (ActivePowerControl<?> x, double v) -> x.setDroop(v),
            (ActivePowerControlAdder<?> x, double v) -> x.withDroop(v));
        doubles(EXT_APC_PARTICIPATION_FACTOR, (ActivePowerControl<?> x) -> x.getParticipationFactor(),
            (ActivePowerControl<?> x, double v) -> x.setParticipationFactor(v),
            (ActivePowerControlAdder<?> x, double v) -> x.withParticipationFactor(v));
        doubles(EXT_APC_MIN_TARGET_P, (ActivePowerControl<?> x) -> x.getMinTargetP().orElse(Double.NaN),
            (ActivePowerControl<?> x, double v) -> x.setMinTargetP(v), (ActivePowerControlAdder<?> x, double v) -> x.withMinTargetP(v));
        doubles(EXT_APC_MAX_TARGET_P, (ActivePowerControl<?> x) -> x.getMaxTargetP().orElse(Double.NaN),
            (ActivePowerControl<?> x, double v) -> x.setMaxTargetP(v), (ActivePowerControlAdder<?> x, double v) -> x.withMaxTargetP(v));
        doubles(EXT_CRC_Q_PERCENT, CoordinatedReactiveControl::getQPercent, CoordinatedReactiveControl::setQPercent,
            CoordinatedReactiveControlAdder::withQPercent);
        doubles(EXT_HADAPC_DROOP, HvdcAngleDroopActivePowerControl::getDroop,
            (HvdcAngleDroopActivePowerControl x, double v) -> x.setDroop((float) v),
            (HvdcAngleDroopActivePowerControlAdder x, double v) -> x.withDroop((float) v));
        doubles(EXT_HADAPC_P0, HvdcAngleDroopActivePowerControl::getP0,
            (HvdcAngleDroopActivePowerControl x, double v) -> x.setP0((float) v),
            (HvdcAngleDroopActivePowerControlAdder x, double v) -> x.withP0((float) v));
        doubles(EXT_HOAR_OPR_CS1_TO_CS2, HvdcOperatorActivePowerRange::getOprFromCS1toCS2,
            (HvdcOperatorActivePowerRange x, double v) -> x.setOprFromCS1toCS2((float) v),
            (HvdcOperatorActivePowerRangeAdder x, double v) -> x.withOprFromCS1toCS2((float) v));
        doubles(EXT_HOAR_OPR_CS2_TO_CS1, HvdcOperatorActivePowerRange::getOprFromCS2toCS1,
            (HvdcOperatorActivePowerRange x, double v) -> x.setOprFromCS2toCS1((float) v),
            (HvdcOperatorActivePowerRangeAdder x, double v) -> x.withOprFromCS2toCS1((float) v));
        doubles(BBS_V, BusbarSection::getV, null, null);
        doubles(BBS_ANGLE, BusbarSection::getAngle, null, null);
        doubles(BAT_TARGET_P, Battery::getTargetP, Battery::setTargetP, BatteryAdder::setTargetP);
        doubles(BAT_MIN_P, Battery::getMinP, Battery::setMinP, BatteryAdder::setMinP);
        doubles(BAT_MAX_P, Battery::getMaxP, Battery::setMaxP, BatteryAdder::setMaxP);
        doubles(RL_MIN_Q, (MinMaxReactiveLimits x) -> x.getMinQ(), null, (MinMaxReactiveLimitsAdder x, double v) -> x.setMinQ(v));
        doubles(RL_MAX_Q, (MinMaxReactiveLimits x) -> x.getMaxQ(), null, (MinMaxReactiveLimitsAdder x, double v) -> x.setMaxQ(v));
        doubles(CURVE_MIN_P, ReactiveCapabilityCurve::getMinP, null, null);
        doubles(CURVE_MAX_P, ReactiveCapabilityCurve::getMaxP, null, null);
        doubles(PLANE_ALPHA, ReactiveCapabilityShapePlane::getAlpha, null, null);
        doubles(PLANE_BETA, ReactiveCapabilityShapePlane::getBeta, null, null);
        doubles(PLANE_GAMMA, ReactiveCapabilityShapePlane::getGamma, null, null);
        doubles(PLANE_DELTA, ReactiveCapabilityShapePlane::getDelta, null, null);
        doubles(POINT_P, ReactiveCapabilityCurve.Point::getP, null, ReactiveCapabilityCurveAdder.PointAdder::setP);
        doubles(POINT_MIN_Q, ReactiveCapabilityCurve.Point::getMinQ, null, ReactiveCapabilityCurveAdder.PointAdder::setMinQ);
        doubles(POINT_MAX_Q, ReactiveCapabilityCurve.Point::getMaxQ, null, ReactiveCapabilityCurveAdder.PointAdder::setMaxQ);
        doubles(LL_PERMANENT_LIMIT, LoadingLimits::getPermanentLimit, LoadingLimits::setPermanentLimit,
            (LoadingLimitsAdder<?, ?> x, double v) -> x.setPermanentLimit(v));
        doubles(TL_VALUE, TemporaryLimit::getValue, null, (LoadingLimitsAdder.TemporaryLimitAdder<?> x, double v) -> x.setValue(v));
        doubles(AREA_INTERCHANGE_TARGET, (Area x) -> x.getInterchangeTarget().orElse(Double.NaN), Area::setInterchangeTarget,
            AreaAdder::setInterchangeTarget);
        doubles(AREA_AC_INTERCHANGE, Area::getAcInterchange, null, null);
        doubles(AREA_DC_INTERCHANGE, Area::getDcInterchange, null, null);
        doubles(AREA_INTERCHANGE, Area::getInterchange, null, null);
        doubles(AREA_BOUNDARY_P, AreaBoundary::getP, null, null);
        doubles(AREA_BOUNDARY_Q, AreaBoundary::getQ, null, null);
        doubles(VAL_LOW_LIMIT, (VoltageAngleLimit x) -> x.getLowLimit().orElse(Double.NaN), null, VoltageAngleLimitAdder::setLowLimit);
        doubles(VAL_HIGH_LIMIT, (VoltageAngleLimit x) -> x.getHighLimit().orElse(Double.NaN), null, VoltageAngleLimitAdder::setHighLimit);
        doubles(TRIPPING_CURRENT_LIMIT, OverloadManagementSystem.Tripping::getCurrentLimit,
            OverloadManagementSystem.Tripping::setCurrentLimit,
            (OverloadManagementSystemAdder.TrippingAdder<?> x, double v) -> x.setCurrentLimit(v));
        doubles(OVERLOAD_PREVIOUS_LIMIT, Overload::getPreviousLimit, null, null);
        doubles(OVERLOAD_LIMIT_SCALING, Overload::getLimitScalingCoefficient, null, null);
        doubles(THREE_WT_RATED_U0, ThreeWindingsTransformer::getRatedU0, ThreeWindingsTransformer::setRatedU0,
            ThreeWindingsTransformerAdder::setRatedU0);
        doubles(OBS_QUALITY_STANDARD_DEVIATION, (ObservabilityQuality<?> x) -> x.getStandardDeviation(),
            (ObservabilityQuality<?> x, double v) -> x.setStandardDeviation(v), null);
        doubles(MEAS_VALUE, Measurement::getValue, Measurement::setValue, MeasurementAdder::setValue);
        doubles(MEAS_STD_DEV, Measurement::getStandardDeviation, Measurement::setStandardDeviation, MeasurementAdder::setStandardDeviation);
        doubles(EXT_SC_DIRECT_SUBTRANS_X, (ShortCircuitExtension<?> x) -> x.getDirectSubtransX(),
            (ShortCircuitExtension<?> x, double v) -> x.setDirectSubtransX(v),
            (ShortCircuitExtensionAdder<?, ?, ?> x, double v) -> x.withDirectSubtransX(v));
        doubles(EXT_SC_DIRECT_TRANS_X, (ShortCircuitExtension<?> x) -> x.getDirectTransX(),
            (ShortCircuitExtension<?> x, double v) -> x.setDirectTransX(v),
            (ShortCircuitExtensionAdder<?, ?, ?> x, double v) -> x.withDirectTransX(v));
        doubles(EXT_SC_STEP_UP_TRANSFORMER_X, (ShortCircuitExtension<?> x) -> x.getStepUpTransformerX(),
            (ShortCircuitExtension<?> x, double v) -> x.setStepUpTransformerX(v),
            (ShortCircuitExtensionAdder<?, ?, ?> x, double v) -> x.withStepUpTransformerX(v));
        doubles(EXT_ISC_IP_MIN, (IdentifiableShortCircuit<?> x) -> x.getIpMin(), (IdentifiableShortCircuit<?> x, double v) -> x.setIpMin(v),
            (IdentifiableShortCircuitAdder<?> x, double v) -> x.withIpMin(v));
        doubles(EXT_ISC_IP_MAX, (IdentifiableShortCircuit<?> x) -> x.getIpMax(), (IdentifiableShortCircuit<?> x, double v) -> x.setIpMax(v),
            (IdentifiableShortCircuitAdder<?> x, double v) -> x.withIpMax(v));
        doubles(EXT_GS_PLANNED_ACTIVE_POWER_SETPOINT, GeneratorStartup::getPlannedActivePowerSetpoint,
            GeneratorStartup::setPlannedActivePowerSetpoint, GeneratorStartupAdder::withPlannedActivePowerSetpoint);
        doubles(EXT_GS_STARTUP_COST, GeneratorStartup::getStartupCost, GeneratorStartup::setStartupCost,
            GeneratorStartupAdder::withStartupCost);
        doubles(EXT_GS_MARGINAL_COST, GeneratorStartup::getMarginalCost, GeneratorStartup::setMarginalCost,
            GeneratorStartupAdder::withMarginalCost);
        doubles(EXT_GS_PLANNED_OUTAGE_RATE, GeneratorStartup::getPlannedOutageRate, GeneratorStartup::setPlannedOutageRate,
            GeneratorStartupAdder::withPlannedOutageRate);
        doubles(EXT_GS_FORCED_OUTAGE_RATE, GeneratorStartup::getForcedOutageRate, GeneratorStartup::setForcedOutageRate,
            GeneratorStartupAdder::withForcedOutageRate);
        doubles(EXT_LD_FIXED_ACTIVE_POWER, LoadDetail::getFixedActivePower, LoadDetail::setFixedActivePower,
            LoadDetailAdder::withFixedActivePower);
        doubles(EXT_LD_FIXED_REACTIVE_POWER, LoadDetail::getFixedReactivePower, LoadDetail::setFixedReactivePower,
            LoadDetailAdder::withFixedReactivePower);
        doubles(EXT_LD_VARIABLE_ACTIVE_POWER, LoadDetail::getVariableActivePower, LoadDetail::setVariableActivePower,
            LoadDetailAdder::withVariableActivePower);
        doubles(EXT_LD_VARIABLE_REACTIVE_POWER, LoadDetail::getVariableReactivePower, LoadDetail::setVariableReactivePower,
            LoadDetailAdder::withVariableReactivePower);
        doubles(EXT_SA_B0, StandbyAutomaton::getB0, StandbyAutomaton::setB0, StandbyAutomatonAdder::withB0);
        doubles(EXT_SA_LOW_VOLTAGE_SETPOINT, StandbyAutomaton::getLowVoltageSetpoint, StandbyAutomaton::setLowVoltageSetpoint,
            StandbyAutomatonAdder::withLowVoltageSetpoint);
        doubles(EXT_SA_HIGH_VOLTAGE_SETPOINT, StandbyAutomaton::getHighVoltageSetpoint, StandbyAutomaton::setHighVoltageSetpoint,
            StandbyAutomatonAdder::withHighVoltageSetpoint);
        doubles(EXT_SA_LOW_VOLTAGE_THRESHOLD, StandbyAutomaton::getLowVoltageThreshold, StandbyAutomaton::setLowVoltageThreshold,
            StandbyAutomatonAdder::withLowVoltageThreshold);
        doubles(EXT_SA_HIGH_VOLTAGE_THRESHOLD, StandbyAutomaton::getHighVoltageThreshold, StandbyAutomaton::setHighVoltageThreshold,
            StandbyAutomatonAdder::withHighVoltageThreshold);
        doubles(EXT_GF_RZ, GeneratorFortescue::getRz, GeneratorFortescue::setRz, GeneratorFortescueAdder::withRz);
        doubles(EXT_GF_XZ, GeneratorFortescue::getXz, GeneratorFortescue::setXz, GeneratorFortescueAdder::withXz);
        doubles(EXT_GF_RN, GeneratorFortescue::getRn, GeneratorFortescue::setRn, GeneratorFortescueAdder::withRn);
        doubles(EXT_GF_XN, GeneratorFortescue::getXn, GeneratorFortescue::setXn, GeneratorFortescueAdder::withXn);
        doubles(EXT_GF_GROUNDING_R, GeneratorFortescue::getGroundingR, GeneratorFortescue::setGroundingR,
            GeneratorFortescueAdder::withGroundingR);
        doubles(EXT_GF_GROUNDING_X, GeneratorFortescue::getGroundingX, GeneratorFortescue::setGroundingX,
            GeneratorFortescueAdder::withGroundingX);
        doubles(EXT_LINE_F_RZ, LineFortescue::getRz, LineFortescue::setRz, LineFortescueAdder::withRz);
        doubles(EXT_LINE_F_XZ, LineFortescue::getXz, LineFortescue::setXz, LineFortescueAdder::withXz);
        doubles(EXT_LINE_F_G1Z, LineFortescue::getG1z, LineFortescue::setG1z, LineFortescueAdder::withG1z);
        doubles(EXT_LINE_F_B1Z, LineFortescue::getB1z, LineFortescue::setB1z, LineFortescueAdder::withB1z);
        doubles(EXT_LINE_F_G2Z, LineFortescue::getG2z, LineFortescue::setG2z, LineFortescueAdder::withG2z);
        doubles(EXT_LINE_F_B2Z, LineFortescue::getB2z, LineFortescue::setB2z, LineFortescueAdder::withB2z);
        doubles(EXT_LA_DELTA_PA, LoadAsymmetrical::getDeltaPa, LoadAsymmetrical::setDeltaPa, LoadAsymmetricalAdder::withDeltaPa);
        doubles(EXT_LA_DELTA_QA, LoadAsymmetrical::getDeltaQa, LoadAsymmetrical::setDeltaQa, LoadAsymmetricalAdder::withDeltaQa);
        doubles(EXT_LA_DELTA_PB, LoadAsymmetrical::getDeltaPb, LoadAsymmetrical::setDeltaPb, LoadAsymmetricalAdder::withDeltaPb);
        doubles(EXT_LA_DELTA_QB, LoadAsymmetrical::getDeltaQb, LoadAsymmetrical::setDeltaQb, LoadAsymmetricalAdder::withDeltaQb);
        doubles(EXT_LA_DELTA_PC, LoadAsymmetrical::getDeltaPc, LoadAsymmetrical::setDeltaPc, LoadAsymmetricalAdder::withDeltaPc);
        doubles(EXT_LA_DELTA_QC, LoadAsymmetrical::getDeltaQc, LoadAsymmetrical::setDeltaQc, LoadAsymmetricalAdder::withDeltaQc);
        doubles(EXT_TWO_WTF_RZ, TwoWindingsTransformerFortescue::getRz, TwoWindingsTransformerFortescue::setRz,
            TwoWindingsTransformerFortescueAdder::withRz);
        doubles(EXT_TWO_WTF_XZ, TwoWindingsTransformerFortescue::getXz, TwoWindingsTransformerFortescue::setXz,
            TwoWindingsTransformerFortescueAdder::withXz);
        doubles(EXT_TWO_WTF_XM, TwoWindingsTransformerFortescue::getXm, TwoWindingsTransformerFortescue::setXm,
            TwoWindingsTransformerFortescueAdder::withXm);
        doubles(EXT_TWO_WTF_GROUNDING_R1, TwoWindingsTransformerFortescue::getGroundingR1, TwoWindingsTransformerFortescue::setGroundingR1,
            TwoWindingsTransformerFortescueAdder::withGroundingR1);
        doubles(EXT_TWO_WTF_GROUNDING_X1, TwoWindingsTransformerFortescue::getGroundingX1, TwoWindingsTransformerFortescue::setGroundingX1,
            TwoWindingsTransformerFortescueAdder::withGroundingX1);
        doubles(EXT_TWO_WTF_GROUNDING_R2, TwoWindingsTransformerFortescue::getGroundingR2, TwoWindingsTransformerFortescue::setGroundingR2,
            TwoWindingsTransformerFortescueAdder::withGroundingR2);
        doubles(EXT_TWO_WTF_GROUNDING_X2, TwoWindingsTransformerFortescue::getGroundingX2, TwoWindingsTransformerFortescue::setGroundingX2,
            TwoWindingsTransformerFortescueAdder::withGroundingX2);
        doubles(EXT_LEG_F_RZ, LegFortescue::getRz, LegFortescue::setRz, null);
        doubles(EXT_LEG_F_XZ, LegFortescue::getXz, LegFortescue::setXz, null);
        doubles(EXT_LEG_F_GROUNDING_R, LegFortescue::getGroundingR, LegFortescue::setGroundingR, null);
        doubles(EXT_LEG_F_GROUNDING_X, LegFortescue::getGroundingX, LegFortescue::setGroundingX, null);
        doubles(COORD_LATITUDE, Coordinate::getLatitude, null, null);
        doubles(COORD_LONGITUDE, Coordinate::getLongitude, null, null);
        doubles(EXT_SVC_TARGET_V, PilotPoint::getTargetV, PilotPoint::setTargetV, null);
        doubles(EXT_MC_R, MutualCoupling::getR, MutualCoupling::setR, MutualCouplingAdder::withR);
        doubles(EXT_MC_X, MutualCoupling::getX, MutualCoupling::setX, MutualCouplingAdder::withX);
        doubles(EXT_MC_SEGMENT_START, LineSegment::start, null, null);
        doubles(EXT_MC_SEGMENT_END, LineSegment::end, null, null);
        doubles(OBS_QUALITY_P, null, (InjectionObservability<?> x, double v) -> x.setQualityP(v),
            (InjectionObservabilityAdder<?> x, double v) -> x.withStandardDeviationP(v));
        doubles(OBS_QUALITY_Q, null, (InjectionObservability<?> x, double v) -> x.setQualityQ(v),
            (InjectionObservabilityAdder<?> x, double v) -> x.withStandardDeviationQ(v));
        doubles(OBS_QUALITY_V, null, (InjectionObservability<?> x, double v) -> x.setQualityV(v),
            (InjectionObservabilityAdder<?> x, double v) -> x.withStandardDeviationV(v));
        doubles(OBS_QUALITY_P1, null, (BranchObservability<?> x, double v) -> x.setQualityP1(v),
            (BranchObservabilityAdder<?> x, double v) -> x.withStandardDeviationP1(v));
        doubles(OBS_QUALITY_P2, null, (BranchObservability<?> x, double v) -> x.setQualityP2(v),
            (BranchObservabilityAdder<?> x, double v) -> x.withStandardDeviationP2(v));
        doubles(OBS_QUALITY_Q1, null, (BranchObservability<?> x, double v) -> x.setQualityQ1(v),
            (BranchObservabilityAdder<?> x, double v) -> x.withStandardDeviationQ1(v));
        doubles(OBS_QUALITY_Q2, null, (BranchObservability<?> x, double v) -> x.setQualityQ2(v),
            (BranchObservabilityAdder<?> x, double v) -> x.withStandardDeviationQ2(v));
        doubles(PLANE_LESS_OR_EQUAL, null, ReactiveCapabilityShapePlane::lessOrEqual, null);
        doubles(PLANE_GREATER_OR_EQUAL, null, ReactiveCapabilityShapePlane::greaterOrEqual, null);

        // ── Ints ────────────────────────────────────────────────────────────────────────────────────
        ints(NETWORK_FORECAST_DISTANCE, Network::getForecastDistance, Network::setForecastDistance, null);
        // getSectionCount() fails when undefined
        ints(SHUNT_SECTION_COUNT, ShuntCompensator::getSectionCount, ShuntCompensator::setSectionCount,
            ShuntCompensatorAdder::setSectionCount);
        ints(SHUNT_MAX_SECTION_COUNT, ShuntCompensator::getMaximumSectionCount, ShuntCompensatorLinearModel::setMaximumSectionCount,
            ShuntCompensatorLinearModelAdder::setMaximumSectionCount);
        ints(IC_NODE1, VoltageLevel.NodeBreakerView.InternalConnection::getNode1, null,
            VoltageLevel.NodeBreakerView.InternalConnectionAdder::setNode1);
        ints(IC_NODE2, VoltageLevel.NodeBreakerView.InternalConnection::getNode2, null,
            VoltageLevel.NodeBreakerView.InternalConnectionAdder::setNode2);
        ints(TL_ACCEPTABLE_DURATION, TemporaryLimit::getAcceptableDuration, null,
            (LoadingLimitsAdder.TemporaryLimitAdder<?> x, int v) -> x.setAcceptableDuration(v));
        ints(COMPONENT_NUM, Component::getNum, null, null);
        ints(COMPONENT_SIZE, Component::getSize, null, null);
        ints(TC_LOW_TAP_POSITION, (TapChanger<?, ?, ?, ?> x) -> x.getLowTapPosition(),
            (TapChanger<?, ?, ?, ?> x, int v) -> x.setLowTapPosition(v),
            (TapChangerAdder<?, ?, ?, ?, ?, ?> x, int v) -> x.setLowTapPosition(v));
        ints(TC_HIGH_TAP_POSITION, (TapChanger<?, ?, ?, ?> x) -> x.getHighTapPosition(), null, null);
        // getTapPosition() fails when unset
        ints(TC_TAP_POSITION, (TapChanger<?, ?, ?, ?> x) -> x.getTapPosition(), (TapChanger<?, ?, ?, ?> x, int v) -> x.setTapPosition(v),
            (TapChangerAdder<?, ?, ?, ?, ?, ?> x, int v) -> x.setTapPosition(v));
        ints(TC_STEP_COUNT, (TapChanger<?, ?, ?, ?> x) -> x.getStepCount(), null, null);
        ints(CURVE_POINT_COUNT, ReactiveCapabilityCurve::getPointCount, null, null);
        ints(DMEAS_VALUE_INT, (DiscreteMeasurement x) -> x.getValueAsInt(), (DiscreteMeasurement x, int v) -> x.setValue(v),
            (DiscreteMeasurementAdder x, int v) -> x.setValue(v));
        ints(EXT_GEC_CODE, GeneratorEntsoeCategory::getCode, GeneratorEntsoeCategory::setCode, GeneratorEntsoeCategoryAdder::withCode);
        ints(EXT_TWO_WT_PAC_CLOCK, TwoWindingsTransformerPhaseAngleClock::getPhaseAngleClock,
            TwoWindingsTransformerPhaseAngleClock::setPhaseAngleClock, TwoWindingsTransformerPhaseAngleClockAdder::withPhaseAngleClock);
        ints(EXT_THREE_WT_PAC_CLOCK_LEG2, ThreeWindingsTransformerPhaseAngleClock::getPhaseAngleClockLeg2,
            ThreeWindingsTransformerPhaseAngleClock::setPhaseAngleClockLeg2,
            ThreeWindingsTransformerPhaseAngleClockAdder::withPhaseAngleClockLeg2);
        ints(EXT_THREE_WT_PAC_CLOCK_LEG3, ThreeWindingsTransformerPhaseAngleClock::getPhaseAngleClockLeg3,
            ThreeWindingsTransformerPhaseAngleClock::setPhaseAngleClockLeg3,
            ThreeWindingsTransformerPhaseAngleClockAdder::withPhaseAngleClockLeg3);
        ints(EXT_BSP_BUSBAR_INDEX, BusbarSectionPosition::getBusbarIndex, BusbarSectionPosition::setBusbarIndex,
            BusbarSectionPositionAdder::withBusbarIndex);
        ints(EXT_BSP_SECTION_INDEX, BusbarSectionPosition::getSectionIndex, BusbarSectionPosition::setSectionIndex,
            BusbarSectionPositionAdder::withSectionIndex);
        ints(EXT_RP_PRIORITY, ReferencePriority::getPriority, null, ReferencePriorityAdder::setPriority);
        ints(SHUNT_SOLVED_SECTION_COUNT, null, ShuntCompensator::setSolvedSectionCount, ShuntCompensatorAdder::setSolvedSectionCount);
        ints(TC_SOLVED_TAP_POSITION, null, (TapChanger<?, ?, ?, ?> x, int v) -> x.setSolvedTapPosition(v),
            (TapChangerAdder<?, ?, ?, ?, ?, ?> x, int v) -> x.setSolvedTapPosition(v));
        ints(EXT_FEEDER_ORDER, null, ConnectablePosition.Feeder::setOrder, null);
        ints(SW_NODE1, null, null, VoltageLevel.NodeBreakerView.SwitchAdder::setNode1);
        ints(SW_NODE2, null, null, VoltageLevel.NodeBreakerView.SwitchAdder::setNode2);
        ints(BRANCH_NODE1, null, null, (BranchAdder<?, ?> x, int v) -> x.setNode1(v));
        ints(BRANCH_NODE2, null, null, (BranchAdder<?, ?> x, int v) -> x.setNode2(v));
        ints(EXT_THREE_WTF_LEG, null, null, Creation.FortescueLegs::select);

        // ── Enums, as their ordinal ─────────────────────────────────────────────────────────────────
        enums(GEN_ENERGY_SOURCE, EnergySource.class, Generator::getEnergySource, Generator::setEnergySource,
            GeneratorAdder::setEnergySource);
        enums(LOAD_TYPE, LoadType.class, Load::getLoadType, Load::setLoadType, LoadAdder::setLoadType);
        enums(VL_TOPOLOGY_KIND, TopologyKind.class, (VoltageLevel x) -> x.getTopologyKind(),
            (VoltageLevel x, TopologyKind v) -> x.convertToTopology(v), (VoltageLevelAdder x, TopologyKind v) -> x.setTopologyKind(v));
        enums(HVDC_CONVERTERS_MODE, HvdcLine.ConvertersMode.class, HvdcLine::getConvertersMode, null, HvdcLineAdder::setConvertersMode);
        enums(SW_KIND, SwitchKind.class, (Switch x) -> x.getKind(), null,
            (VoltageLevel.NodeBreakerView.SwitchAdder x, SwitchKind v) -> x.setKind(v));
        enums(OMS_MONITORED_SIDE, ThreeSides.class, OverloadManagementSystem::getMonitoredSide, null,
            OverloadManagementSystemAdder::setMonitoredElementSide);
        enums(LL_DETECTION_KIND, DetectionKind.class, LoadingLimits::getDetectionKind, null,
            (LoadingLimitsAdder<?, ?> x, DetectionKind v) -> x.setDetectionKind(v));
        enums(MEAS_TYPE, Measurement.Type.class, Measurement::getType, null, MeasurementAdder::setType);
        enums(DMEAS_TYPE, DiscreteMeasurement.Type.class, DiscreteMeasurement::getType, null, DiscreteMeasurementAdder::setType);
        enums(EXT_OS_STATUS, OperatingStatus.Status.class, (OperatingStatus<?> x) -> x.getStatus(),
            (OperatingStatus<?> x, OperatingStatus.Status v) -> x.setStatus(v),
            (OperatingStatusAdder<?> x, OperatingStatus.Status v) -> x.withStatus(v));
        enums(EXT_LA_CONNECTION_TYPE, LoadConnectionType.class, LoadAsymmetrical::getConnectionType, LoadAsymmetrical::setConnectionType,
            LoadAsymmetricalAdder::withConnectionType);
        enums(EXT_LEG_F_CONNECTION_TYPE, WindingConnectionType.class, LegFortescue::getConnectionType, LegFortescue::setConnectionType,
            null);

        // ── Read-only enums, as their ordinal ───────────────────────────────────────────────────────
        ordinals(IDENTIFIABLE_TYPE, (Identifiable<?> x) -> x.getType());
        ordinals(CONTAINER_TYPE, (Container<?> x) -> x.getContainerType());
        ordinals(NETWORK_VALIDATION_LEVEL, Network::getValidationLevel);
        ordinals(CONVERTER_STATION_HVDC_TYPE, (HvdcConverterStation<?> x) -> x.getHvdcType());
        ordinals(LOAD_MODEL_TYPE, LoadModel::getType);
        ordinals(SHUNT_MODEL_TYPE, ShuntCompensator::getModelType);
        ordinals(TRIPPING_TYPE, OverloadManagementSystem.Tripping::getType);
        ordinals(LL_LIMIT_TYPE, OperationalLimits::getLimitType);
        ordinals(PTC_REGULATION_MODE, PhaseTapChanger::getRegulationMode);
        ordinals(LEG_SIDE, ThreeWindingsTransformer.Leg::getSide);
        ordinals(RL_KIND, ReactiveLimits::getKind);
        ordinals(DMEAS_VALUE_TYPE, DiscreteMeasurement::getValueType);
        ordinals(EXT_TWO_WTF_CONNECTION_TYPE1, TwoWindingsTransformerFortescue::getConnectionType1);
        ordinals(EXT_TWO_WTF_CONNECTION_TYPE2, TwoWindingsTransformerFortescue::getConnectionType2);
        ordinals(EXT_FEEDER_DIRECTION, ConnectablePosition.Feeder::getDirection);

        // ── Booleans ────────────────────────────────────────────────────────────────────────────────
        bools(AREA_BOUNDARY_AC, AreaBoundary::isAc, null, AreaBoundaryAdder::setAc);
        bools(VARIANT_MULTI_THREAD_ACCESS, VariantManager::isVariantMultiThreadAccessAllowed, VariantManager::allowVariantMultiThreadAccess,
            null);
        bools(NETWORK_DETACHABLE, Network::isDetachable, null, null);
        bools(AUTOMATION_SYSTEM_ENABLED, (AutomationSystem<?> x) -> x.isEnabled(), (AutomationSystem<?> x, boolean v) -> x.setEnabled(v),
            OverloadManagementSystemAdder::setEnabled);
        bools(TRIPPING_OPEN_ACTION, OverloadManagementSystem.Tripping::isOpenAction, OverloadManagementSystem.Tripping::setOpenAction,
            (OverloadManagementSystemAdder.TrippingAdder<?> x, boolean v) -> x.setOpenAction(v));
        bools(BL_PAIRED, BoundaryLine::isPaired, null, null);
        bools(GENERATION_VOLTAGE_REGULATION_ON, BoundaryLine.Generation::isVoltageRegulationOn,
            BoundaryLine.Generation::setVoltageRegulationOn, BoundaryLineAdder.GenerationAdder::setVoltageRegulationOn);
        bools(PLANE_LESS_OR_EQUAL, ReactiveCapabilityShapePlane::isLessOrEqual, null, null);
        bools(PLANE_GREATER_OR_EQUAL, ReactiveCapabilityShapePlane::isGreaterOrEqual, null, null);
        bools(FICTITIOUS, (Identifiable<?> x) -> x.isFictitious(), (Identifiable<?> x, boolean v) -> x.setFictitious(v),
            (IdentifiableAdder<?, ?> x, boolean v) -> x.setFictitious(v));
        bools(EQUIVALENT, (Connectable<?> x) -> x.isEquivalent(), (Connectable<?> x, boolean v) -> x.setEquivalent(v),
            (ConnectableAdder<?, ?> x, boolean v) -> x.setEquivalent(v));
        bools(GEN_CONDENSER, Generator::isCondenser, null, GeneratorAdder::setCondenser);
        bools(TC_LOAD_TAP_CHANGING_CAPABILITIES, (TapChanger<?, ?, ?, ?> x) -> x.hasLoadTapChangingCapabilities(),
            (TapChanger<?, ?, ?, ?> x, boolean v) -> x.setLoadTapChangingCapabilities(v),
            (TapChangerAdder<?, ?, ?, ?, ?, ?> x, boolean v) -> x.setLoadTapChangingCapabilities(v));
        bools(HAS_REGULATING_TERMINAL, (VoltageRegulationHolder<?> x) -> x.hasRegulatingTerminal(), null, null);
        bools(VR_REGULATING, VoltageRegulation::isRegulating, VoltageRegulation::setRegulating,
            (VoltageRegulationAdderOrBuilder<?> x, boolean v) -> x.withRegulating(v));
        bools(VR_WITH_TERMINAL, VoltageRegulation::isWithTerminal, null, null);
        bools(TERMINAL_CONNECTED, Terminal::isConnected, null, null);
        bools(BUS_IN_MAIN_CONNECTED_COMPONENT, Bus::isInMainConnectedComponent, null, null);
        bools(BUS_IN_MAIN_SYNCHRONOUS_COMPONENT, Bus::isInMainSynchronousComponent, null, null);
        bools(TL_FICTITIOUS, TemporaryLimit::isFictitious, null,
            (LoadingLimitsAdder.TemporaryLimitAdder<?> x, boolean v) -> x.setFictitious(v));
        bools(SW_OPEN, Switch::isOpen, Switch::setOpen, null);
        bools(SW_RETAINED, Switch::isRetained, Switch::setRetained, VoltageLevel.NodeBreakerView.SwitchAdder::setRetained);
        bools(EXT_APC_PARTICIPATE, (ActivePowerControl<?> x) -> x.isParticipate(),
            (ActivePowerControl<?> x, boolean v) -> x.setParticipate(v), (ActivePowerControlAdder<?> x, boolean v) -> x.withParticipate(v));
        bools(EXT_HADAPC_ENABLED, HvdcAngleDroopActivePowerControl::isEnabled, HvdcAngleDroopActivePowerControl::setEnabled,
            HvdcAngleDroopActivePowerControlAdder::withEnabled);
        bools(EXT_ST_EMPTY, SlackTerminal::isEmpty, null, null);
        bools(EXT_TWO_WT_TBE_RTC, (TwoWindingsTransformerToBeEstimated x) -> x.shouldEstimateRatioTapChanger(),
            (TwoWindingsTransformerToBeEstimated x, boolean v) -> x.shouldEstimateRatioTapChanger(v),
            (TwoWindingsTransformerToBeEstimatedAdder x, boolean v) -> x.withRatioTapChangerStatus(v));
        bools(EXT_TWO_WT_TBE_PTC, (TwoWindingsTransformerToBeEstimated x) -> x.shouldEstimatePhaseTapChanger(),
            (TwoWindingsTransformerToBeEstimated x, boolean v) -> x.shouldEstimatePhaseTapChanger(v),
            (TwoWindingsTransformerToBeEstimatedAdder x, boolean v) -> x.withPhaseTapChangerStatus(v));
        bools(EXT_THREE_WT_TBE_RTC1, (ThreeWindingsTransformerToBeEstimated x) -> x.shouldEstimateRatioTapChanger1(),
            (ThreeWindingsTransformerToBeEstimated x, boolean v) -> x.shouldEstimateRatioTapChanger1(v),
            (ThreeWindingsTransformerToBeEstimatedAdder x, boolean v) -> x.withRatioTapChanger1Status(v));
        bools(EXT_THREE_WT_TBE_RTC2, (ThreeWindingsTransformerToBeEstimated x) -> x.shouldEstimateRatioTapChanger2(),
            (ThreeWindingsTransformerToBeEstimated x, boolean v) -> x.shouldEstimateRatioTapChanger2(v),
            (ThreeWindingsTransformerToBeEstimatedAdder x, boolean v) -> x.withRatioTapChanger2Status(v));
        bools(EXT_THREE_WT_TBE_RTC3, (ThreeWindingsTransformerToBeEstimated x) -> x.shouldEstimateRatioTapChanger3(),
            (ThreeWindingsTransformerToBeEstimated x, boolean v) -> x.shouldEstimateRatioTapChanger3(v),
            (ThreeWindingsTransformerToBeEstimatedAdder x, boolean v) -> x.withRatioTapChanger3Status(v));
        bools(EXT_THREE_WT_TBE_PTC1, (ThreeWindingsTransformerToBeEstimated x) -> x.shouldEstimatePhaseTapChanger1(),
            (ThreeWindingsTransformerToBeEstimated x, boolean v) -> x.shouldEstimatePhaseTapChanger1(v),
            (ThreeWindingsTransformerToBeEstimatedAdder x, boolean v) -> x.withPhaseTapChanger1Status(v));
        bools(EXT_THREE_WT_TBE_PTC2, (ThreeWindingsTransformerToBeEstimated x) -> x.shouldEstimatePhaseTapChanger2(),
            (ThreeWindingsTransformerToBeEstimated x, boolean v) -> x.shouldEstimatePhaseTapChanger2(v),
            (ThreeWindingsTransformerToBeEstimatedAdder x, boolean v) -> x.withPhaseTapChanger2Status(v));
        bools(EXT_THREE_WT_TBE_PTC3, (ThreeWindingsTransformerToBeEstimated x) -> x.shouldEstimatePhaseTapChanger3(),
            (ThreeWindingsTransformerToBeEstimated x, boolean v) -> x.shouldEstimatePhaseTapChanger3(v),
            (ThreeWindingsTransformerToBeEstimatedAdder x, boolean v) -> x.withPhaseTapChanger3Status(v));
        bools(EXT_OBS_OBSERVABLE, (Observability<?> x) -> x.isObservable(), (Observability<?> x, boolean v) -> x.setObservable(v), null);
        bools(OLG_EMPTY, OperationalLimitsGroup::isEmpty, null, null);
        bools(MEAS_VALID, Measurement::isValid, Measurement::setValid, MeasurementAdder::setValid);
        bools(DMEAS_VALUE_BOOL, (DiscreteMeasurement x) -> x.getValueAsBoolean(), (DiscreteMeasurement x, boolean v) -> x.setValue(v),
            (DiscreteMeasurementAdder x, boolean v) -> x.setValue(v));
        bools(DMEAS_VALID, DiscreteMeasurement::isValid, DiscreteMeasurement::setValid, DiscreteMeasurementAdder::setValid);
        bools(EXT_OBS_AREA_CONSISTENT, ObservabilityArea::isConsistentWithTopology, null, null);
        bools(EXT_SA_STANDBY, StandbyAutomaton::isStandby, StandbyAutomaton::setStandby, StandbyAutomatonAdder::withStandbyStatus);
        bools(EXT_MFRR_PARTICIPATE, (ManualFrequencyRestorationReserve<?> x) -> x.isParticipate(),
            (ManualFrequencyRestorationReserve<?> x, boolean v) -> x.setParticipate(v),
            (ManualFrequencyRestorationReserveAdder<?> x, boolean v) -> x.withParticipate(v));
        bools(EXT_SVC_UNIT_PARTICIPATE, ControlUnit::isParticipate, ControlUnit::setParticipate, ControlUnitAdder::withParticipate);
        bools(EXT_GF_GROUNDED, GeneratorFortescue::isGrounded, GeneratorFortescue::setGrounded, GeneratorFortescueAdder::withGrounded);
        bools(EXT_LINE_F_OPEN_PHASE_A, LineFortescue::isOpenPhaseA, LineFortescue::setOpenPhaseA, LineFortescueAdder::withOpenPhaseA);
        bools(EXT_LINE_F_OPEN_PHASE_B, LineFortescue::isOpenPhaseB, LineFortescue::setOpenPhaseB, LineFortescueAdder::withOpenPhaseB);
        bools(EXT_LINE_F_OPEN_PHASE_C, LineFortescue::isOpenPhaseC, LineFortescue::setOpenPhaseC, LineFortescueAdder::withOpenPhaseC);
        bools(EXT_TWO_WTF_FREE_FLUXES, TwoWindingsTransformerFortescue::isFreeFluxes, TwoWindingsTransformerFortescue::setFreeFluxes,
            TwoWindingsTransformerFortescueAdder::withFreeFluxes);
        bools(EXT_LEG_F_FREE_FLUXES, LegFortescue::isFreeFluxes, LegFortescue::setFreeFluxes, null);
        bools(REGULATING, null, PhaseTapChanger::setRegulating, PhaseTapChangerAdder::setRegulating);

        // ── Strings ─────────────────────────────────────────────────────────────────────────────────
        strings(ID, (Identifiable<?> x) -> x.getId(), (Identifiable<?> x, String v) -> x.setId(v),
            (IdentifiableAdder<?, ?> x, String v) -> x.setId(v));
        strings(NAME, (Identifiable<?> x) -> x.getNameOrId(), (Identifiable<?> x, String v) -> x.setName(v),
            (IdentifiableAdder<?, ?> x, String v) -> x.setName(v));
        strings(TL_NAME, TemporaryLimit::getName, null, (LoadingLimitsAdder.TemporaryLimitAdder<?> x, String v) -> x.setName(v));
        strings(NETWORK_SOURCE_FORMAT, Network::getSourceFormat, null, null);
        strings(OLG_ID, OperationalLimitsGroup::getId, null, null);
        strings(OVERLOAD_PREVIOUS_LIMIT_NAME, Overload::getPreviousLimitName, null, null);
        strings(AREA_TYPE, Area::getAreaType, null, AreaAdder::setAreaType);
        strings(VAL_ID, VoltageAngleLimit::getId, null, VoltageAngleLimitAdder::setId);
        strings(OMS_MONITORED_ELEMENT_ID, OverloadManagementSystem::getMonitoredElementId, null,
            OverloadManagementSystemAdder::setMonitoredElementId);
        strings(WORKING_VARIANT_ID, VariantManager::getWorkingVariantId, VariantManager::setWorkingVariant, null);
        strings(TRIPPING_KEY, OverloadManagementSystem.Tripping::getKey, null,
            (OverloadManagementSystemAdder.TrippingAdder<?> x, String v) -> x.setKey(v));
        strings(TRIPPING_NAME, OverloadManagementSystem.Tripping::getNameOrKey, OverloadManagementSystem.Tripping::setName,
            (OverloadManagementSystemAdder.TrippingAdder<?> x, String v) -> x.setName(v));
        strings(OVERLOAD_OLG_ID, Overload::getOperationalLimitsGroupId, null, null);
        strings(LL_PERMANENT_LIMIT_NAME, LoadingLimits::getPermanentLimitName, LoadingLimits::setPermanentLimitName,
            (LoadingLimitsAdder<?, ?> x, String v) -> x.setPermanentLimitName(v));
        strings(MEAS_ID, Measurement::getId, null, MeasurementAdder::setId);
        strings(DMEAS_ID, DiscreteMeasurement::getId, null, DiscreteMeasurementAdder::setId);
        strings(DMEAS_VALUE_STRING, (DiscreteMeasurement x) -> x.getValueAsString(), (DiscreteMeasurement x, String v) -> x.setValue(v),
            (DiscreteMeasurementAdder x, String v) -> x.setValue(v));
        strings(EXT_DMI_MODEL_NAME, (DynamicModelInfo<?> x) -> x.getModelName(), (DynamicModelInfo<?> x, String v) -> x.setModelName(v),
            (DynamicModelInfoAdder<?> x, String v) -> x.setModelName(v));
        strings(EXT_SVC_ZONE_NAME, ControlZone::getName, null, ControlZoneAdder::withName);
        strings(EXT_SVC_UNIT_ID, ControlUnit::getId, null, ControlUnitAdder::withId);
        strings(ADD_GEOGRAPHICAL_TAG, null, Substation::addGeographicalTag, null);
        strings(REMOVE_VARIANT, null, VariantManager::removeVariant, null);
        strings(BL_PAIRING_KEY, null, BoundaryLine::setPairingKey, BoundaryLineAdder::setPairingKey);
        strings(SUBSTATION_TSO, null, Substation::setTso, SubstationAdder::setTso);
        strings(EXT_FEEDER_NAME, null, ConnectablePosition.Feeder::setName, null);
        strings(BRANCH_VOLTAGE_LEVEL1, null, null, (BranchAdder<?, ?> x, String v) -> x.setVoltageLevel1(v));
        strings(BRANCH_VOLTAGE_LEVEL2, null, null, (BranchAdder<?, ?> x, String v) -> x.setVoltageLevel2(v));
        strings(BRANCH_BUS1, null, null, (BranchAdder<?, ?> x, String v) -> x.setBus1(v));
        strings(BRANCH_BUS2, null, null, (BranchAdder<?, ?> x, String v) -> x.setBus2(v));
        strings(BRANCH_CONNECTABLE_BUS1, null, null, (BranchAdder<?, ?> x, String v) -> x.setConnectableBus1(v));
        strings(BRANCH_CONNECTABLE_BUS2, null, null, (BranchAdder<?, ?> x, String v) -> x.setConnectableBus2(v));
        strings(HVDC_CONVERTER_STATION_ID1, null, null, HvdcLineAdder::setConverterStationId1);
        strings(HVDC_CONVERTER_STATION_ID2, null, null, HvdcLineAdder::setConverterStationId2);
        strings(BOUNDARY_LINE1_ID, null, null, TieLineAdder::setBoundaryLine1);
        strings(SW_BUS1, null, null, VoltageLevel.BusBreakerView.SwitchAdder::setBus1);
        strings(SW_BUS2, null, null, VoltageLevel.BusBreakerView.SwitchAdder::setBus2);
        strings(BOUNDARY_LINE2_ID, null, null, TieLineAdder::setBoundaryLine2);
    }

    // Each function keeps its own type (G, S, A): it checks the type of the object it is given. The
    // casts to Object are unchecked for that reason.

    @SuppressWarnings("unchecked")
    private static <G, S, A> void doubles(int code, ToDoubleFunction<G> get, ObjDoubleConsumer<S> set, ObjDoubleConsumer<A> add) {
        DOUBLES[code] = new DoubleProperty((ToDoubleFunction<Object>) get, (ObjDoubleConsumer<Object>) set, (ObjDoubleConsumer<Object>) add);
    }

    @SuppressWarnings("unchecked")
    private static <G, S, A> void ints(int code, ToIntFunction<G> get, ObjIntConsumer<S> set, ObjIntConsumer<A> add) {
        INTS[code] = new IntProperty((ToIntFunction<Object>) get, (ObjIntConsumer<Object>) set, (ObjIntConsumer<Object>) add);
    }

    @SuppressWarnings("unchecked")
    private static <G, S, A, E extends Enum<E>> void enums(int code, Class<E> type, Function<G, E> get, BiConsumer<S, E> set,
                                                           BiConsumer<A, E> add) {
        Function<Object, E> getter = (Function<Object, E>) get;
        BiConsumer<Object, E> setter = (BiConsumer<Object, E>) set;
        BiConsumer<Object, E> adder = (BiConsumer<Object, E>) add;
        INTS[code] = new IntProperty(getter == null ? null : o -> getter.apply(o).ordinal(),
            setter == null ? null : (o, v) -> setter.accept(o, Creation.enumValue(type, v)),
            adder == null ? null : (o, v) -> adder.accept(o, Creation.enumValue(type, v)));
    }

    @SuppressWarnings("unchecked")
    private static <G> void ordinals(int code, Function<G, ? extends Enum<?>> get) {
        Function<Object, ? extends Enum<?>> getter = (Function<Object, ? extends Enum<?>>) get;
        INTS[code] = new IntProperty(o -> getter.apply(o).ordinal(), null, null);
    }

    @SuppressWarnings("unchecked")
    private static <G, S, A> void bools(int code, Predicate<G> get, BoolSetter<S> set, BoolSetter<A> add) {
        BOOLS[code] = new BoolProperty((Predicate<Object>) get, (BoolSetter<Object>) set, (BoolSetter<Object>) add);
    }

    @SuppressWarnings("unchecked")
    private static <G, S, A> void strings(int code, Function<G, String> get, BiConsumer<S, String> set, BiConsumer<A, String> add) {
        STRINGS[code] = new StringProperty((Function<Object, String>) get, (BiConsumer<Object, String>) set,
            (BiConsumer<Object, String>) add);
    }
}
