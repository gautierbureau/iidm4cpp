package com.powsybl.iidmbridge;

import com.powsybl.commons.extensions.Extendable;
import com.powsybl.commons.extensions.ExtensionAdder;
import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidm.network.regulation.RegulationMode;
import com.powsybl.iidm.network.regulation.VoltageRegulation;
import com.powsybl.iidm.network.regulation.VoltageRegulationAdder;
import com.powsybl.iidm.network.regulation.VoltageRegulationAdderOrBuilder;
import com.powsybl.iidm.network.regulation.VoltageRegulationBuilder;
import com.powsybl.iidm.network.regulation.VoltageRegulationHolder;
import com.powsybl.iidm.network.regulation.VoltageRegulationHolderAdder;

import java.util.Objects;

import static com.powsybl.iidmbridge.PropertyCodes.*;

/**
 * Creation and removal of network objects, called from {@link PropertyDispatcher#invokeObject}.
 *
 * <p>An object is created as in powsybl, in three steps:</p>
 * <ol>
 *   <li>{@code invoke(parent, TYPE)} returns an adder for a new object of that type (a
 *       {@link PendingAdd}); the network does not change;</li>
 *   <li>the usual setters, with the object's own property codes, set the adder's fields:
 *       {@code setDouble(adder, GEN_TARGET_P, 100)} calls {@code GeneratorAdder.setTargetP(100)};</li>
 *   <li>{@code invoke(adder, ACT_ADD)} adds the object and returns it.</li>
 * </ol>
 * <p>Nested adders (the voltage regulation of a generator adder) work the same way, except that
 * adding one returns null: its parent adder goes on. A field that the adder does not have fails
 * with a ClassCastException, i.e. a PropertyNotFoundException, as a property of another type of
 * object does.</p>
 *
 * <p>The same steps apply a change that takes several values at once, which separate setter calls
 * could not do without going through an invalid state: {@code invoke(regulation, REL_TERMINAL)}
 * starts {@code VoltageRegulation.setTerminal(terminal, targetValue)}, whose arguments are then
 * set with REL_TERMINAL and VR_TARGET_VALUE, and ACT_ADD applies it.</p>
 */
public final class Creation {

    /** Source format of the networks created from C++. */
    public static final String SOURCE_FORMAT = "iidm4cpp";

    private Creation() {}

    /** A powsybl adder (or builder) handed to C++, with what it belongs to: an object, or a parent adder. */
    record PendingAdd(Object adder, Object parent) {}

    /** The arguments of VoltageRegulation.setTerminal(terminal, targetValue); null terminal: local regulation. */
    static final class TerminalChange {
        private final VoltageRegulation regulation;
        private Terminal terminal;
        private double targetValue = Double.NaN;

        TerminalChange(VoltageRegulation regulation) {
            this.regulation = regulation;
        }

        void apply() {
            regulation.setTerminal(terminal, targetValue);
        }
    }

    /** The argument of holder.newOperationalLimitsGroup(id): field OLG_ID. */
    static final class NewGroup {
        private final LimitsDispatch.Holder holder;
        private String id;

        NewGroup(LimitsDispatch.Holder holder) {
            this.holder = holder;
        }
    }

    /** holder.getOrCreateSelectedOperationalLimitsGroup(), or (id) when OLG_ID is set. */
    static final class SelectedGroup {
        private final LimitsDispatch.Holder holder;
        private String id;

        SelectedGroup(LimitsDispatch.Holder holder) {
            this.holder = holder;
        }
    }

    /** The arguments of LoadingLimits.setTemporaryLimitValue(acceptableDuration, value). */
    static final class TemporaryLimitValueChange {
        private final LoadingLimits limits;
        private Integer acceptableDuration;
        private double value = Double.NaN;

        TemporaryLimitValueChange(LoadingLimits limits) {
            this.limits = limits;
        }

        void apply() {
            limits.setTemporaryLimitValue(Objects.requireNonNull(acceptableDuration, "acceptable duration"), value);
        }
    }

    /** A new, empty network. */
    public static Network createNetwork(String id) {
        return Network.create(id, SOURCE_FORMAT);
    }

    static PendingAdd newAdder(Object parent, int type) {
        Object owner = parent instanceof PendingAdd p ? p.adder() : parent;
        Object adder = switch (type) {
            case SUBSTATION -> ((Network) owner).newSubstation();
            case VOLTAGE_LEVEL -> owner instanceof Substation s ? s.newVoltageLevel() : ((Network) owner).newVoltageLevel();
            case BUS -> ((VoltageLevel) owner).getBusBreakerView().newBus();
            case GENERATOR -> ((VoltageLevel) owner).newGenerator();
            case LOAD -> ((VoltageLevel) owner).newLoad();
            case VOLTAGE_REGULATION -> owner instanceof VoltageRegulationHolderAdder<?> holderAdder
                ? holderAdder.newVoltageRegulation()
                : ((VoltageRegulationHolder<?>) owner).newVoltageRegulation();
            case ACTIVE_POWER_CONTROL -> newExtension(owner, ActivePowerControlAdder.class);
            case COORDINATED_REACTIVE_CONTROL -> newExtension(owner, CoordinatedReactiveControlAdder.class);
            case HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL -> newExtension(owner, HvdcAngleDroopActivePowerControlAdder.class);
            case HVDC_OPERATOR_ACTIVE_POWER_RANGE -> newExtension(owner, HvdcOperatorActivePowerRangeAdder.class);
            case SLACK_TERMINAL -> newExtension(owner, SlackTerminalAdder.class);
            case TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED -> newExtension(owner, TwoWindingsTransformerToBeEstimatedAdder.class);
            case THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED -> newExtension(owner, ThreeWindingsTransformerToBeEstimatedAdder.class);
            case INJECTION_OBSERVABILITY -> newExtension(owner, InjectionObservabilityAdder.class);
            case BRANCH_OBSERVABILITY -> newExtension(owner, BranchObservabilityAdder.class);
            case MEASUREMENTS -> newExtension(owner, MeasurementsAdder.class);
            case DISCRETE_MEASUREMENTS -> newExtension(owner, DiscreteMeasurementsAdder.class);
            case MEASUREMENT -> ((Measurements<?>) owner).newMeasurement();
            case DISCRETE_MEASUREMENT -> ((DiscreteMeasurements<?>) owner).newDiscreteMeasurement();
            case REL_TERMINAL -> new TerminalChange((VoltageRegulation) owner);
            case OPERATIONAL_LIMITS_GROUP -> new NewGroup(LimitsDispatch.holder(owner));
            case REL_SELECTED_OLG -> new SelectedGroup(LimitsDispatch.holder(owner));
            case CURRENT_LIMITS -> ((OperationalLimitsGroup) owner).newCurrentLimits();
            case ACTIVE_POWER_LIMITS -> ((OperationalLimitsGroup) owner).newActivePowerLimits();
            case APPARENT_POWER_LIMITS -> ((OperationalLimitsGroup) owner).newApparentPowerLimits();
            case TEMPORARY_LIMIT -> ((LoadingLimitsAdder<?, ?>) owner).beginTemporaryLimit();
            case LL_TEMPORARY_LIMIT_VALUE -> new TemporaryLimitValueChange((LoadingLimits) owner);
            case TC_STEPS_REPLACER -> PropertyDispatcher.tapChanger(owner).stepsReplacer();
            case RATIO_TAP_CHANGER_STEP -> ((RatioTapChangerStepsReplacer) owner).beginStep();
            case PHASE_TAP_CHANGER_STEP -> ((PhaseTapChangerStepsReplacer) owner).beginStep();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown type to create: " + type);
        };
        return new PendingAdd(adder, parent);
    }

    /** Adds what an adder describes and returns it; null for a nested adder (its parent goes on). */
    static Object add(Object obj) {
        if (!(obj instanceof PendingAdd pending)) {
            throw new BridgeErrors.PropertyNotFoundException("Not an adder: " + obj);
        }
        return switch (pending.adder()) {
            case IdentifiableAdder<?, ?> adder -> adder.add();
            case ExtensionAdder<?, ?> adder -> adder.add();
            case MeasurementAdder adder -> adder.add();
            case DiscreteMeasurementAdder adder -> adder.add();
            case VoltageRegulationBuilder builder -> builder.build();
            case VoltageRegulationAdder<?> adder -> {
                adder.add();
                yield null;
            }
            case TerminalChange change -> {
                change.apply();
                yield null;
            }
            case OperationalLimitsAdder<?, ?> adder -> adder.add();
            case LoadingLimitsAdder.TemporaryLimitAdder<?> adder -> {
                adder.endTemporaryLimit();
                yield null;
            }
            case TemporaryLimitValueChange change -> {
                change.apply();
                yield null;
            }
            case TapChangerStepAdder<?, ?> step -> {
                step.endStep();
                yield null;
            }
            case TapChangerStepsReplacer<?, ?> replacer -> {
                replacer.replaceSteps();
                yield null;
            }
            case NewGroup group -> group.holder.newOperationalLimitsGroup(Objects.requireNonNull(group.id, "group id"));
            case SelectedGroup group -> group.id == null
                ? group.holder.getOrCreateSelectedOperationalLimitsGroup()
                : group.holder.getOrCreateSelectedOperationalLimitsGroup(group.id);
            default -> throw new BridgeErrors.PropertyNotFoundException("Not an adder: " + pending.adder());
        };
    }

    /** Removes an identifiable from its network, a tap changer from its transformer, a measurement. */
    static void remove(Object obj) {
        switch (obj) {
            case Connectable<?> connectable -> connectable.remove();
            case TapChanger<?, ?, ?, ?> tapChanger -> tapChanger.remove();
            case Measurement measurement -> measurement.remove();
            case DiscreteMeasurement measurement -> measurement.remove();
            case OperationalLimits limits -> limits.remove();
            case VoltageLevel voltageLevel -> voltageLevel.remove();
            case Substation substation -> substation.remove();
            case HvdcLine hvdcLine -> hvdcLine.remove();
            case Bus bus -> bus.getVoltageLevel().getBusBreakerView().removeBus(bus.getId());
            default -> throw new BridgeErrors.PropertyNotFoundException("Cannot remove " + obj);
        }
    }

    /** setRelated(host, code, INVALID_HANDLE): removes the host's voltage regulation or extension. */
    static void clear(Object host, int code, Object value) {
        if (value != null) {
            throw new BridgeErrors.PropertyNotFoundException(
                "Code " + code + " can only be cleared (INVALID_HANDLE): new ones are created with invoke");
        }
        switch (code) {
            case VOLTAGE_REGULATION -> ((VoltageRegulationHolder<?>) host).removeVoltageRegulation();
            case ACTIVE_POWER_CONTROL -> removeExtension(host, ActivePowerControl.class);
            case COORDINATED_REACTIVE_CONTROL -> removeExtension(host, CoordinatedReactiveControl.class);
            case HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL -> removeExtension(host, HvdcAngleDroopActivePowerControl.class);
            case HVDC_OPERATOR_ACTIVE_POWER_RANGE -> removeExtension(host, HvdcOperatorActivePowerRange.class);
            case SLACK_TERMINAL -> removeExtension(host, SlackTerminal.class);
            case TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED -> removeExtension(host, TwoWindingsTransformerToBeEstimated.class);
            case THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED -> removeExtension(host, ThreeWindingsTransformerToBeEstimated.class);
            case INJECTION_OBSERVABILITY -> removeExtension(host, InjectionObservability.class);
            case BRANCH_OBSERVABILITY -> removeExtension(host, BranchObservability.class);
            case MEASUREMENTS -> removeExtension(host, Measurements.class);
            case DISCRETE_MEASUREMENTS -> removeExtension(host, DiscreteMeasurements.class);
            case OBSERVABILITY_AREA -> removeExtension(host, ObservabilityArea.class);
            case REL_SELECTED_OLG -> LimitsDispatch.holder(host).cancelSelectedOperationalLimitsGroup();
            case CURRENT_LIMITS -> ((OperationalLimitsGroup) host).removeCurrentLimits();
            case ACTIVE_POWER_LIMITS -> ((OperationalLimitsGroup) host).removeActivePowerLimits();
            case APPARENT_POWER_LIMITS -> ((OperationalLimitsGroup) host).removeApparentPowerLimits();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown code to clear: " + code);
        }
    }

    // ── Adder fields: the codes of the object's own properties ────────────

    static void setDouble(PendingAdd pending, int property, double value) {
        Object a = pending.adder();
        switch (property) {
            case VL_NOMINAL_V -> ((VoltageLevelAdder) a).setNominalV(value);
            case VL_LOW_VOLTAGE_LIMIT -> ((VoltageLevelAdder) a).setLowVoltageLimit(value);
            case VL_HIGH_VOLTAGE_LIMIT -> ((VoltageLevelAdder) a).setHighVoltageLimit(value);
            case GEN_MIN_P -> ((GeneratorAdder) a).setMinP(value);
            case GEN_MAX_P -> ((GeneratorAdder) a).setMaxP(value);
            case GEN_TARGET_P -> ((GeneratorAdder) a).setTargetP(value);
            case GEN_RATED_S -> ((GeneratorAdder) a).setRatedS(value);
            case LOCAL_TARGET_V -> ((VoltageRegulationHolderAdder<?>) a).setLocalTargetV(value);
            case LOCAL_TARGET_Q -> ((VoltageRegulationHolderAdder<?>) a).setLocalTargetQ(value);
            case LOAD_P0 -> ((LoadAdder) a).setP0(value);
            case LOAD_Q0 -> ((LoadAdder) a).setQ0(value);
            case VR_TARGET_VALUE -> {
                if (a instanceof TerminalChange change) {
                    change.targetValue = value;
                } else {
                    ((VoltageRegulationAdderOrBuilder<?>) a).withTargetValue(value);
                }
            }
            case VR_TARGET_DEADBAND -> ((VoltageRegulationAdderOrBuilder<?>) a).withTargetDeadband(value);
            case VR_SLOPE -> ((VoltageRegulationAdderOrBuilder<?>) a).withSlope(value);
            case EXT_APC_DROOP -> ((ActivePowerControlAdder<?>) a).withDroop(value);
            case EXT_APC_PARTICIPATION_FACTOR -> ((ActivePowerControlAdder<?>) a).withParticipationFactor(value);
            case EXT_APC_MIN_TARGET_P -> ((ActivePowerControlAdder<?>) a).withMinTargetP(value);
            case EXT_APC_MAX_TARGET_P -> ((ActivePowerControlAdder<?>) a).withMaxTargetP(value);
            case EXT_CRC_Q_PERCENT -> ((CoordinatedReactiveControlAdder) a).withQPercent(value);
            case EXT_HADAPC_DROOP -> ((HvdcAngleDroopActivePowerControlAdder) a).withDroop((float) value);
            case EXT_HADAPC_P0 -> ((HvdcAngleDroopActivePowerControlAdder) a).withP0((float) value);
            case EXT_HOAR_OPR_CS1_TO_CS2 -> ((HvdcOperatorActivePowerRangeAdder) a).withOprFromCS1toCS2((float) value);
            case EXT_HOAR_OPR_CS2_TO_CS1 -> ((HvdcOperatorActivePowerRangeAdder) a).withOprFromCS2toCS1((float) value);
            case OBS_QUALITY_P  -> ((InjectionObservabilityAdder<?>) a).withStandardDeviationP(value);
            case OBS_QUALITY_Q  -> ((InjectionObservabilityAdder<?>) a).withStandardDeviationQ(value);
            case OBS_QUALITY_V  -> ((InjectionObservabilityAdder<?>) a).withStandardDeviationV(value);
            case OBS_QUALITY_P1 -> ((BranchObservabilityAdder<?>) a).withStandardDeviationP1(value);
            case OBS_QUALITY_P2 -> ((BranchObservabilityAdder<?>) a).withStandardDeviationP2(value);
            case OBS_QUALITY_Q1 -> ((BranchObservabilityAdder<?>) a).withStandardDeviationQ1(value);
            case OBS_QUALITY_Q2 -> ((BranchObservabilityAdder<?>) a).withStandardDeviationQ2(value);
            case MEAS_VALUE -> ((MeasurementAdder) a).setValue(value);
            case MEAS_STD_DEV -> ((MeasurementAdder) a).setStandardDeviation(value);
            case LL_PERMANENT_LIMIT -> ((LoadingLimitsAdder<?, ?>) a).setPermanentLimit(value);
            case TL_VALUE -> {
                if (a instanceof TemporaryLimitValueChange change) {
                    change.value = value;
                } else {
                    ((LoadingLimitsAdder.TemporaryLimitAdder<?>) a).setValue(value);
                }
            }
            case STEP_RHO -> ((TapChangerStepAdder<?, ?>) a).setRho(value);
            case STEP_R -> ((TapChangerStepAdder<?, ?>) a).setR(value);
            case STEP_X -> ((TapChangerStepAdder<?, ?>) a).setX(value);
            case STEP_G -> ((TapChangerStepAdder<?, ?>) a).setG(value);
            case STEP_B -> ((TapChangerStepAdder<?, ?>) a).setB(value);
            case PTC_STEP_ALPHA -> ((PhaseTapChangerStepAdder<?, ?>) a).setAlpha(value);
            default -> throw unknownField("double", property);
        }
    }

    static void setInt(PendingAdd pending, int property, int value) {
        Object a = pending.adder();
        switch (property) {
            case TERMINAL_NODE -> ((InjectionAdder<?, ?>) a).setNode(value);
            case VL_TOPOLOGY_KIND -> ((VoltageLevelAdder) a).setTopologyKind(enumValue(TopologyKind.class, value));
            case GEN_ENERGY_SOURCE -> ((GeneratorAdder) a).setEnergySource(enumValue(EnergySource.class, value));
            case LOAD_TYPE -> ((LoadAdder) a).setLoadType(enumValue(LoadType.class, value));
            case VR_MODE -> ((VoltageRegulationAdderOrBuilder<?>) a).withMode(value < 0 ? null : enumValue(RegulationMode.class, value));
            case OBS_QUALITY_P  -> ((InjectionObservabilityAdder<?>) a).withRedundantP(redundant(value));
            case OBS_QUALITY_Q  -> ((InjectionObservabilityAdder<?>) a).withRedundantQ(redundant(value));
            case OBS_QUALITY_V  -> ((InjectionObservabilityAdder<?>) a).withRedundantV(redundant(value));
            case OBS_QUALITY_P1 -> ((BranchObservabilityAdder<?>) a).withRedundantP1(redundant(value));
            case OBS_QUALITY_P2 -> ((BranchObservabilityAdder<?>) a).withRedundantP2(redundant(value));
            case OBS_QUALITY_Q1 -> ((BranchObservabilityAdder<?>) a).withRedundantQ1(redundant(value));
            case OBS_QUALITY_Q2 -> ((BranchObservabilityAdder<?>) a).withRedundantQ2(redundant(value));
            case MEAS_TYPE -> ((MeasurementAdder) a).setType(enumValue(Measurement.Type.class, value));
            case MEAS_SIDE -> ((MeasurementAdder) a).setSide(value < 0 ? null : enumValue(ThreeSides.class, value));
            case DMEAS_TYPE -> ((DiscreteMeasurementAdder) a).setType(enumValue(DiscreteMeasurement.Type.class, value));
            case DMEAS_TAP_CHANGER -> ((DiscreteMeasurementAdder) a)
                .setTapChanger(value < 0 ? null : enumValue(DiscreteMeasurement.TapChanger.class, value));
            case DMEAS_VALUE_INT -> ((DiscreteMeasurementAdder) a).setValue(value);
            case LL_DETECTION_KIND -> ((LoadingLimitsAdder<?, ?>) a).setDetectionKind(enumValue(DetectionKind.class, value));
            case TL_ACCEPTABLE_DURATION -> {
                if (a instanceof TemporaryLimitValueChange change) {
                    change.acceptableDuration = value;
                } else {
                    ((LoadingLimitsAdder.TemporaryLimitAdder<?>) a).setAcceptableDuration(value);
                }
            }
            default -> throw unknownField("int", property);
        }
    }

    static void setBool(PendingAdd pending, int property, boolean value) {
        Object a = pending.adder();
        switch (property) {
            case FICTITIOUS -> ((IdentifiableAdder<?, ?>) a).setFictitious(value);
            case ENSURE_ID_UNICITY -> {
                switch (a) {
                    case MeasurementAdder m -> m.setEnsureIdUnicity(value);
                    case DiscreteMeasurementAdder m -> m.setEnsureIdUnicity(value);
                    case LoadingLimitsAdder.TemporaryLimitAdder<?> t -> {
                        // the name of a temporary limit
                        if (value) t.ensureNameUnicity();
                    }
                    default -> ((IdentifiableAdder<?, ?>) a).setEnsureIdUnicity(value);
                }
            }
            case EQUIVALENT -> ((ConnectableAdder<?, ?>) a).setEquivalent(value);
            case GEN_CONDENSER -> ((GeneratorAdder) a).setCondenser(value);
            case VR_REGULATING -> ((VoltageRegulationAdderOrBuilder<?>) a).withRegulating(value);
            case EXT_APC_PARTICIPATE -> ((ActivePowerControlAdder<?>) a).withParticipate(value);
            case EXT_HADAPC_ENABLED -> ((HvdcAngleDroopActivePowerControlAdder) a).withEnabled(value);
            case EXT_TWO_WT_TBE_RTC -> ((TwoWindingsTransformerToBeEstimatedAdder) a).withRatioTapChangerStatus(value);
            case EXT_TWO_WT_TBE_PTC -> ((TwoWindingsTransformerToBeEstimatedAdder) a).withPhaseTapChangerStatus(value);
            case EXT_THREE_WT_TBE_RTC1 -> ((ThreeWindingsTransformerToBeEstimatedAdder) a).withRatioTapChanger1Status(value);
            case EXT_THREE_WT_TBE_RTC2 -> ((ThreeWindingsTransformerToBeEstimatedAdder) a).withRatioTapChanger2Status(value);
            case EXT_THREE_WT_TBE_RTC3 -> ((ThreeWindingsTransformerToBeEstimatedAdder) a).withRatioTapChanger3Status(value);
            case EXT_THREE_WT_TBE_PTC1 -> ((ThreeWindingsTransformerToBeEstimatedAdder) a).withPhaseTapChanger1Status(value);
            case EXT_THREE_WT_TBE_PTC2 -> ((ThreeWindingsTransformerToBeEstimatedAdder) a).withPhaseTapChanger2Status(value);
            case EXT_THREE_WT_TBE_PTC3 -> ((ThreeWindingsTransformerToBeEstimatedAdder) a).withPhaseTapChanger3Status(value);
            case EXT_OBS_OBSERVABLE -> {
                if (a instanceof BranchObservabilityAdder<?> branch) branch.withObservable(value);
                else ((InjectionObservabilityAdder<?>) a).withObservable(value);
            }
            case MEAS_VALID -> ((MeasurementAdder) a).setValid(value);
            case DMEAS_VALID -> ((DiscreteMeasurementAdder) a).setValid(value);
            case DMEAS_VALUE_BOOL -> ((DiscreteMeasurementAdder) a).setValue(value);
            case TL_FICTITIOUS -> ((LoadingLimitsAdder.TemporaryLimitAdder<?>) a).setFictitious(value);
            default -> throw unknownField("bool", property);
        }
    }

    static void setString(PendingAdd pending, int property, String value) {
        Object a = pending.adder();
        switch (property) {
            case ID -> ((IdentifiableAdder<?, ?>) a).setId(value);
            case NAME -> ((IdentifiableAdder<?, ?>) a).setName(value);
            case COUNTRY -> ((SubstationAdder) a).setCountry(Country.valueOf(value));
            case SUBSTATION_TSO -> ((SubstationAdder) a).setTso(value);
            case TERMINAL_BUS_BREAKER_BUS_ID -> ((InjectionAdder<?, ?>) a).setBus(value);
            case TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID -> ((InjectionAdder<?, ?>) a).setConnectableBus(value);
            case MEAS_ID -> ((MeasurementAdder) a).setId(value);
            case DMEAS_ID -> ((DiscreteMeasurementAdder) a).setId(value);
            case DMEAS_VALUE_STRING -> ((DiscreteMeasurementAdder) a).setValue(value);
            case OLG_ID -> {
                switch (a) {
                    case NewGroup group -> group.id = value;
                    default -> ((SelectedGroup) a).id = value;
                }
            }
            case LL_PERMANENT_LIMIT_NAME -> ((LoadingLimitsAdder<?, ?>) a).setPermanentLimitName(value);
            case TL_NAME -> ((LoadingLimitsAdder.TemporaryLimitAdder<?>) a).setName(value);
            default -> throw unknownField("string", property);
        }
    }

    /** Properties of the adders that are properties holders (limits, reactive limits...). */
    static void setKeyedString(PendingAdd pending, int property, String key, String value) {
        Object a = pending.adder();
        switch (property) {
            case PROPERTY -> {
                Objects.requireNonNull(value, "property value");
                switch (a) {
                    case MeasurementAdder m -> m.putProperty(key, value);
                    case DiscreteMeasurementAdder m -> m.putProperty(key, value);
                    default -> ((BasePropertiesHolder) a).setProperty(key, value);
                }
            }
            default -> throw unknownField("keyed string", property);
        }
    }

    static void setRelated(PendingAdd pending, int relation, Object value) {
        Object a = pending.adder();
        switch (relation) {
            case REL_TERMINAL -> {
                switch (a) {
                    case TerminalChange change -> change.terminal = (Terminal) value;
                    case SlackTerminalAdder adder -> adder.withTerminal((Terminal) value);
                    default -> ((VoltageRegulationAdderOrBuilder<?>) a).withTerminal((Terminal) value);
                }
            }
            default -> throw unknownField("related object", relation);
        }
    }

    /** An optional boolean: -1 = undefined. */
    private static Boolean redundant(int value) {
        return value < 0 ? null : value != 0;
    }

    /** The constant of an enum from its ordinal (the C++ enums follow the Java declaration order). */
    static <E extends Enum<E>> E enumValue(Class<E> type, int ordinal) {
        E[] values = type.getEnumConstants();
        if (ordinal < 0 || ordinal >= values.length) {
            throw new IllegalArgumentException("Invalid " + type.getSimpleName() + " value: " + ordinal);
        }
        return values[ordinal];
    }

    private static BridgeErrors.PropertyNotFoundException unknownField(String type, int property) {
        return new BridgeErrors.PropertyNotFoundException("Unknown " + type + " field for an adder: " + property);
    }

    @SuppressWarnings({"unchecked", "rawtypes"})
    private static Object newExtension(Object host, Class type) {
        return ((Extendable) host).newExtension(type);
    }

    @SuppressWarnings({"unchecked", "rawtypes"})
    private static void removeExtension(Object host, Class type) {
        ((Extendable) host).removeExtension(type);
    }
}
