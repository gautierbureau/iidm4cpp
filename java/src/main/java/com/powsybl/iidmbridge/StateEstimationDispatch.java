package com.powsybl.iidmbridge;

import com.powsybl.commons.extensions.Extendable;
import com.powsybl.commons.extensions.Extension;
import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;

import java.util.Objects;
import java.util.stream.Stream;

import static com.powsybl.iidmbridge.PropertyCodes.*;

/**
 * State estimation extensions, called from {@link PropertyDispatcher}:
 * {@link TwoWindingsTransformerToBeEstimated}, {@link ThreeWindingsTransformerToBeEstimated},
 * {@link InjectionObservability}, {@link BranchObservability}, {@link Measurements},
 * {@link DiscreteMeasurements} and {@link ObservabilityArea}.
 */
final class StateEstimationDispatch {

    private StateEstimationDispatch() {}

    /**
     * An observability area together with the voltage level it belongs to: the voltage level is
     * needed to resolve the area's bus-breaker bus ids into {@link Bus} objects.
     */
    record ObservabilityAreaRef(VoltageLevel voltageLevel, ObservabilityArea.AreaCharacteristics area) {}

    // ── Generic extension lookup ──────────────────────────────────────────

    @SuppressWarnings({"unchecked", "rawtypes"})
    static <E extends Extension<?>> E findExtension(Object obj, Class<? super E> type) {
        return (E) ((Extendable) obj).getExtension(type);
    }

    static <E extends Extension<?>> E extension(Object obj, Class<? super E> type) {
        E ext = findExtension(obj, type);
        if (ext == null) {
            throw new BridgeErrors.ExtensionNotFoundException(type.getSimpleName() + " extension not present on "
                + (obj instanceof com.powsybl.iidm.network.Identifiable<?> i ? i.getId() : String.valueOf(obj)));
        }
        return ext;
    }

    // ── ToBeEstimated ─────────────────────────────────────────────────────

    static boolean getToBeEstimated(Object obj, int property) {
        if (property == EXT_TWO_WT_TBE_EXISTS) {
            return findExtension(obj, TwoWindingsTransformerToBeEstimated.class) != null;
        }
        if (property == EXT_THREE_WT_TBE_EXISTS) {
            return findExtension(obj, ThreeWindingsTransformerToBeEstimated.class) != null;
        }
        if (property == EXT_TWO_WT_TBE_RTC || property == EXT_TWO_WT_TBE_PTC) {
            TwoWindingsTransformerToBeEstimated ext = extension(obj, TwoWindingsTransformerToBeEstimated.class);
            return property == EXT_TWO_WT_TBE_RTC ? ext.shouldEstimateRatioTapChanger() : ext.shouldEstimatePhaseTapChanger();
        }
        ThreeWindingsTransformerToBeEstimated ext = extension(obj, ThreeWindingsTransformerToBeEstimated.class);
        return switch (property) {
            case EXT_THREE_WT_TBE_RTC1 -> ext.shouldEstimateRatioTapChanger1();
            case EXT_THREE_WT_TBE_RTC2 -> ext.shouldEstimateRatioTapChanger2();
            case EXT_THREE_WT_TBE_RTC3 -> ext.shouldEstimateRatioTapChanger3();
            case EXT_THREE_WT_TBE_PTC1 -> ext.shouldEstimatePhaseTapChanger1();
            case EXT_THREE_WT_TBE_PTC2 -> ext.shouldEstimatePhaseTapChanger2();
            case EXT_THREE_WT_TBE_PTC3 -> ext.shouldEstimatePhaseTapChanger3();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown ToBeEstimated property: " + property);
        };
    }

    static void setToBeEstimated(Object obj, int property, boolean value) {
        if (property == EXT_TWO_WT_TBE_RTC || property == EXT_TWO_WT_TBE_PTC) {
            TwoWindingsTransformerToBeEstimated ext = extension(obj, TwoWindingsTransformerToBeEstimated.class);
            if (property == EXT_TWO_WT_TBE_RTC) {
                ext.shouldEstimateRatioTapChanger(value);
            } else {
                ext.shouldEstimatePhaseTapChanger(value);
            }
            return;
        }
        ThreeWindingsTransformerToBeEstimated ext = extension(obj, ThreeWindingsTransformerToBeEstimated.class);
        switch (property) {
            case EXT_THREE_WT_TBE_RTC1 -> ext.shouldEstimateRatioTapChanger1(value);
            case EXT_THREE_WT_TBE_RTC2 -> ext.shouldEstimateRatioTapChanger2(value);
            case EXT_THREE_WT_TBE_RTC3 -> ext.shouldEstimateRatioTapChanger3(value);
            case EXT_THREE_WT_TBE_PTC1 -> ext.shouldEstimatePhaseTapChanger1(value);
            case EXT_THREE_WT_TBE_PTC2 -> ext.shouldEstimatePhaseTapChanger2(value);
            case EXT_THREE_WT_TBE_PTC3 -> ext.shouldEstimatePhaseTapChanger3(value);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown ToBeEstimated property for set: " + property);
        }
    }

    // ── Injection / branch observability ──────────────────────────────────

    static boolean getObservabilityFlag(Object obj, int property) {
        return switch (property) {
            case EXT_INJ_OBS_EXISTS        -> findExtension(obj, InjectionObservability.class) != null;
            case EXT_INJ_OBS_OBSERVABLE    -> ((InjectionObservability<?>) extension(obj, InjectionObservability.class)).isObservable();
            case EXT_BRANCH_OBS_EXISTS     -> findExtension(obj, BranchObservability.class) != null;
            case EXT_BRANCH_OBS_OBSERVABLE -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).isObservable();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown observability property: " + property);
        };
    }

    static void setObservable(Object obj, int property, boolean value) {
        switch (property) {
            case EXT_INJ_OBS_OBSERVABLE    -> ((InjectionObservability<?>) extension(obj, InjectionObservability.class)).setObservable(value);
            case EXT_BRANCH_OBS_OBSERVABLE -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).setObservable(value);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown observability property for set: " + property);
        }
    }

    /** Quality block base (e.g. {@code EXT_INJ_OBS_QUALITY_P}) of a quality property code. */
    private static int qualityBase(int property) {
        return property - Math.floorMod(property, 10);
    }

    private static ObservabilityQuality<?> findQuality(Object obj, int base) {
        return switch (base) {
            case EXT_INJ_OBS_QUALITY_P     -> ((InjectionObservability<?>) extension(obj, InjectionObservability.class)).getNullableQualityP();
            case EXT_INJ_OBS_QUALITY_Q     -> ((InjectionObservability<?>) extension(obj, InjectionObservability.class)).getNullableQualityQ();
            case EXT_INJ_OBS_QUALITY_V     -> ((InjectionObservability<?>) extension(obj, InjectionObservability.class)).getNullableQualityV();
            case EXT_BRANCH_OBS_QUALITY_P1 -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).getNullableQualityP1();
            case EXT_BRANCH_OBS_QUALITY_P2 -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).getNullableQualityP2();
            case EXT_BRANCH_OBS_QUALITY_Q1 -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).getNullableQualityQ1();
            case EXT_BRANCH_OBS_QUALITY_Q2 -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).getNullableQualityQ2();
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown observability quality property: " + base);
        };
    }

    private static ObservabilityQuality<?> quality(Object obj, int base) {
        ObservabilityQuality<?> q = findQuality(obj, base);
        if (q == null) {
            throw new BridgeErrors.PropertyNotFoundException("Observability quality not defined: " + base);
        }
        return q;
    }

    static boolean qualityExists(Object obj, int property) {
        return findQuality(obj, qualityBase(property)) != null;
    }

    static double getQualityStandardDeviation(Object obj, int property) {
        return quality(obj, qualityBase(property)).getStandardDeviation();
    }

    /** Sets the standard deviation, creating the quality (with an undefined redundancy) if needed. */
    static void setQualityStandardDeviation(Object obj, int property, double standardDeviation) {
        int base = qualityBase(property);
        ObservabilityQuality<?> q = findQuality(obj, base);
        if (q != null) {
            q.setStandardDeviation(standardDeviation);
            return;
        }
        switch (base) {
            case EXT_INJ_OBS_QUALITY_P     -> ((InjectionObservability<?>) extension(obj, InjectionObservability.class)).setQualityP(standardDeviation);
            case EXT_INJ_OBS_QUALITY_Q     -> ((InjectionObservability<?>) extension(obj, InjectionObservability.class)).setQualityQ(standardDeviation);
            case EXT_INJ_OBS_QUALITY_V     -> ((InjectionObservability<?>) extension(obj, InjectionObservability.class)).setQualityV(standardDeviation);
            case EXT_BRANCH_OBS_QUALITY_P1 -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).setQualityP1(standardDeviation);
            case EXT_BRANCH_OBS_QUALITY_P2 -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).setQualityP2(standardDeviation);
            case EXT_BRANCH_OBS_QUALITY_Q1 -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).setQualityQ1(standardDeviation);
            case EXT_BRANCH_OBS_QUALITY_Q2 -> ((BranchObservability<?>) extension(obj, BranchObservability.class)).setQualityQ2(standardDeviation);
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown observability quality property: " + property);
        }
    }

    /** -1 = undefined, 0 = false, 1 = true. */
    static int getQualityRedundant(Object obj, int property) {
        return quality(obj, qualityBase(property)).isRedundant().map(r -> r ? 1 : 0).orElse(-1);
    }

    static void setQualityRedundant(Object obj, int property, int value) {
        quality(obj, qualityBase(property)).setRedundant(value < 0 ? null : value != 0);
    }

    // ── Measurements ──────────────────────────────────────────────────────

    static Stream<?> measurements(Object obj) {
        Measurements<?> ext = findExtension(obj, Measurements.class);
        return ext == null ? Stream.empty() : ext.getMeasurements().stream();
    }

    static Object measurementById(Object obj, String id) {
        Measurements<?> ext = findExtension(obj, Measurements.class);
        return ext == null ? null : ext.getMeasurement(id);
    }

    /** -1 when the measurement has no side. */
    static int measurementSide(Measurement m) {
        ThreeSides side = m.getSide();
        return side == null ? -1 : side.ordinal();
    }

    // ── DiscreteMeasurements ──────────────────────────────────────────────

    static Stream<?> discreteMeasurements(Object obj) {
        DiscreteMeasurements<?> ext = findExtension(obj, DiscreteMeasurements.class);
        return ext == null ? Stream.empty() : ext.getDiscreteMeasurements().stream();
    }

    static Object discreteMeasurementById(Object obj, String id) {
        DiscreteMeasurements<?> ext = findExtension(obj, DiscreteMeasurements.class);
        return ext == null ? null : ext.getDiscreteMeasurement(id);
    }

    /** -1 when the measurement is not attached to a tap changer. */
    static int discreteMeasurementTapChanger(DiscreteMeasurement m) {
        DiscreteMeasurement.TapChanger tc = m.getTapChanger();
        return tc == null ? -1 : tc.ordinal();
    }

    // ── ObservabilityArea ─────────────────────────────────────────────────

    static Stream<?> observabilityAreas(Object obj) {
        VoltageLevel vl = (VoltageLevel) obj;
        ObservabilityArea ext = vl.getExtension(ObservabilityArea.class);
        return ext == null ? Stream.empty()
            : ext.getObservabilityAreas().stream()
                // powsybl returns them in no particular order
                .sorted(java.util.Comparator.comparingInt(ObservabilityArea.AreaCharacteristics::getAreaNumber))
                .map(area -> new ObservabilityAreaRef(vl, area));
    }

    static Object observabilityAreaByNode(Object obj, int node) {
        VoltageLevel vl = (VoltageLevel) obj;
        ObservabilityArea ext = extension(vl, ObservabilityArea.class);
        return wrap(vl, ext.getNodeBreakerView().getObservabilityArea(node));
    }

    static Object observabilityAreaByBus(Object obj, int relation, String busId) {
        VoltageLevel vl = (VoltageLevel) obj;
        ObservabilityArea ext = extension(vl, ObservabilityArea.class);
        ObservabilityArea.BusView view = relation == REL_OBS_AREA_BY_BBV_BUS ? ext.getBusBreakerView() : ext.getBusView();
        return wrap(vl, view.getObservabilityArea(busId, false));
    }

    private static Object wrap(VoltageLevel vl, ObservabilityArea.AreaCharacteristics area) {
        return area == null ? null : new ObservabilityAreaRef(vl, area);
    }

    // The node-breaker data of an area of a bus-breaker voltage level (and vice versa) throws when
    // used, so the topology kind of the voltage level decides which data is read.

    private static boolean isNodeBreaker(ObservabilityAreaRef ref) {
        return ref.voltageLevel().getTopologyKind() == TopologyKind.NODE_BREAKER;
    }

    /** Sorted node numbers of a node-breaker area, empty for a bus-breaker area. */
    static int[] observabilityAreaNodes(Object obj) {
        ObservabilityAreaRef ref = (ObservabilityAreaRef) obj;
        if (!isNodeBreaker(ref)) {
            return new int[0];
        }
        return ref.area().getNodeBreakerData().getNodes().stream().mapToInt(Integer::intValue).sorted().toArray();
    }

    static Stream<?> observabilityAreaTerminals(Object obj) {
        ObservabilityAreaRef ref = (ObservabilityAreaRef) obj;
        if (isNodeBreaker(ref)) {
            // Resolved from the voltage level rather than AreaCharacteristics.getTerminals(): in
            // powsybl-core 7.4 the latter reads the node of every terminal of every connectable of
            // the voltage level, and throws when a branch has its other side in a bus-breaker
            // voltage level.
            VoltageLevel.NodeBreakerView view = ref.voltageLevel().getNodeBreakerView();
            return ref.area().getNodeBreakerData().getNodes().stream().sorted()
                .map(view::getTerminal).filter(Objects::nonNull);
        }
        return ref.area().getTerminals().stream()
            .filter(t -> t.getVoltageLevel() == ref.voltageLevel());
    }

    /** Bus-breaker view buses of a bus-breaker area, sorted by id; empty for a node-breaker area. */
    static Stream<?> observabilityAreaBuses(Object obj) {
        ObservabilityAreaRef ref = (ObservabilityAreaRef) obj;
        if (isNodeBreaker(ref)) {
            return Stream.empty();
        }
        VoltageLevel.BusBreakerView view = ref.voltageLevel().getBusBreakerView();
        return ref.area().getBusBreakerData().getBusIds().stream().sorted()
            .map(view::getBus).filter(Objects::nonNull);
    }
}
