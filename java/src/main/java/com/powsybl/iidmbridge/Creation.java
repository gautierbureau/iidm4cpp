package com.powsybl.iidmbridge;

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
import com.powsybl.iidm.network.util.SwitchPredicates;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.function.BiConsumer;
import java.util.function.BiFunction;
import java.util.function.Function;
import java.util.function.Predicate;

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
 * <p>The same steps make a powsybl call that takes several values at once (a {@link PendingCall}),
 * which separate setter calls could not do without going through an invalid state:
 * {@code invoke(regulation, REL_TERMINAL)} starts {@code VoltageRegulation.setTerminal(terminal,
 * targetValue)}, whose arguments are then set with REL_TERMINAL and VR_TARGET_VALUE, and ACT_ADD
 * makes it. So do moving an equipment's end ({@code invoke(terminal, MOVE_CONNECTABLE)}), cloning a
 * variant, and the parts of an object that powsybl adds with one call (a plane of a reactive
 * capability shape). A query that takes several values goes the same way, ACT_ADD returning its
 * result: the checks of the temporary limits of a side.</p>
 */
public final class Creation {

    /** Source format of the networks created from C++. */
    public static final String SOURCE_FORMAT = "iidm4cpp";

    private Creation() {}

    /**
     * A powsybl adder (or builder) handed to C++, with what it belongs to: an object, or a parent adder.
     * COPY_OF restarts it from powsybl's adder of a copy (see {@link #copyAdder}).
     */
    static final class PendingAdd {
        private Object adder;
        private final Object parent;

        PendingAdd(Object adder, Object parent) {
            this.adder = adder;
            this.parent = parent;
        }

        Object adder() {
            return adder;
        }

        Object parent() {
            return parent;
        }
    }

    /**
     * A powsybl call that takes several values at once, made in steps like an adder: its arguments
     * are its fields, set one by one by code, and ACT_ADD makes it and returns its result (null for
     * none). A field set several times is a list. A code that is not one of its fields fails at once,
     * as an unknown field; a value of another type, when the call is made.
     */
    static final class PendingCall {
        private final Set<Integer> fields;
        private final Function<PendingCall, Object> call;
        private final Map<Integer, List<Object>> values = new HashMap<>();

        private PendingCall(Set<Integer> fields, Function<PendingCall, Object> call) {
            this.fields = fields;
            this.call = call;
        }

        void set(String type, int field, Object value) {
            if (!fields.contains(field)) {
                throw unknownField(type, field);
            }
            values.computeIfAbsent(field, f -> new ArrayList<>()).add(value);
        }

        /** This call, which a nested one adds to: an unknown field when it does not have the field. */
        PendingCall takes(int field) {
            if (!fields.contains(field)) {
                throw unknownField("nested", field);
            }
            return this;
        }

        Object make() {
            return call.apply(this);
        }

        /** The last value set; null when none was. */
        @SuppressWarnings("unchecked")
        <T> T get(int field) {
            List<Object> set = values.get(field);
            return set == null ? null : (T) set.get(set.size() - 1);
        }

        <T> T required(int field, String name) {
            return Objects.requireNonNull(get(field), name);
        }

        /** NaN when unset. */
        double dbl(int field) {
            Double value = get(field);
            return value == null ? Double.NaN : value;
        }

        /** 0 when unset. */
        int integer(int field) {
            Integer value = get(field);
            return value == null ? 0 : value;
        }

        /** False when unset. */
        boolean bool(int field) {
            Boolean value = get(field);
            return value != null && value;
        }

        /** Every value set, in order: a ClassCastException for one of another type, before the call. */
        <T> List<T> list(int field, Class<T> type) {
            List<T> list = new ArrayList<>();
            for (Object value : values.getOrDefault(field, List.of())) {
                list.add(type.cast(value));
            }
            return list;
        }
    }

    private static Set<Integer> fields(Integer... codes) {
        return Set.of(codes);
    }

    /** A call on the target, whose result ACT_ADD returns. */
    private static <T> PendingCall call(T target, Set<Integer> fields, BiFunction<T, PendingCall, Object> call) {
        return new PendingCall(fields, c -> call.apply(target, c));
    }

    /** A call on the target without a result. */
    private static <T> PendingCall run(T target, Set<Integer> fields, BiConsumer<T, PendingCall> call) {
        return new PendingCall(fields, c -> {
            call.accept(target, c);
            return null;
        });
    }

    /** A new ThreeWindingsTransformerFortescue: EXT_THREE_WTF_LEG selects the leg the leg fields set. */
    static final class FortescueLegs {
        private final ThreeWindingsTransformerFortescueAdder adder;
        private ThreeWindingsTransformerFortescueAdder.LegFortescueAdder leg;

        FortescueLegs(ThreeWindingsTransformerFortescueAdder adder) {
            this.adder = adder;
        }

        void select(int side) {
            leg = switch (enumValue(ThreeSides.class, side)) {
                case ONE -> adder.leg1();
                case TWO -> adder.leg2();
                case THREE -> adder.leg3();
            };
        }

        ThreeWindingsTransformerFortescueAdder.LegFortescueAdder leg() {
            if (leg == null) {
                throw new IllegalStateException("No leg selected: leg1(), leg2() or leg3() first");
            }
            return leg;
        }
    }

    /** A leg of a new three windings transformer: LEG_SIDE selects newLeg1(), newLeg2() or newLeg3(), then its fields. */
    static final class NewLeg {
        private final ThreeWindingsTransformerAdder adder;
        private ThreeWindingsTransformerAdder.LegAdder leg;

        NewLeg(ThreeWindingsTransformerAdder adder) {
            this.adder = adder;
        }

        void select(int side) {
            leg = switch (enumValue(ThreeSides.class, side)) {
                case ONE -> adder.newLeg1();
                case TWO -> adder.newLeg2();
                case THREE -> adder.newLeg3();
            };
        }

        ThreeWindingsTransformerAdder.LegAdder leg() {
            if (leg == null) {
                throw new IllegalStateException("No side selected: LEG_SIDE first");
            }
            return leg;
        }
    }

    /** powsybl's adder of a copy of the object (Network.newLine(copied)...), on the parent of a pending adder. */
    private static Object copyAdder(Object parent, Object copied) {
        Object owner = parent instanceof PendingAdd p ? p.adder() : parent;
        return switch (copied) {
            case Line line -> ((Network) owner).newLine(line);
            case TwoWindingsTransformer transformer -> ((Substation) owner).newTwoWindingsTransformer(transformer);
            case RatioTapChanger tapChanger -> ((RatioTapChangerHolder) owner).newRatioTapChanger(tapChanger);
            case PhaseTapChanger tapChanger -> ((PhaseTapChangerHolder) owner).newPhaseTapChanger(tapChanger);
            case ReactiveCapabilityCurve curve -> ((ReactiveLimitsHolder) owner).newReactiveCapabilityCurve(curve);
            case CurrentLimits limits -> ((OperationalLimitsGroup) owner).newCurrentLimits(limits);
            case ActivePowerLimits limits -> ((OperationalLimitsGroup) owner).newActivePowerLimits(limits);
            case ApparentPowerLimits limits -> ((OperationalLimitsGroup) owner).newApparentPowerLimits(limits);
            default -> throw new BridgeErrors.PropertyNotFoundException("No copy of " + copied);
        };
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
            case REL_BBV_SWITCH -> ((VoltageLevel) owner).getBusBreakerView().newSwitch();
            case REL_NBV_SWITCH -> ((VoltageLevel) owner).getNodeBreakerView().newSwitch();
            case BUSBAR_SECTION -> ((VoltageLevel) owner).getNodeBreakerView().newBusbarSection();
            case INTERNAL_CONNECTION -> ((VoltageLevel) owner).getNodeBreakerView().newInternalConnection();
            case GENERATOR -> ((VoltageLevel) owner).newGenerator();
            case LOAD -> ((VoltageLevel) owner).newLoad();
            case GROUND -> ((VoltageLevel) owner).newGround();
            case BATTERY -> ((VoltageLevel) owner).newBattery();
            case SHUNT_COMPENSATOR -> ((VoltageLevel) owner).newShuntCompensator();
            case SHUNT_COMPENSATOR_LINEAR_MODEL -> ((ShuntCompensatorAdder) owner).newLinearModel();
            case SHUNT_COMPENSATOR_NON_LINEAR_MODEL -> ((ShuntCompensatorAdder) owner).newNonLinearModel();
            case SHUNT_SECTION -> ((ShuntCompensatorNonLinearModelAdder) owner).beginSection();
            case STATIC_VAR_COMPENSATOR -> ((VoltageLevel) owner).newStaticVarCompensator();
            case VSC_CONVERTER_STATION -> ((VoltageLevel) owner).newVscConverterStation();
            case LCC_CONVERTER_STATION -> ((VoltageLevel) owner).newLccConverterStation();
            case BOUNDARY_LINE -> ((VoltageLevel) owner).newBoundaryLine();
            case BOUNDARY_LINE_GENERATION -> ((BoundaryLineAdder) owner).newGeneration();
            case TIE_LINE -> ((Network) owner).newTieLine();
            case LINE -> ((Network) owner).newLine();
            case HVDC_LINE -> ((Network) owner).newHvdcLine();
            case TWO_WINDINGS_TRANSFORMER -> ((Substation) owner).newTwoWindingsTransformer();
            case THREE_WINDINGS_TRANSFORMER -> ((Substation) owner).newThreeWindingsTransformer();
            case THREE_WINDINGS_TRANSFORMER_LEG -> new NewLeg((ThreeWindingsTransformerAdder) owner);
            case AREA -> ((Network) owner).newArea();
            case AREA_BOUNDARY -> owner instanceof AreaAdder areaAdder
                ? run(areaAdder, fields(REL_TERMINAL, BOUNDARY, AREA_BOUNDARY_AC), Creation::addAreaBoundary)
                : ((Area) owner).newAreaBoundary();
            case VOLTAGE_ANGLE_LIMIT -> ((Network) owner).newVoltageAngleLimit();
            case OVERLOAD_MANAGEMENT_SYSTEM -> ((Substation) owner).newOverloadManagementSystem();
            case VARIANT_CLONE -> run((VariantManager) owner, fields(VARIANT_SOURCE_ID, VARIANT_TARGET_ID, VARIANT_MAY_OVERWRITE),
                (manager, c) -> manager.cloneVariant(c.required(VARIANT_SOURCE_ID, "source variant id"),
                    c.list(VARIANT_TARGET_ID, String.class), c.bool(VARIANT_MAY_OVERWRITE)));
            case NETWORK -> call((Network) owner, fields(ID, NAME, NETWORK_SOURCE_FORMAT),
                (network, c) -> network.createSubnetwork(c.get(ID), c.get(NAME), c.get(NETWORK_SOURCE_FORMAT)));
            case BOUNDARY_ELEMENT -> call((Network) owner, fields(IDENTIFIABLE), (network, c) -> {
                Identifiable<?> identifiable = c.required(IDENTIFIABLE, "identifiable");
                return network.isBoundaryElement(identifiable) ? identifiable : null;
            });
            case THREE_WINDINGS_TRANSFORMER_FORTESCUE ->
                new FortescueLegs((ThreeWindingsTransformerFortescueAdder) Extensions.newAdder(owner, type));
            case SUBSTATION_POSITION, LINE_POSITION -> call(Extensions.newAdder(owner, type), fields(COORDINATE), Creation::addPosition);
            case COORDINATE -> run(((PendingCall) owner).takes(COORDINATE), fields(COORD_LATITUDE, COORD_LONGITUDE), (position, c) ->
                position.set("related object", COORDINATE, new Coordinate(c.dbl(COORD_LATITUDE), c.dbl(COORD_LONGITUDE))));
            case FEEDER -> run((ConnectablePositionAdder<?>) owner,
                fields(EXT_FEEDER_SLOT, EXT_FEEDER_NAME, EXT_FEEDER_ORDER, EXT_FEEDER_DIRECTION), Creation::addFeeder);
            case AREA_CHARACTERISTICS -> run((ObservabilityAreaAdder) owner,
                fields(OBS_AREA_NUMBER, OBS_AREA_STATUS, OBS_AREA_BUS_BREAKER_BUS_ID, OBS_AREA_BUS_ID, OBS_AREA_NODE),
                Creation::addObservabilityArea);
            case REFERENCE_PRIORITY -> ((ReferencePriorities<?>) owner).newReferencePriority();
            case EXT_RP_PRIORITY -> run(owner, fields(TERMINAL_SIDE, EXT_RP_PRIORITY),
                (connectable, c) -> setPriority(connectable, c.integer(TERMINAL_SIDE), c.integer(EXT_RP_PRIORITY)));
            case REFERENCE_TERMINALS -> call((ReferenceTerminalsAdder) Extensions.newAdder(owner, type), fields(ADD_REFERENCE_TERMINAL),
                (terminals, c) -> terminals.withTerminals(new LinkedHashSet<>(c.list(ADD_REFERENCE_TERMINAL, Terminal.class))).add());
            case TERMINAL -> run((ReferenceTerminals) owner, fields(ADD_REFERENCE_TERMINAL),
                (terminals, c) -> terminals.setReferenceTerminals(new LinkedHashSet<>(c.list(ADD_REFERENCE_TERMINAL, Terminal.class))));
            case CONTROL_ZONE -> ((SecondaryVoltageControlAdder) owner).newControlZone();
            case PILOT_POINT -> run(((ControlZoneAdder) owner).newPilotPoint(), fields(EXT_SVC_TARGET_V, EXT_SVC_PILOT_POINT_IDS),
                (pilotPoint, c) -> pilotPoint.withTargetV(c.dbl(EXT_SVC_TARGET_V))
                    .withBusbarSectionsOrBusesIds(c.list(EXT_SVC_PILOT_POINT_IDS, String.class)).add());
            case CONTROL_UNIT -> ((ControlZoneAdder) owner).newControlUnit();
            case MUTUAL_COUPLING -> ((LineCouplings) owner).newMutualCoupling();
            case EXT_MC_LINE1_SEGMENT, EXT_MC_LINE2_SEGMENT -> run(owner instanceof MutualCouplingAdder ? owner : (MutualCoupling) owner,
                fields(EXT_MC_SEGMENT_START, EXT_MC_SEGMENT_END), (target, c) -> setSegment(target, type == EXT_MC_LINE1_SEGMENT,
                    new LineSegment(c.dbl(EXT_MC_SEGMENT_START), c.dbl(EXT_MC_SEGMENT_END))));
            case FIND_MUTUAL_COUPLING -> call((LineCouplings) owner, fields(EXT_MC_LINE1, EXT_MC_LINE2),
                (couplings, c) -> couplings.findMutualCoupling(c.get(EXT_MC_LINE1), c.get(EXT_MC_LINE2)).orElse(null));
            case REMOVE_MUTUAL_COUPLING -> call((LineCouplings) owner, fields(MUTUAL_COUPLING, EXT_MC_LINE1, EXT_MC_LINE2),
                Creation::removeMutualCoupling);
            case SWITCH_TRIPPING -> ((OverloadManagementSystemAdder) owner).newSwitchTripping();
            case BRANCH_TRIPPING -> ((OverloadManagementSystemAdder) owner).newBranchTripping();
            case THREE_WINDINGS_TRANSFORMER_TRIPPING -> ((OverloadManagementSystemAdder) owner).newThreeWindingsTransformerTripping();
            case VOLTAGE_REGULATION -> owner instanceof VoltageRegulationHolderAdder<?> holderAdder
                ? holderAdder.newVoltageRegulation()
                : ((VoltageRegulationHolder<?>) owner).newVoltageRegulation();
            case MEASUREMENT -> ((Measurements<?>) owner).newMeasurement();
            case DISCRETE_MEASUREMENT -> ((DiscreteMeasurements<?>) owner).newDiscreteMeasurement();
            case REL_TERMINAL -> run((VoltageRegulation) owner, fields(REL_TERMINAL, VR_TARGET_VALUE),
                (regulation, c) -> regulation.setTerminal(c.get(REL_TERMINAL), c.dbl(VR_TARGET_VALUE)));
            case OPERATIONAL_LIMITS_GROUP -> call(LimitsDispatch.holder(owner), fields(OLG_ID),
                (holder, c) -> holder.newOperationalLimitsGroup(c.required(OLG_ID, "group id")));
            case REL_SELECTED_OLG -> call(LimitsDispatch.holder(owner), fields(OLG_ID), (holder, c) -> {
                String id = c.get(OLG_ID);
                return id == null ? holder.getOrCreateSelectedOperationalLimitsGroup()
                                  : holder.getOrCreateSelectedOperationalLimitsGroup(id);
            });
            case CURRENT_LIMITS -> ((OperationalLimitsGroup) owner).newCurrentLimits();
            case ACTIVE_POWER_LIMITS -> ((OperationalLimitsGroup) owner).newActivePowerLimits();
            case APPARENT_POWER_LIMITS -> ((OperationalLimitsGroup) owner).newApparentPowerLimits();
            case TEMPORARY_LIMIT -> ((LoadingLimitsAdder<?, ?>) owner).beginTemporaryLimit();
            case LL_TEMPORARY_LIMIT_VALUE -> run((LoadingLimits) owner, fields(TL_ACCEPTABLE_DURATION, TL_VALUE), (limits, c) ->
                limits.setTemporaryLimitValue(c.required(TL_ACCEPTABLE_DURATION, "acceptable duration"), c.dbl(TL_VALUE)));
            case TC_STEPS_REPLACER -> PropertyDispatcher.tapChanger(owner).stepsReplacer();
            case RATIO_TAP_CHANGER -> ((RatioTapChangerHolder) owner).newRatioTapChanger();
            case PHASE_TAP_CHANGER -> ((PhaseTapChangerHolder) owner).newPhaseTapChanger();
            case RATIO_TAP_CHANGER_STEP -> owner instanceof RatioTapChangerAdder ratioAdder
                ? ratioAdder.beginStep()
                : ((RatioTapChangerStepsReplacer) owner).beginStep();
            case PHASE_TAP_CHANGER_STEP -> owner instanceof PhaseTapChangerAdder phaseAdder
                ? phaseAdder.beginStep()
                : ((PhaseTapChangerStepsReplacer) owner).beginStep();
            case MIN_MAX_REACTIVE_LIMITS -> ((ReactiveLimitsHolder) owner).newMinMaxReactiveLimits();
            case REACTIVE_CAPABILITY_CURVE -> ((ReactiveLimitsHolder) owner).newReactiveCapabilityCurve();
            case REACTIVE_CURVE_POINT -> ((ReactiveCapabilityCurveAdder) owner).beginPoint();
            case REACTIVE_CAPABILITY_SHAPE -> ((ReactiveLimitsHolder) owner).newReactiveCapabilityShape();
            case REACTIVE_CAPABILITY_SHAPE_PLANE -> run((ReactiveCapabilityShapeAdder) owner,
                fields(PLANE_ALPHA, PLANE_BETA, PLANE_DELTA, PLANE_GREATER_OR_EQUAL, PLANE_GAMMA), (shape, c) -> shape.addPlane(
                    c.dbl(PLANE_ALPHA), c.dbl(PLANE_BETA), c.dbl(PLANE_DELTA), c.bool(PLANE_GREATER_OR_EQUAL), c.dbl(PLANE_GAMMA)));
            case ZIP_LOAD_MODEL -> ((LoadAdder) owner).newZipModel();
            case EXPONENTIAL_LOAD_MODEL -> ((LoadAdder) owner).newExponentialModel();
            case CONNECT, DISCONNECT -> call(owner, fields(SWITCH_PREDICATE, TERMINAL_SIDE), (target, c) ->
                PropertyDispatcher.connect(target, type == CONNECT,
                    switchPredicate(c.required(SWITCH_PREDICATE, "switch predicate")), c.get(TERMINAL_SIDE)) ? target : null);
            case MOVE_CONNECTABLE -> run((Terminal) owner,
                fields(TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, TERMINAL_CONNECTED, TERMINAL_NODE, TERMINAL_VOLTAGE_LEVEL_ID),
                Creation::moveConnectable);
            case FICTITIOUS_P0 -> run(((VoltageLevel) owner).getNodeBreakerView(), fields(TERMINAL_NODE, FICTITIOUS_P0),
                (view, c) -> view.setFictitiousP0(c.required(TERMINAL_NODE, "node"), c.dbl(FICTITIOUS_P0)));
            case FICTITIOUS_Q0 -> run(((VoltageLevel) owner).getNodeBreakerView(), fields(TERMINAL_NODE, FICTITIOUS_Q0),
                (view, c) -> view.setFictitiousQ0(c.required(TERMINAL_NODE, "node"), c.dbl(FICTITIOUS_Q0)));
            case VL_NBV_REMOVE_INTERNAL_CONNECTIONS -> run(((VoltageLevel) owner).getNodeBreakerView(), fields(IC_NODE1, IC_NODE2),
                (view, c) -> view.removeInternalConnections(c.required(IC_NODE1, "node1"), c.required(IC_NODE2, "node2")));
            case CHECK_TEMPORARY_LIMITS, CHECK_ALL_TEMPORARY_LIMITS -> call(PropertyDispatcher.overloadChecked(owner),
                fields(TERMINAL_SIDE, LIMIT_REDUCTION, LL_LIMIT_TYPE),
                (checked, c) -> checkTemporaryLimits(checked, type == CHECK_ALL_TEMPORARY_LIMITS, c));
            default -> {
                // An extension type code: a new extension of that type
                if (!Extensions.isType(type)) {
                    throw new BridgeErrors.PropertyNotFoundException("Unknown type to create: " + type);
                }
                yield Extensions.newAdder(owner, type);
            }
        };
        return new PendingAdd(adder, parent);
    }

    // ── Calls made in steps (PendingCall) ─────────────────────────────────

    /** One boundary of a new area: AreaAdder.addAreaBoundary(terminal or boundary, ac). */
    private static void addAreaBoundary(AreaAdder adder, PendingCall c) {
        boolean ac = c.required(AREA_BOUNDARY_AC, "ac");
        Terminal terminal = c.get(REL_TERMINAL);
        if (terminal != null) {
            adder.addAreaBoundary(terminal, ac);
        } else {
            adder.addAreaBoundary(c.<Boundary>required(BOUNDARY, "terminal or boundary"), ac);
        }
    }

    /** A new LinePosition, or SubstationPosition (its last coordinate): its coordinates are nested calls. */
    private static Object addPosition(Object adder, PendingCall c) {
        List<Coordinate> coordinates = c.list(COORDINATE, Coordinate.class);
        if (adder instanceof LinePositionAdder<?> line) {
            return line.withCoordinates(coordinates).add();
        }
        SubstationPositionAdder substation = (SubstationPositionAdder) adder;
        if (!coordinates.isEmpty()) {
            substation.withCoordinate(coordinates.get(coordinates.size() - 1));
        }
        return substation.add();
    }

    /** A feeder of a new ConnectablePosition: newFeeder() (slot 0) or newFeeder1() to newFeeder3(). */
    private static void addFeeder(ConnectablePositionAdder<?> adder, PendingCall c) {
        int slot = c.integer(EXT_FEEDER_SLOT);
        ConnectablePositionAdder.FeederAdder<?> feeder = switch (slot) {
            case 0 -> adder.newFeeder();
            case 1 -> adder.newFeeder1();
            case 2 -> adder.newFeeder2();
            case 3 -> adder.newFeeder3();
            default -> throw new IllegalArgumentException("Invalid feeder slot: " + slot);
        };
        String name = c.get(EXT_FEEDER_NAME);
        Integer order = c.get(EXT_FEEDER_ORDER);
        Integer direction = c.get(EXT_FEEDER_DIRECTION);
        if (name != null) feeder.withName(name);
        if (order != null) feeder.withOrder(order);
        if (direction != null) feeder.withDirection(enumValue(ConnectablePosition.Direction.class, direction));
        feeder.add();
    }

    /** An area of a new ObservabilityArea: by bus-breaker view buses, by a bus view bus, or by nodes. */
    private static void addObservabilityArea(ObservabilityAreaAdder adder, PendingCall c) {
        int number = c.integer(OBS_AREA_NUMBER);
        Integer ordinal = c.get(OBS_AREA_STATUS);
        ObservabilityArea.ObservabilityStatus status = ordinal == null ? null
            : enumValue(ObservabilityArea.ObservabilityStatus.class, ordinal);
        String busId = c.get(OBS_AREA_BUS_ID);
        Set<Integer> nodes = new LinkedHashSet<>(c.list(OBS_AREA_NODE, Integer.class));
        if (busId != null) {
            adder.withObservabilityAreaByBusViewBus(busId, number, status);
        } else if (!nodes.isEmpty()) {
            adder.withObservabilityAreaByNodes(nodes, number, status);
        } else {
            Set<String> busIds = new LinkedHashSet<>(c.list(OBS_AREA_BUS_BREAKER_BUS_ID, String.class));
            adder.withObservabilityAreaByBusBreakerViewBuses(busIds, number, status);
        }
    }

    /** ReferencePriority.set of an injection, or of a side of a branch or of a three windings transformer. */
    private static void setPriority(Object connectable, int side, int priority) {
        switch (connectable) {
            case Injection<?> injection -> ReferencePriority.set(injection, priority);
            case Branch<?> branch -> ReferencePriority.set(branch, enumValue(TwoSides.class, side), priority);
            default -> ReferencePriority.set((ThreeWindingsTransformer) connectable, enumValue(ThreeSides.class, side), priority);
        }
    }

    /**
     * Moves the end of an equipment: to a bus, Terminal.BusBreakerView.moveConnectable(busId,
     * connected); to a node when the node is set, Terminal.NodeBreakerView.moveConnectable(node,
     * voltageLevelId).
     */
    private static void moveConnectable(Terminal terminal, PendingCall c) {
        Integer node = c.get(TERMINAL_NODE);
        if (node != null) {
            terminal.getNodeBreakerView().moveConnectable(node, c.required(TERMINAL_VOLTAGE_LEVEL_ID, "voltage level id"));
        } else {
            terminal.getBusBreakerView().moveConnectable(c.required(TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, "bus id"),
                c.<Boolean>required(TERMINAL_CONNECTED, "connected"));
        }
    }

    /** The segment of a line of a mutual coupling, or of a new one: its start and end, set at once. */
    private static void setSegment(Object target, boolean line1, LineSegment segment) {
        switch (target) {
            case MutualCouplingAdder adder when line1 -> adder.withLine1Segment(segment);
            case MutualCouplingAdder adder -> adder.withLine2Segment(segment);
            case MutualCoupling coupling when line1 -> coupling.setLine1Segment(segment);
            default -> ((MutualCoupling) target).setLine2Segment(segment);
        }
    }

    /** LineCouplings.removeMutualCoupling(coupling), or (line1, line2): the extension when one was removed, else null. */
    private static Object removeMutualCoupling(LineCouplings couplings, PendingCall c) {
        MutualCoupling coupling = c.get(MUTUAL_COUPLING);
        boolean removed = coupling != null ? couplings.removeMutualCoupling(coupling)
                                           : couplings.removeMutualCoupling(c.<Line>get(EXT_MC_LINE1), c.<Line>get(EXT_MC_LINE2));
        return removed ? couplings : null;
    }

    /**
     * checkTemporaryLimits, or checkAllTemporaryLimits, of a side of a branch or a three windings
     * transformer: fields TERMINAL_SIDE, LIMIT_REDUCTION (none: the check without one) and
     * LL_LIMIT_TYPE. Its result is the overload (null without one), or the overloads.
     */
    private static Object checkTemporaryLimits(Object checked, boolean all, PendingCall c) {
        int side = c.required(TERMINAL_SIDE, "side");
        LimitType type = enumValue(LimitType.class, c.required(LL_LIMIT_TYPE, "limit type"));
        Double reduction = c.get(LIMIT_REDUCTION);
        if (checked instanceof ThreeWindingsTransformer twt) {
            ThreeSides threeSides = enumValue(ThreeSides.class, side);
            if (all) {
                return reduction == null ? twt.checkAllTemporaryLimits(threeSides, type)
                                         : twt.checkAllTemporaryLimits(threeSides, reduction, type);
            }
            return reduction == null ? twt.checkTemporaryLimits(threeSides, type)
                                     : twt.checkTemporaryLimits(threeSides, reduction, type);
        }
        Branch<?> branch = (Branch<?>) checked;
        TwoSides twoSides = enumValue(TwoSides.class, side);
        if (all) {
            return reduction == null ? branch.checkAllTemporaryLimits(twoSides, type)
                                     : branch.checkAllTemporaryLimits(twoSides, reduction, type);
        }
        return reduction == null ? branch.checkTemporaryLimits(twoSides, type)
                                 : branch.checkTemporaryLimits(twoSides, reduction, type);
    }

    /** Adds what an adder describes and returns it; null for a nested adder (its parent goes on). */
    static Object add(Object obj) {
        if (!(obj instanceof PendingAdd pending)) {
            throw new BridgeErrors.PropertyNotFoundException("Not an adder: " + obj);
        }
        return switch (pending.adder()) {
            case PendingCall call -> call.make();
            case IdentifiableAdder<?, ?> adder -> adder.add();
            case ExtensionAdder<?, ?> adder -> adder.add();
            case MeasurementAdder adder -> adder.add();
            case DiscreteMeasurementAdder adder -> adder.add();
            case VoltageRegulationBuilder builder -> builder.build();
            case VoltageRegulationAdder<?> adder -> {
                adder.add();
                yield null;
            }
            case OperationalLimitsAdder<?, ?> adder -> adder.add();
            case LoadingLimitsAdder.TemporaryLimitAdder<?> adder -> {
                adder.endTemporaryLimit();
                yield null;
            }
            case FortescueLegs legs -> legs.adder.add();
            case ReferencePriorityAdder adder -> adder.add();
            case ControlZoneAdder adder -> {
                adder.add();
                yield null;
            }
            case ControlUnitAdder adder -> {
                adder.add();
                yield null;
            }
            case MutualCouplingAdder adder -> adder.add();
            case AreaBoundaryAdder adder -> adder.add(); // the area
            case OverloadManagementSystemAdder.TrippingAdder<?> adder -> {
                adder.add();
                yield null;
            }
            case RatioTapChangerAdder adder -> adder.add();
            case PhaseTapChangerAdder adder -> adder.add();
            case MinMaxReactiveLimitsAdder adder -> adder.add();
            case ReactiveCapabilityCurveAdder adder -> adder.add();
            case ReactiveCapabilityCurveAdder.PointAdder point -> {
                point.endPoint();
                yield null;
            }
            case ReactiveCapabilityShapeAdder adder -> adder.add();
            case TapChangerStepAdder<?, ?> step -> {
                step.endStep();
                yield null;
            }
            case TapChangerStepsReplacer<?, ?> replacer -> {
                replacer.replaceSteps();
                yield null;
            }
            case NewLeg leg -> {
                leg.leg().add();
                yield null;
            }
            case VoltageLevel.NodeBreakerView.InternalConnectionAdder adder -> {
                adder.add();
                yield null;
            }
            case ShuntCompensatorLinearModelAdder adder -> {
                adder.add();
                yield null;
            }
            case ShuntCompensatorNonLinearModelAdder adder -> {
                adder.add();
                yield null;
            }
            case ShuntCompensatorNonLinearModelAdder.SectionAdder section -> {
                section.endSection();
                yield null;
            }
            case BoundaryLineAdder.GenerationAdder generation -> {
                generation.add();
                yield null;
            }
            case ZipLoadModelAdder adder -> {
                adder.add();
                yield null;
            }
            case ExponentialLoadModelAdder adder -> {
                adder.add();
                yield null;
            }
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
            case TieLine tieLine -> tieLine.remove();
            case Area area -> area.remove();
            case AutomationSystem<?> system -> system.remove();
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
            case REL_SELECTED_OLG -> LimitsDispatch.holder(host).cancelSelectedOperationalLimitsGroup();
            case CURRENT_LIMITS -> ((OperationalLimitsGroup) host).removeCurrentLimits();
            case ACTIVE_POWER_LIMITS -> ((OperationalLimitsGroup) host).removeActivePowerLimits();
            case APPARENT_POWER_LIMITS -> ((OperationalLimitsGroup) host).removeApparentPowerLimits();
            default -> {
                if (!Extensions.isType(code)) {
                    throw new BridgeErrors.PropertyNotFoundException("Unknown code to clear: " + code);
                }
                Extensions.remove(host, code);
            }
        }
    }

    // ── Adder fields: the codes of the object's own properties ────────────

    static void setDouble(PendingAdd pending, int property, double value) {
        if (pending.adder() instanceof PendingCall call) {
            call.set("double", property, value);
            return;
        }
        PropertyTable.DoubleProperty table = PropertyTable.doubleProperty(property);
        if (table != null && table.add() != null) {
            table.add().accept(pending.adder(), value);
            return;
        }
        Object a = pending.adder();
        switch (property) {
            case LEG_R -> ((NewLeg) a).leg().setR(value);
            case LEG_X -> ((NewLeg) a).leg().setX(value);
            case LEG_G -> ((NewLeg) a).leg().setG(value);
            case LEG_B -> ((NewLeg) a).leg().setB(value);
            case LEG_RATED_U -> ((NewLeg) a).leg().setRatedU(value);
            case LEG_RATED_S -> ((NewLeg) a).leg().setRatedS(value);
            case EXT_LEG_F_RZ -> ((FortescueLegs) a).leg().withRz(value);
            case EXT_LEG_F_XZ -> ((FortescueLegs) a).leg().withXz(value);
            case EXT_LEG_F_GROUNDING_R -> ((FortescueLegs) a).leg().withGroundingR(value);
            case EXT_LEG_F_GROUNDING_X -> ((FortescueLegs) a).leg().withGroundingX(value);
            default -> throw unknownField("double", property);
        }
    }

    static void setInt(PendingAdd pending, int property, int value) {
        if (pending.adder() instanceof PendingCall call) {
            call.set("int", property, value);
            return;
        }
        PropertyTable.IntProperty table = PropertyTable.intProperty(property);
        if (table != null && table.add() != null) {
            table.add().accept(pending.adder(), value);
            return;
        }
        Object a = pending.adder();
        switch (property) {
            case TERMINAL_NODE -> {
                switch (a) {
                    case NewLeg leg -> leg.leg().setNode(value);
                    case BusbarSectionAdder adder -> adder.setNode(value);
                    default -> ((InjectionAdder<?, ?>) a).setNode(value);
                }
            }
            case TRIPPING_SIDE -> {
                if (a instanceof OverloadManagementSystemAdder.BranchTrippingAdder adder) {
                    adder.setSideToOperate(enumValue(TwoSides.class, value));
                } else {
                    ((OverloadManagementSystemAdder.ThreeWindingsTransformerTrippingAdder) a)
                        .setSideToOperate(enumValue(ThreeSides.class, value));
                }
            }
            case PTC_REGULATION_MODE -> ((PhaseTapChangerAdder) a)
                .setRegulationMode(enumValue(PhaseTapChanger.RegulationMode.class, value));
            case LEG_SIDE -> ((NewLeg) a).select(value);
            case VR_MODE -> ((VoltageRegulationAdderOrBuilder<?>) a).withMode(value < 0 ? null : enumValue(RegulationMode.class, value));
            case OBS_QUALITY_P  -> ((InjectionObservabilityAdder<?>) a).withRedundantP(redundant(value));
            case OBS_QUALITY_Q  -> ((InjectionObservabilityAdder<?>) a).withRedundantQ(redundant(value));
            case OBS_QUALITY_V  -> ((InjectionObservabilityAdder<?>) a).withRedundantV(redundant(value));
            case OBS_QUALITY_P1 -> ((BranchObservabilityAdder<?>) a).withRedundantP1(redundant(value));
            case OBS_QUALITY_P2 -> ((BranchObservabilityAdder<?>) a).withRedundantP2(redundant(value));
            case OBS_QUALITY_Q1 -> ((BranchObservabilityAdder<?>) a).withRedundantQ1(redundant(value));
            case OBS_QUALITY_Q2 -> ((BranchObservabilityAdder<?>) a).withRedundantQ2(redundant(value));
            case MEAS_SIDE -> ((MeasurementAdder) a).setSide(value < 0 ? null : enumValue(ThreeSides.class, value));
            case DMEAS_TAP_CHANGER -> ((DiscreteMeasurementAdder) a)
                .setTapChanger(value < 0 ? null : enumValue(DiscreteMeasurement.TapChanger.class, value));
            case EXT_TWO_WTF_CONNECTION_TYPE1 -> ((TwoWindingsTransformerFortescueAdder) a)
                .withConnectionType1(enumValue(WindingConnectionType.class, value));
            case EXT_TWO_WTF_CONNECTION_TYPE2 -> ((TwoWindingsTransformerFortescueAdder) a)
                .withConnectionType2(enumValue(WindingConnectionType.class, value));
            case EXT_LEG_F_CONNECTION_TYPE -> ((FortescueLegs) a).leg().withConnectionType(enumValue(WindingConnectionType.class, value));
            default -> throw unknownField("int", property);
        }
    }

    static void setBool(PendingAdd pending, int property, boolean value) {
        if (pending.adder() instanceof PendingCall call) {
            call.set("bool", property, value);
            return;
        }
        PropertyTable.BoolProperty table = PropertyTable.boolProperty(property);
        if (table != null && table.add() != null) {
            table.add().accept(pending.adder(), value);
            return;
        }
        Object a = pending.adder();
        switch (property) {
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
            case SW_OPEN -> {
                if (a instanceof VoltageLevel.NodeBreakerView.SwitchAdder adder) {
                    adder.setOpen(value);
                } else {
                    ((VoltageLevel.BusBreakerView.SwitchAdder) a).setOpen(value);
                }
            }
            case EXT_OBS_OBSERVABLE -> {
                if (a instanceof BranchObservabilityAdder<?> branch) branch.withObservable(value);
                else ((InjectionObservabilityAdder<?>) a).withObservable(value);
            }
            case EXT_LEG_F_FREE_FLUXES -> ((FortescueLegs) a).leg().withFreeFluxes(value);
            default -> throw unknownField("bool", property);
        }
    }

    static void setString(PendingAdd pending, int property, String value) {
        if (pending.adder() instanceof PendingCall call) {
            call.set("string", property, value);
            return;
        }
        PropertyTable.StringProperty table = PropertyTable.stringProperty(property);
        if (table != null && table.add() != null) {
            table.add().accept(pending.adder(), value);
            return;
        }
        Object a = pending.adder();
        switch (property) {
            case COUNTRY -> ((SubstationAdder) a).setCountry(Country.valueOf(value));
            case TERMINAL_BUS_BREAKER_BUS_ID -> {
                if (a instanceof NewLeg leg) {
                    leg.leg().setBus(value);
                } else {
                    ((InjectionAdder<?, ?>) a).setBus(value);
                }
            }
            case TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID -> {
                if (a instanceof NewLeg leg) {
                    leg.leg().setConnectableBus(value);
                } else {
                    ((InjectionAdder<?, ?>) a).setConnectableBus(value);
                }
            }
            case TERMINAL_VOLTAGE_LEVEL_ID -> ((NewLeg) a).leg().setVoltageLevel(value);
            case TRIPPING_ELEMENT_ID -> {
                switch (a) {
                    case OverloadManagementSystemAdder.SwitchTrippingAdder adder -> adder.setSwitchToOperateId(value);
                    case OverloadManagementSystemAdder.BranchTrippingAdder adder -> adder.setBranchToOperateId(value);
                    default -> ((OverloadManagementSystemAdder.ThreeWindingsTransformerTrippingAdder) a)
                        .setThreeWindingsTransformerToOperateId(value);
                }
            }
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
        if (pending.adder() instanceof PendingCall call) {
            call.set("related object", relation, value);
            return;
        }
        Object a = pending.adder();
        switch (relation) {
            case REL_TERMINAL -> {
                switch (a) {
                    case SlackTerminalAdder adder -> adder.withTerminal((Terminal) value);
                    case AreaBoundaryAdder adder -> adder.setTerminal((Terminal) value);
                    case ReferencePriorityAdder adder -> adder.setTerminal((Terminal) value);
                    default -> ((VoltageRegulationAdderOrBuilder<?>) a).withTerminal((Terminal) value);
                }
            }
            case BOUNDARY -> ((AreaBoundaryAdder) a).setBoundary((Boundary) value);
            case REL_TERMINAL_FROM -> ((VoltageAngleLimitAdder) a).from((Terminal) value);
            case REL_REGULATION_TERMINAL -> ((PhaseTapChangerAdder) a).setRegulationTerminal((Terminal) value);
            case REL_TERMINAL_TO -> ((VoltageAngleLimitAdder) a).to((Terminal) value);
            case ADD_VOLTAGE_LEVEL -> ((AreaAdder) a).addVoltageLevel((VoltageLevel) value);
            case EXT_MC_LINE1 -> ((MutualCouplingAdder) a).withLine1((Line) value);
            case EXT_MC_LINE2 -> ((MutualCouplingAdder) a).withLine2((Line) value);
            case COPY_OF -> {
                Object copy = copyAdder(pending.parent(), value);
                if (copy.getClass() != a.getClass()) {
                    throw new BridgeErrors.PropertyNotFoundException("Not a copy for this adder: " + value);
                }
                pending.adder = copy;
            }
            case FILL_FROM -> {
                if (a instanceof LineAdder adder) {
                    LineAdder.fillLineAdder(adder, (Line) value);
                } else {
                    TwoWindingsTransformerAdder.fillTwoWindingsTransformerAdder((TwoWindingsTransformerAdder) a,
                        (TwoWindingsTransformer) value);
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
    /** powsybl's SwitchPredicates, in the order of SwitchPredicate in include/iidm/Enums.h. */
    private static final List<Predicate<Switch>> SWITCH_PREDICATES = List.of(
        SwitchPredicates.IS_NONFICTIONAL_CLOSED_BREAKER, SwitchPredicates.IS_NONFICTIONAL_BREAKER,
        SwitchPredicates.IS_CLOSED_BREAKER, SwitchPredicates.IS_BREAKER_OR_DISCONNECTOR,
        SwitchPredicates.IS_OPEN_DISCONNECTOR, SwitchPredicates.IS_BREAKER, SwitchPredicates.IS_NONFICTIONAL,
        SwitchPredicates.IS_OPEN, SwitchPredicates.IS_NON_NULL);

    static Predicate<Switch> switchPredicate(int ordinal) {
        if (ordinal < 0 || ordinal >= SWITCH_PREDICATES.size()) {
            throw new IllegalArgumentException("Invalid SwitchPredicate value: " + ordinal);
        }
        return SWITCH_PREDICATES.get(ordinal);
    }

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
}
