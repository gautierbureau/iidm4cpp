package com.powsybl.iidmbridge;

import com.powsybl.commons.extensions.Extendable;
import com.powsybl.iidm.network.extensions.*;

import java.util.Map;

import static com.powsybl.iidmbridge.PropertyCodes.*;

/**
 * The extensions, by type code: the extension interface and its adder. On a host, the type code gets
 * the extension ({@link PropertyDispatcher#getRelatedObject}), starts a new one ({@link Creation#newAdder})
 * and removes it ({@link Creation#clear}); on any object, {@link PropertyDispatcher#getBool} tells whether
 * the object is one. Their properties have codes of their own, dispatched like any other.
 */
final class Extensions {

    private record Type(Class<?> extension, Class<?> adder) {
    }

    private static final Map<Integer, Type> TYPES = Map.ofEntries(
        type(ACTIVE_POWER_CONTROL, ActivePowerControl.class, ActivePowerControlAdder.class),
        type(COORDINATED_REACTIVE_CONTROL, CoordinatedReactiveControl.class, CoordinatedReactiveControlAdder.class),
        type(HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL, HvdcAngleDroopActivePowerControl.class,
            HvdcAngleDroopActivePowerControlAdder.class),
        type(HVDC_OPERATOR_ACTIVE_POWER_RANGE, HvdcOperatorActivePowerRange.class, HvdcOperatorActivePowerRangeAdder.class),
        type(SLACK_TERMINAL, SlackTerminal.class, SlackTerminalAdder.class),
        type(TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED, TwoWindingsTransformerToBeEstimated.class,
            TwoWindingsTransformerToBeEstimatedAdder.class),
        type(THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED, ThreeWindingsTransformerToBeEstimated.class,
            ThreeWindingsTransformerToBeEstimatedAdder.class),
        type(INJECTION_OBSERVABILITY, InjectionObservability.class, InjectionObservabilityAdder.class),
        type(BRANCH_OBSERVABILITY, BranchObservability.class, BranchObservabilityAdder.class),
        type(MEASUREMENTS, Measurements.class, MeasurementsAdder.class),
        type(DISCRETE_MEASUREMENTS, DiscreteMeasurements.class, DiscreteMeasurementsAdder.class),
        type(OBSERVABILITY_AREA, ObservabilityArea.class, ObservabilityAreaAdder.class),
        type(GENERATOR_SHORT_CIRCUIT, GeneratorShortCircuit.class, GeneratorShortCircuitAdder.class),
        type(BATTERY_SHORT_CIRCUIT, BatteryShortCircuit.class, BatteryShortCircuitAdder.class),
        type(IDENTIFIABLE_SHORT_CIRCUIT, IdentifiableShortCircuit.class, IdentifiableShortCircuitAdder.class),
        type(GENERATOR_ENTSOE_CATEGORY, GeneratorEntsoeCategory.class, GeneratorEntsoeCategoryAdder.class),
        type(GENERATOR_STARTUP, GeneratorStartup.class, GeneratorStartupAdder.class),
        type(LOAD_DETAIL, LoadDetail.class, LoadDetailAdder.class),
        type(STANDBY_AUTOMATON, StandbyAutomaton.class, StandbyAutomatonAdder.class),
        type(MANUAL_FREQUENCY_RESTORATION_RESERVE, ManualFrequencyRestorationReserve.class,
            ManualFrequencyRestorationReserveAdder.class),
        type(OPERATING_STATUS, OperatingStatus.class, OperatingStatusAdder.class),
        type(DYNAMIC_MODEL_INFO, DynamicModelInfo.class, DynamicModelInfoAdder.class),
        type(GENERATOR_FORTESCUE, GeneratorFortescue.class, GeneratorFortescueAdder.class),
        type(LINE_FORTESCUE, LineFortescue.class, LineFortescueAdder.class),
        type(LOAD_ASYMMETRICAL, LoadAsymmetrical.class, LoadAsymmetricalAdder.class),
        type(TWO_WINDINGS_TRANSFORMER_FORTESCUE, TwoWindingsTransformerFortescue.class,
            TwoWindingsTransformerFortescueAdder.class),
        type(THREE_WINDINGS_TRANSFORMER_FORTESCUE, ThreeWindingsTransformerFortescue.class,
            ThreeWindingsTransformerFortescueAdder.class),
        type(TWO_WINDINGS_TRANSFORMER_PHASE_ANGLE_CLOCK, TwoWindingsTransformerPhaseAngleClock.class,
            TwoWindingsTransformerPhaseAngleClockAdder.class),
        type(THREE_WINDINGS_TRANSFORMER_PHASE_ANGLE_CLOCK, ThreeWindingsTransformerPhaseAngleClock.class,
            ThreeWindingsTransformerPhaseAngleClockAdder.class),
        type(BUSBAR_SECTION_POSITION, BusbarSectionPosition.class, BusbarSectionPositionAdder.class),
        type(SUBSTATION_POSITION, SubstationPosition.class, SubstationPositionAdder.class),
        type(LINE_POSITION, LinePosition.class, LinePositionAdder.class),
        type(CONNECTABLE_POSITION, ConnectablePosition.class, ConnectablePositionAdder.class),
        type(REFERENCE_PRIORITIES, ReferencePriorities.class, ReferencePrioritiesAdder.class),
        type(REFERENCE_TERMINALS, ReferenceTerminals.class, ReferenceTerminalsAdder.class),
        type(SECONDARY_VOLTAGE_CONTROL, SecondaryVoltageControl.class, SecondaryVoltageControlAdder.class),
        type(LINE_COUPLINGS, LineCouplings.class, LineCouplingsAdder.class));

    private Extensions() {
    }

    private static Map.Entry<Integer, Type> type(int code, Class<?> extension, Class<?> adder) {
        return Map.entry(code, new Type(extension, adder));
    }

    /** Whether the code is the type code of an extension. */
    static boolean isType(int code) {
        return TYPES.containsKey(code);
    }

    /** Whether the object is an extension of that type. */
    static boolean isInstance(Object obj, int code) {
        return TYPES.get(code).extension().isInstance(obj);
    }

    /** The host's extension of that type, or null. */
    @SuppressWarnings({"unchecked", "rawtypes"})
    static Object get(Object host, int code) {
        // Raw: the extension classes are generic in their extendable, which Class<X> cannot express
        return ((Extendable) host).getExtension(TYPES.get(code).extension());
    }

    /** The adder of a new extension of that type on the host. */
    @SuppressWarnings({"unchecked", "rawtypes"})
    static Object newAdder(Object host, int code) {
        Class adder = TYPES.get(code).adder();
        return ((Extendable) host).newExtension(adder);
    }

    /** Removes the host's extension of that type. */
    @SuppressWarnings({"unchecked", "rawtypes"})
    static void remove(Object host, int code) {
        ((Extendable) host).removeExtension(TYPES.get(code).extension());
    }
}
