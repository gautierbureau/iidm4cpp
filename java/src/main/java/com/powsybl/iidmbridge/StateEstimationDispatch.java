package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;

import java.util.Comparator;
import java.util.Objects;
import java.util.stream.Stream;

import static com.powsybl.iidmbridge.PropertyCodes.*;

/**
 * Helpers for the state estimation extensions, called from {@link PropertyDispatcher}: the
 * measurements' optional enums, and the areas of {@link ObservabilityArea}.
 */
final class StateEstimationDispatch {

    private StateEstimationDispatch() {}

    /**
     * An area of an observability area extension, with the voltage level it belongs to: the voltage
     * level is needed to resolve the area's nodes and bus-breaker bus ids into objects.
     */
    record AreaRef(VoltageLevel voltageLevel, ObservabilityArea.AreaCharacteristics area) {}

    // ── Measurements ──────────────────────────────────────────────────────

    /** -1 when the measurement has no side. */
    static int measurementSide(Measurement m) {
        ThreeSides side = m.getSide();
        return side == null ? -1 : side.ordinal();
    }

    /** -1 when the measurement is not attached to a tap changer. */
    static int discreteMeasurementTapChanger(DiscreteMeasurement m) {
        DiscreteMeasurement.TapChanger tc = m.getTapChanger();
        return tc == null ? -1 : tc.ordinal();
    }

    // ── ObservabilityArea ─────────────────────────────────────────────────

    static Stream<?> observabilityAreas(ObservabilityArea ext) {
        VoltageLevel vl = ext.getExtendable();
        return ext.getObservabilityAreas().stream()
            // powsybl returns them in no particular order
            .sorted(Comparator.comparingInt(ObservabilityArea.AreaCharacteristics::getAreaNumber))
            .map(area -> new AreaRef(vl, area));
    }

    static Object observabilityAreaByNode(ObservabilityArea ext, int node) {
        return wrap(ext.getExtendable(), ext.getNodeBreakerView().getObservabilityArea(node));
    }

    static Object observabilityAreaByBus(ObservabilityArea ext, int relation, String busId) {
        ObservabilityArea.BusView view = relation == REL_OBS_AREA_BY_BBV_BUS ? ext.getBusBreakerView() : ext.getBusView();
        return wrap(ext.getExtendable(), view.getObservabilityArea(busId, false));
    }

    private static Object wrap(VoltageLevel vl, ObservabilityArea.AreaCharacteristics area) {
        return area == null ? null : new AreaRef(vl, area);
    }

    // The node-breaker data of an area of a bus-breaker voltage level (and vice versa) throws when
    // used, so the topology kind of the voltage level decides which data is read.

    private static boolean isNodeBreaker(AreaRef ref) {
        return ref.voltageLevel().getTopologyKind() == TopologyKind.NODE_BREAKER;
    }

    /** Sorted node numbers of a node-breaker area, empty for a bus-breaker area. */
    static int[] observabilityAreaNodes(Object obj) {
        AreaRef ref = (AreaRef) obj;
        if (!isNodeBreaker(ref)) {
            return new int[0];
        }
        return ref.area().getNodeBreakerData().getNodes().stream().mapToInt(Integer::intValue).sorted().toArray();
    }

    static Stream<?> observabilityAreaTerminals(Object obj) {
        AreaRef ref = (AreaRef) obj;
        if (isNodeBreaker(ref)) {
            // Resolved from the voltage level rather than AreaCharacteristics.getTerminals(): in
            // powsybl-core 7.4 the latter reads the node of every terminal of every connectable of
            // the voltage level, and throws when a branch has its other side in a bus-breaker
            // voltage level.
            VoltageLevel.NodeBreakerView view = ref.voltageLevel().getNodeBreakerView();
            return ref.area().getNodeBreakerData().getNodes().stream().sorted()
                .map(view::getTerminal).filter(Objects::nonNull);
        }
        // getTerminals() is a HashSet: sorted, or the order would change from run to run
        return ref.area().getTerminals().stream()
            .filter(t -> t.getVoltageLevel() == ref.voltageLevel())
            .sorted(TERMINAL_ORDER);
    }

    /** By connectable id, then by rank among the connectable's terminals (side). */
    private static final Comparator<Terminal> TERMINAL_ORDER = Comparator
        .comparing((Terminal t) -> t.getConnectable().getId())
        .thenComparingInt(t -> t.getConnectable().getTerminals().indexOf(t));

    /** Bus-breaker view buses of a bus-breaker area, sorted by id; empty for a node-breaker area. */
    static Stream<?> observabilityAreaBuses(Object obj) {
        AreaRef ref = (AreaRef) obj;
        if (isNodeBreaker(ref)) {
            return Stream.empty();
        }
        VoltageLevel.BusBreakerView view = ref.voltageLevel().getBusBreakerView();
        return ref.area().getBusBreakerData().getBusIds().stream().sorted()
            .map(view::getBus).filter(Objects::nonNull);
    }
}
