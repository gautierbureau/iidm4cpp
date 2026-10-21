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
 * set with REL_TERMINAL and VR_TARGET_VALUE, and ACT_ADD applies it. So do moving an equipment's
 * end ({@code invoke(terminal, MOVE_CONNECTABLE)}), the fictitious injection at a node and the
 * removal of internal connections (on a voltage level). A query that takes several values goes the
 * same way, ACT_ADD returning its result: the checks of the temporary limits of a side.</p>
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

    /**
     * Moves the end of an equipment: to a bus, Terminal.BusBreakerView.moveConnectable(busId,
     * connected); to a node when the node is set, Terminal.NodeBreakerView.moveConnectable(node,
     * voltageLevelId).
     */
    static final class ConnectableMove {
        private final Terminal terminal;
        private String busId;
        private Boolean connected;
        private Integer node;
        private String voltageLevelId;

        ConnectableMove(Terminal terminal) {
            this.terminal = terminal;
        }

        void apply() {
            if (node != null) {
                terminal.getNodeBreakerView().moveConnectable(node, Objects.requireNonNull(voltageLevelId, "voltage level id"));
            } else {
                terminal.getBusBreakerView().moveConnectable(Objects.requireNonNull(busId, "bus id"),
                    Objects.requireNonNull(connected, "connected"));
            }
        }
    }

    /** NodeBreakerView.setFictitiousP0(node, p0), or setFictitiousQ0(node, q0): the code says which. */
    static final class NodeFictitiousInjection {
        private final VoltageLevel.NodeBreakerView view;
        private final int code;
        private Integer node;
        private double value = Double.NaN;

        NodeFictitiousInjection(VoltageLevel.NodeBreakerView view, int code) {
            this.view = view;
            this.code = code;
        }

        void setValue(int property, double value) {
            if (property != code) {
                throw unknownField("double", property);
            }
            this.value = value;
        }

        void apply() {
            int at = Objects.requireNonNull(node, "node");
            if (code == FICTITIOUS_P0) {
                view.setFictitiousP0(at, value);
            } else {
                view.setFictitiousQ0(at, value);
            }
        }
    }

    /** The arguments of NodeBreakerView.removeInternalConnections(node1, node2). */
    static final class InternalConnectionsRemoval {
        private final VoltageLevel.NodeBreakerView view;
        private Integer node1;
        private Integer node2;

        InternalConnectionsRemoval(VoltageLevel.NodeBreakerView view) {
            this.view = view;
        }

        void apply() {
            view.removeInternalConnections(Objects.requireNonNull(node1, "node1"), Objects.requireNonNull(node2, "node2"));
        }
    }

    /**
     * checkTemporaryLimits, or checkAllTemporaryLimits, of a side of a branch or a three windings
     * transformer: fields TERMINAL_SIDE, LIMIT_REDUCTION (none: the check without one) and
     * LL_LIMIT_TYPE. Its result is the overload (null without one), or the overloads.
     */
    static final class TemporaryLimitsCheck {
        private final Object checked;
        private final boolean all;
        private Integer side;
        private Double limitReduction;
        private Integer type;

        TemporaryLimitsCheck(Object checked, boolean all) {
            this.checked = checked;
            this.all = all;
        }

        Object run() {
            int at = Objects.requireNonNull(side, "side");
            LimitType limitType = enumValue(LimitType.class, Objects.requireNonNull(type, "limit type"));
            if (checked instanceof ThreeWindingsTransformer twt) {
                ThreeSides threeSides = enumValue(ThreeSides.class, at);
                if (all) {
                    return limitReduction == null ? twt.checkAllTemporaryLimits(threeSides, limitType)
                                                  : twt.checkAllTemporaryLimits(threeSides, limitReduction, limitType);
                }
                return limitReduction == null ? twt.checkTemporaryLimits(threeSides, limitType)
                                              : twt.checkTemporaryLimits(threeSides, limitReduction, limitType);
            }
            Branch<?> branch = (Branch<?>) checked;
            TwoSides twoSides = enumValue(TwoSides.class, at);
            if (all) {
                return limitReduction == null ? branch.checkAllTemporaryLimits(twoSides, limitType)
                                              : branch.checkAllTemporaryLimits(twoSides, limitReduction, limitType);
            }
            return limitReduction == null ? branch.checkTemporaryLimits(twoSides, limitType)
                                          : branch.checkTemporaryLimits(twoSides, limitReduction, limitType);
        }
    }

    /** One boundary of a new area: AreaAdder.addAreaBoundary(terminal or boundary, ac). */
    static final class NewAreaBoundary {
        private final AreaAdder adder;
        private Terminal terminal;
        private Boundary boundary;
        private Boolean ac;

        NewAreaBoundary(AreaAdder adder) {
            this.adder = adder;
        }

        void apply() {
            boolean isAc = Objects.requireNonNull(ac, "ac");
            if (terminal != null) {
                adder.addAreaBoundary(terminal, isAc);
            } else {
                adder.addAreaBoundary(Objects.requireNonNull(boundary, "terminal or boundary"), isAc);
            }
        }
    }

    /** VariantManager.cloneVariant(source, targets, mayOverwrite): each target id set adds one. */
    static final class VariantClone {
        private final VariantManager manager;
        private String sourceId;
        private final java.util.List<String> targetIds = new java.util.ArrayList<>();
        private boolean mayOverwrite;

        VariantClone(VariantManager manager) {
            this.manager = manager;
        }

        void apply() {
            manager.cloneVariant(Objects.requireNonNull(sourceId, "source variant id"), targetIds, mayOverwrite);
        }
    }

    /** Network.createSubnetwork(id, name, sourceFormat). */
    static final class NewSubnetwork {
        private final Network network;
        private String id;
        private String name;
        private String sourceFormat;

        NewSubnetwork(Network network) {
            this.network = network;
        }

        Network create() {
            return network.createSubnetwork(id, name, sourceFormat);
        }
    }

    /** Network.isBoundaryElement(identifiable): the identifiable when it is one, else null. */
    static final class BoundaryElementCheck {
        private final Network network;
        private Identifiable<?> identifiable;

        BoundaryElementCheck(Network network) {
            this.network = network;
        }

        Identifiable<?> run() {
            return network.isBoundaryElement(Objects.requireNonNull(identifiable, "identifiable")) ? identifiable : null;
        }
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

    /** A new SubstationPosition or LinePosition: its coordinates, added one by one (COORDINATE). */
    static final class NewPosition {
        private final Object adder;
        private final java.util.List<Coordinate> coordinates = new java.util.ArrayList<>();

        NewPosition(Object adder) {
            this.adder = adder;
        }

        Object add() {
            if (adder instanceof LinePositionAdder<?> line) {
                return line.withCoordinates(coordinates).add();
            }
            SubstationPositionAdder substation = (SubstationPositionAdder) adder;
            if (!coordinates.isEmpty()) {
                substation.withCoordinate(coordinates.get(coordinates.size() - 1));
            }
            return substation.add();
        }
    }

    /** A coordinate of a new position. */
    static final class NewCoordinate {
        private final NewPosition position;
        private double latitude = Double.NaN;
        private double longitude = Double.NaN;

        NewCoordinate(NewPosition position) {
            this.position = position;
        }

        void apply() {
            position.coordinates.add(new Coordinate(latitude, longitude));
        }
    }

    /** A feeder of a new ConnectablePosition: newFeeder() (slot 0) or newFeeder1() to newFeeder3(). */
    static final class NewFeeder {
        private final ConnectablePositionAdder<?> adder;
        private int slot;
        private String name;
        private Integer order;
        private ConnectablePosition.Direction direction;

        NewFeeder(ConnectablePositionAdder<?> adder) {
            this.adder = adder;
        }

        void apply() {
            ConnectablePositionAdder.FeederAdder<?> feeder = switch (slot) {
                case 0 -> adder.newFeeder();
                case 1 -> adder.newFeeder1();
                case 2 -> adder.newFeeder2();
                case 3 -> adder.newFeeder3();
                default -> throw new IllegalArgumentException("Invalid feeder slot: " + slot);
            };
            if (name != null) feeder.withName(name);
            if (order != null) feeder.withOrder(order);
            if (direction != null) feeder.withDirection(direction);
            feeder.add();
        }
    }

    /** An area of a new ObservabilityArea: by bus-breaker view buses, by a bus view bus, or by nodes. */
    static final class NewObservabilityArea {
        private final ObservabilityAreaAdder adder;
        private int number;
        private ObservabilityArea.ObservabilityStatus status;
        private final java.util.Set<String> busBreakerBusIds = new java.util.LinkedHashSet<>();
        private String busId;
        private final java.util.Set<Integer> nodes = new java.util.LinkedHashSet<>();

        NewObservabilityArea(ObservabilityAreaAdder adder) {
            this.adder = adder;
        }

        void apply() {
            if (busId != null) {
                adder.withObservabilityAreaByBusViewBus(busId, number, status);
            } else if (!nodes.isEmpty()) {
                adder.withObservabilityAreaByNodes(nodes, number, status);
            } else {
                adder.withObservabilityAreaByBusBreakerViewBuses(busBreakerBusIds, number, status);
            }
        }
    }

    /** ReferencePriority.set of an injection, or of a side of a branch or of a 3WT. */
    static final class PrioritySet {
        private final Object connectable;
        private int side;
        private int priority;

        PrioritySet(Object connectable) {
            this.connectable = connectable;
        }

        void apply() {
            switch (connectable) {
                case Injection<?> injection -> ReferencePriority.set(injection, priority);
                case Branch<?> branch -> ReferencePriority.set(branch, enumValue(TwoSides.class, side), priority);
                default -> ReferencePriority.set((ThreeWindingsTransformer) connectable, enumValue(ThreeSides.class, side), priority);
            }
        }
    }

    /** The terminals of a new ReferenceTerminals, or the new ones of an existing one (setReferenceTerminals). */
    static final class NewReferenceTerminals {
        private final Object target;
        private final java.util.Set<Terminal> terminals = new java.util.LinkedHashSet<>();

        NewReferenceTerminals(Object target) {
            this.target = target;
        }

        Object apply() {
            if (target instanceof ReferenceTerminalsAdder adder) {
                return adder.withTerminals(terminals).add();
            }
            ((ReferenceTerminals) target).setReferenceTerminals(terminals);
            return null;
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

    /** A plane of a new reactive capability shape: ReactiveCapabilityShapeAdder.addPlane, its values set one by one. */
    static final class NewPlane {
        private final ReactiveCapabilityShapeAdder adder;
        private double alpha = Double.NaN;
        private double beta = Double.NaN;
        private double delta = Double.NaN;
        private boolean greaterOrEqual;
        private double gamma = Double.NaN;

        NewPlane(ReactiveCapabilityShapeAdder adder) {
            this.adder = adder;
        }

        void apply() {
            adder.addPlane(alpha, beta, delta, greaterOrEqual, gamma);
        }
    }

    /** The pilot point of a new control zone: its busbar sections or buses, added one by one. */
    static final class NewPilotPoint {
        private final PilotPointAdder adder;
        private final java.util.List<String> ids = new java.util.ArrayList<>();

        NewPilotPoint(PilotPointAdder adder) {
            this.adder = adder;
        }

        void apply() {
            adder.withBusbarSectionsOrBusesIds(ids).add();
        }
    }

    /** The segment of a line of a mutual coupling, or of a new one: its start and end, set at once. */
    static final class SegmentChange {
        private final MutualCouplingAdder adder;
        private final MutualCoupling coupling;
        private final boolean line1;
        private double start = Double.NaN;
        private double end = Double.NaN;

        SegmentChange(Object target, int code) {
            this.adder = target instanceof MutualCouplingAdder a ? a : null;
            this.coupling = adder == null ? (MutualCoupling) target : null;
            this.line1 = code == EXT_MC_LINE1_SEGMENT;
        }

        void apply() {
            LineSegment segment = new LineSegment(start, end);
            if (adder != null) {
                if (line1) adder.withLine1Segment(segment);
                else adder.withLine2Segment(segment);
            } else if (line1) {
                coupling.setLine1Segment(segment);
            } else {
                coupling.setLine2Segment(segment);
            }
        }
    }

    /**
     * LineCouplings.findMutualCoupling(line1, line2): the coupling, or null; removeMutualCoupling(coupling)
     * or removeMutualCoupling(line1, line2): the extension when one was removed, else null.
     */
    static final class MutualCouplingQuery {
        private final LineCouplings couplings;
        private final boolean remove;
        private MutualCoupling coupling;
        private Line line1;
        private Line line2;

        MutualCouplingQuery(LineCouplings couplings, boolean remove) {
            this.couplings = couplings;
            this.remove = remove;
        }

        Object run() {
            if (!remove) {
                return couplings.findMutualCoupling(line1, line2).orElse(null);
            }
            boolean removed = coupling != null ? couplings.removeMutualCoupling(coupling) : couplings.removeMutualCoupling(line1, line2);
            return removed ? couplings : null;
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
            case AREA_BOUNDARY -> owner instanceof AreaAdder areaAdder ? new NewAreaBoundary(areaAdder) : ((Area) owner).newAreaBoundary();
            case VOLTAGE_ANGLE_LIMIT -> ((Network) owner).newVoltageAngleLimit();
            case OVERLOAD_MANAGEMENT_SYSTEM -> ((Substation) owner).newOverloadManagementSystem();
            case VARIANT_CLONE -> new VariantClone((VariantManager) owner);
            case NETWORK -> new NewSubnetwork((Network) owner);
            case BOUNDARY_ELEMENT -> new BoundaryElementCheck((Network) owner);
            case THREE_WINDINGS_TRANSFORMER_FORTESCUE ->
                new FortescueLegs((ThreeWindingsTransformerFortescueAdder) Extensions.newAdder(owner, type));
            case SUBSTATION_POSITION, LINE_POSITION -> new NewPosition(Extensions.newAdder(owner, type));
            case COORDINATE -> new NewCoordinate((NewPosition) owner);
            case FEEDER -> new NewFeeder((ConnectablePositionAdder<?>) owner);
            case AREA_CHARACTERISTICS -> new NewObservabilityArea((ObservabilityAreaAdder) owner);
            case REFERENCE_PRIORITY -> ((ReferencePriorities<?>) owner).newReferencePriority();
            case EXT_RP_PRIORITY -> new PrioritySet(owner);
            case REFERENCE_TERMINALS -> new NewReferenceTerminals(Extensions.newAdder(owner, type));
            case TERMINAL -> new NewReferenceTerminals((ReferenceTerminals) owner);
            case CONTROL_ZONE -> ((SecondaryVoltageControlAdder) owner).newControlZone();
            case PILOT_POINT -> new NewPilotPoint(((ControlZoneAdder) owner).newPilotPoint());
            case CONTROL_UNIT -> ((ControlZoneAdder) owner).newControlUnit();
            case MUTUAL_COUPLING -> ((LineCouplings) owner).newMutualCoupling();
            case EXT_MC_LINE1_SEGMENT, EXT_MC_LINE2_SEGMENT -> new SegmentChange(owner, type);
            case FIND_MUTUAL_COUPLING, REMOVE_MUTUAL_COUPLING -> new MutualCouplingQuery((LineCouplings) owner, type == REMOVE_MUTUAL_COUPLING);
            case SWITCH_TRIPPING -> ((OverloadManagementSystemAdder) owner).newSwitchTripping();
            case BRANCH_TRIPPING -> ((OverloadManagementSystemAdder) owner).newBranchTripping();
            case THREE_WINDINGS_TRANSFORMER_TRIPPING -> ((OverloadManagementSystemAdder) owner).newThreeWindingsTransformerTripping();
            case VOLTAGE_REGULATION -> owner instanceof VoltageRegulationHolderAdder<?> holderAdder
                ? holderAdder.newVoltageRegulation()
                : ((VoltageRegulationHolder<?>) owner).newVoltageRegulation();
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
            case REACTIVE_CAPABILITY_SHAPE_PLANE -> new NewPlane((ReactiveCapabilityShapeAdder) owner);
            case ZIP_LOAD_MODEL -> ((LoadAdder) owner).newZipModel();
            case EXPONENTIAL_LOAD_MODEL -> ((LoadAdder) owner).newExponentialModel();
            case MOVE_CONNECTABLE -> new ConnectableMove((Terminal) owner);
            case FICTITIOUS_P0, FICTITIOUS_Q0 -> new NodeFictitiousInjection(((VoltageLevel) owner).getNodeBreakerView(), type);
            case VL_NBV_REMOVE_INTERNAL_CONNECTIONS -> new InternalConnectionsRemoval(((VoltageLevel) owner).getNodeBreakerView());
            case CHECK_TEMPORARY_LIMITS -> new TemporaryLimitsCheck(PropertyDispatcher.overloadChecked(owner), false);
            case CHECK_ALL_TEMPORARY_LIMITS -> new TemporaryLimitsCheck(PropertyDispatcher.overloadChecked(owner), true);
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
            case ConnectableMove move -> {
                move.apply();
                yield null;
            }
            case NodeFictitiousInjection change -> {
                change.apply();
                yield null;
            }
            case InternalConnectionsRemoval removal -> {
                removal.apply();
                yield null;
            }
            case TemporaryLimitsCheck check -> check.run();
            case NewSubnetwork subnetwork -> subnetwork.create();
            case FortescueLegs legs -> legs.adder.add();
            case NewPosition position -> position.add();
            case NewCoordinate coordinate -> {
                coordinate.apply();
                yield null;
            }
            case NewFeeder feeder -> {
                feeder.apply();
                yield null;
            }
            case NewObservabilityArea area -> {
                area.apply();
                yield null;
            }
            case ReferencePriorityAdder adder -> adder.add();
            case PrioritySet set -> {
                set.apply();
                yield null;
            }
            case NewReferenceTerminals terminals -> terminals.apply();
            case ControlZoneAdder adder -> {
                adder.add();
                yield null;
            }
            case NewPilotPoint pilotPoint -> {
                pilotPoint.apply();
                yield null;
            }
            case ControlUnitAdder adder -> {
                adder.add();
                yield null;
            }
            case MutualCouplingAdder adder -> adder.add();
            case SegmentChange change -> {
                change.apply();
                yield null;
            }
            case MutualCouplingQuery query -> query.run();
            case BoundaryElementCheck check -> check.run();
            case VariantClone clone -> {
                clone.apply();
                yield null;
            }
            case AreaBoundaryAdder adder -> adder.add(); // the area
            case OverloadManagementSystemAdder.TrippingAdder<?> adder -> {
                adder.add();
                yield null;
            }
            case NewAreaBoundary boundary -> {
                boundary.apply();
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
            case NewPlane plane -> {
                plane.apply();
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
            case LINE_R -> ((LineAdder) a).setR(value);
            case LINE_X -> ((LineAdder) a).setX(value);
            case LINE_G1 -> ((LineAdder) a).setG1(value);
            case LINE_B1 -> ((LineAdder) a).setB1(value);
            case LINE_G2 -> ((LineAdder) a).setG2(value);
            case LINE_B2 -> ((LineAdder) a).setB2(value);
            case TWO_WT_R -> ((TwoWindingsTransformerAdder) a).setR(value);
            case TWO_WT_X -> ((TwoWindingsTransformerAdder) a).setX(value);
            case TWO_WT_G -> ((TwoWindingsTransformerAdder) a).setG(value);
            case TWO_WT_B -> ((TwoWindingsTransformerAdder) a).setB(value);
            case TWO_WT_RATED_U1 -> ((TwoWindingsTransformerAdder) a).setRatedU1(value);
            case TWO_WT_RATED_U2 -> ((TwoWindingsTransformerAdder) a).setRatedU2(value);
            case TWO_WT_RATED_S -> ((TwoWindingsTransformerAdder) a).setRatedS(value);
            case THREE_WT_RATED_U0 -> ((ThreeWindingsTransformerAdder) a).setRatedU0(value);
            case LEG_R -> ((NewLeg) a).leg().setR(value);
            case LEG_X -> ((NewLeg) a).leg().setX(value);
            case LEG_G -> ((NewLeg) a).leg().setG(value);
            case LEG_B -> ((NewLeg) a).leg().setB(value);
            case LEG_RATED_U -> ((NewLeg) a).leg().setRatedU(value);
            case LEG_RATED_S -> ((NewLeg) a).leg().setRatedS(value);
            case HVDC_R -> ((HvdcLineAdder) a).setR(value);
            case HVDC_NOMINAL_V -> ((HvdcLineAdder) a).setNominalV(value);
            case HVDC_ACTIVE_POWER_SETPOINT -> ((HvdcLineAdder) a).setActivePowerSetpoint(value);
            case HVDC_MAX_P -> ((HvdcLineAdder) a).setMaxP(value);
            case BAT_TARGET_P -> ((BatteryAdder) a).setTargetP(value);
            case BAT_MIN_P -> ((BatteryAdder) a).setMinP(value);
            case BAT_MAX_P -> ((BatteryAdder) a).setMaxP(value);
            case SVC_B_MIN -> ((StaticVarCompensatorAdder) a).setBmin(value);
            case SVC_B_MAX -> ((StaticVarCompensatorAdder) a).setBmax(value);
            case CONVERTER_STATION_LOSS_FACTOR -> ((HvdcConverterStationAdder<?, ?>) a).setLossFactor((float) value);
            case LCC_POWER_FACTOR -> ((LccConverterStationAdder) a).setPowerFactor((float) value);
            case SHUNT_B_PER_SECTION -> ((ShuntCompensatorLinearModelAdder) a).setBPerSection(value);
            case SHUNT_G_PER_SECTION -> ((ShuntCompensatorLinearModelAdder) a).setGPerSection(value);
            case SHUNT_SECTION_B -> ((ShuntCompensatorNonLinearModelAdder.SectionAdder) a).setB(value);
            case SHUNT_SECTION_G -> ((ShuntCompensatorNonLinearModelAdder.SectionAdder) a).setG(value);
            case BL_P0 -> ((BoundaryLineAdder) a).setP0(value);
            case BL_Q0 -> ((BoundaryLineAdder) a).setQ0(value);
            case BL_R -> ((BoundaryLineAdder) a).setR(value);
            case BL_X -> ((BoundaryLineAdder) a).setX(value);
            case BL_G -> ((BoundaryLineAdder) a).setG(value);
            case BL_B -> ((BoundaryLineAdder) a).setB(value);
            case GENERATION_TARGET_P -> ((BoundaryLineAdder.GenerationAdder) a).setTargetP(value);
            case GENERATION_MIN_P -> ((BoundaryLineAdder.GenerationAdder) a).setMinP(value);
            case GENERATION_MAX_P -> ((BoundaryLineAdder.GenerationAdder) a).setMaxP(value);
            case GENERATION_TARGET_Q -> ((BoundaryLineAdder.GenerationAdder) a).setTargetQ(value);
            case GENERATION_TARGET_V -> ((BoundaryLineAdder.GenerationAdder) a).setTargetV(value);
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
            case PTC_REGULATION_VALUE -> ((PhaseTapChangerAdder) a).setRegulationValue(value);
            case PTC_TARGET_DEADBAND -> ((PhaseTapChangerAdder) a).setTargetDeadband(value);
            case RL_MIN_Q -> ((MinMaxReactiveLimitsAdder) a).setMinQ(value);
            case RL_MAX_Q -> ((MinMaxReactiveLimitsAdder) a).setMaxQ(value);
            case POINT_P -> ((ReactiveCapabilityCurveAdder.PointAdder) a).setP(value);
            case POINT_MIN_Q -> ((ReactiveCapabilityCurveAdder.PointAdder) a).setMinQ(value);
            case POINT_MAX_Q -> ((ReactiveCapabilityCurveAdder.PointAdder) a).setMaxQ(value);
            case PLANE_ALPHA -> ((NewPlane) a).alpha = value;
            case PLANE_BETA -> ((NewPlane) a).beta = value;
            case PLANE_DELTA -> ((NewPlane) a).delta = value;
            case PLANE_GAMMA -> ((NewPlane) a).gamma = value;
            case ZIP_C0P -> ((ZipLoadModelAdder) a).setC0p(value);
            case ZIP_C1P -> ((ZipLoadModelAdder) a).setC1p(value);
            case ZIP_C2P -> ((ZipLoadModelAdder) a).setC2p(value);
            case ZIP_C0Q -> ((ZipLoadModelAdder) a).setC0q(value);
            case ZIP_C1Q -> ((ZipLoadModelAdder) a).setC1q(value);
            case ZIP_C2Q -> ((ZipLoadModelAdder) a).setC2q(value);
            case EXP_NP -> ((ExponentialLoadModelAdder) a).setNp(value);
            case EXP_NQ -> ((ExponentialLoadModelAdder) a).setNq(value);
            case FICTITIOUS_P0, FICTITIOUS_Q0 -> ((NodeFictitiousInjection) a).setValue(property, value);
            case LIMIT_REDUCTION -> ((TemporaryLimitsCheck) a).limitReduction = value;
            case AREA_INTERCHANGE_TARGET -> ((AreaAdder) a).setInterchangeTarget(value);
            case VAL_LOW_LIMIT -> ((VoltageAngleLimitAdder) a).setLowLimit(value);
            case VAL_HIGH_LIMIT -> ((VoltageAngleLimitAdder) a).setHighLimit(value);
            case TRIPPING_CURRENT_LIMIT -> ((OverloadManagementSystemAdder.TrippingAdder<?>) a).setCurrentLimit(value);
            case EXT_SC_DIRECT_SUBTRANS_X -> ((ShortCircuitExtensionAdder<?, ?, ?>) a).withDirectSubtransX(value);
            case EXT_SC_DIRECT_TRANS_X -> ((ShortCircuitExtensionAdder<?, ?, ?>) a).withDirectTransX(value);
            case EXT_SC_STEP_UP_TRANSFORMER_X -> ((ShortCircuitExtensionAdder<?, ?, ?>) a).withStepUpTransformerX(value);
            case EXT_ISC_IP_MIN -> ((IdentifiableShortCircuitAdder<?>) a).withIpMin(value);
            case EXT_ISC_IP_MAX -> ((IdentifiableShortCircuitAdder<?>) a).withIpMax(value);
            case EXT_GS_PLANNED_ACTIVE_POWER_SETPOINT -> ((GeneratorStartupAdder) a).withPlannedActivePowerSetpoint(value);
            case EXT_GS_STARTUP_COST -> ((GeneratorStartupAdder) a).withStartupCost(value);
            case EXT_GS_MARGINAL_COST -> ((GeneratorStartupAdder) a).withMarginalCost(value);
            case EXT_GS_PLANNED_OUTAGE_RATE -> ((GeneratorStartupAdder) a).withPlannedOutageRate(value);
            case EXT_GS_FORCED_OUTAGE_RATE -> ((GeneratorStartupAdder) a).withForcedOutageRate(value);
            case EXT_LD_FIXED_ACTIVE_POWER -> ((LoadDetailAdder) a).withFixedActivePower(value);
            case EXT_LD_FIXED_REACTIVE_POWER -> ((LoadDetailAdder) a).withFixedReactivePower(value);
            case EXT_LD_VARIABLE_ACTIVE_POWER -> ((LoadDetailAdder) a).withVariableActivePower(value);
            case EXT_LD_VARIABLE_REACTIVE_POWER -> ((LoadDetailAdder) a).withVariableReactivePower(value);
            case EXT_SA_B0 -> ((StandbyAutomatonAdder) a).withB0(value);
            case EXT_SA_LOW_VOLTAGE_SETPOINT -> ((StandbyAutomatonAdder) a).withLowVoltageSetpoint(value);
            case EXT_SA_HIGH_VOLTAGE_SETPOINT -> ((StandbyAutomatonAdder) a).withHighVoltageSetpoint(value);
            case EXT_SA_LOW_VOLTAGE_THRESHOLD -> ((StandbyAutomatonAdder) a).withLowVoltageThreshold(value);
            case EXT_SA_HIGH_VOLTAGE_THRESHOLD -> ((StandbyAutomatonAdder) a).withHighVoltageThreshold(value);
            case EXT_GF_RZ -> ((GeneratorFortescueAdder) a).withRz(value);
            case EXT_GF_XZ -> ((GeneratorFortescueAdder) a).withXz(value);
            case EXT_GF_RN -> ((GeneratorFortescueAdder) a).withRn(value);
            case EXT_GF_XN -> ((GeneratorFortescueAdder) a).withXn(value);
            case EXT_GF_GROUNDING_R -> ((GeneratorFortescueAdder) a).withGroundingR(value);
            case EXT_GF_GROUNDING_X -> ((GeneratorFortescueAdder) a).withGroundingX(value);
            case EXT_LINE_F_RZ -> ((LineFortescueAdder) a).withRz(value);
            case EXT_LINE_F_XZ -> ((LineFortescueAdder) a).withXz(value);
            case EXT_LINE_F_G1Z -> ((LineFortescueAdder) a).withG1z(value);
            case EXT_LINE_F_B1Z -> ((LineFortescueAdder) a).withB1z(value);
            case EXT_LINE_F_G2Z -> ((LineFortescueAdder) a).withG2z(value);
            case EXT_LINE_F_B2Z -> ((LineFortescueAdder) a).withB2z(value);
            case EXT_LA_DELTA_PA -> ((LoadAsymmetricalAdder) a).withDeltaPa(value);
            case EXT_LA_DELTA_QA -> ((LoadAsymmetricalAdder) a).withDeltaQa(value);
            case EXT_LA_DELTA_PB -> ((LoadAsymmetricalAdder) a).withDeltaPb(value);
            case EXT_LA_DELTA_QB -> ((LoadAsymmetricalAdder) a).withDeltaQb(value);
            case EXT_LA_DELTA_PC -> ((LoadAsymmetricalAdder) a).withDeltaPc(value);
            case EXT_LA_DELTA_QC -> ((LoadAsymmetricalAdder) a).withDeltaQc(value);
            case EXT_TWO_WTF_RZ -> ((TwoWindingsTransformerFortescueAdder) a).withRz(value);
            case EXT_TWO_WTF_XZ -> ((TwoWindingsTransformerFortescueAdder) a).withXz(value);
            case EXT_TWO_WTF_XM -> ((TwoWindingsTransformerFortescueAdder) a).withXm(value);
            case EXT_TWO_WTF_GROUNDING_R1 -> ((TwoWindingsTransformerFortescueAdder) a).withGroundingR1(value);
            case EXT_TWO_WTF_GROUNDING_X1 -> ((TwoWindingsTransformerFortescueAdder) a).withGroundingX1(value);
            case EXT_TWO_WTF_GROUNDING_R2 -> ((TwoWindingsTransformerFortescueAdder) a).withGroundingR2(value);
            case EXT_TWO_WTF_GROUNDING_X2 -> ((TwoWindingsTransformerFortescueAdder) a).withGroundingX2(value);
            case EXT_LEG_F_RZ -> ((FortescueLegs) a).leg().withRz(value);
            case EXT_LEG_F_XZ -> ((FortescueLegs) a).leg().withXz(value);
            case EXT_LEG_F_GROUNDING_R -> ((FortescueLegs) a).leg().withGroundingR(value);
            case EXT_LEG_F_GROUNDING_X -> ((FortescueLegs) a).leg().withGroundingX(value);
            case COORD_LATITUDE -> ((NewCoordinate) a).latitude = value;
            case COORD_LONGITUDE -> ((NewCoordinate) a).longitude = value;
            case EXT_SVC_TARGET_V -> ((NewPilotPoint) a).adder.withTargetV(value);
            case EXT_MC_R -> ((MutualCouplingAdder) a).withR(value);
            case EXT_MC_X -> ((MutualCouplingAdder) a).withX(value);
            case EXT_MC_SEGMENT_START -> ((SegmentChange) a).start = value;
            case EXT_MC_SEGMENT_END -> ((SegmentChange) a).end = value;
            default -> throw unknownField("double", property);
        }
    }

    static void setInt(PendingAdd pending, int property, int value) {
        Object a = pending.adder();
        switch (property) {
            case TERMINAL_NODE -> {
                switch (a) {
                    case ConnectableMove move -> move.node = value;
                    case NodeFictitiousInjection change -> change.node = value;
                    case NewLeg leg -> leg.leg().setNode(value);
                    case BusbarSectionAdder adder -> adder.setNode(value);
                    default -> ((InjectionAdder<?, ?>) a).setNode(value);
                }
            }
            case IC_NODE1 -> {
                if (a instanceof VoltageLevel.NodeBreakerView.InternalConnectionAdder adder) {
                    adder.setNode1(value);
                } else {
                    ((InternalConnectionsRemoval) a).node1 = value;
                }
            }
            case TERMINAL_SIDE -> {
                if (a instanceof PrioritySet set) {
                    set.side = value;
                } else {
                    ((TemporaryLimitsCheck) a).side = value;
                }
            }
            case EXT_RP_PRIORITY -> {
                if (a instanceof PrioritySet set) {
                    set.priority = value;
                } else {
                    ((ReferencePriorityAdder) a).setPriority(value);
                }
            }
            case OBS_AREA_NUMBER -> ((NewObservabilityArea) a).number = value;
            case OBS_AREA_STATUS -> ((NewObservabilityArea) a).status = enumValue(ObservabilityArea.ObservabilityStatus.class, value);
            case OBS_AREA_NODE -> ((NewObservabilityArea) a).nodes.add(value);
            case OMS_MONITORED_SIDE -> ((OverloadManagementSystemAdder) a).setMonitoredElementSide(enumValue(ThreeSides.class, value));
            case TRIPPING_SIDE -> {
                if (a instanceof OverloadManagementSystemAdder.BranchTrippingAdder adder) {
                    adder.setSideToOperate(enumValue(TwoSides.class, value));
                } else {
                    ((OverloadManagementSystemAdder.ThreeWindingsTransformerTrippingAdder) a)
                        .setSideToOperate(enumValue(ThreeSides.class, value));
                }
            }
            case LL_LIMIT_TYPE -> ((TemporaryLimitsCheck) a).type = value;
            case TC_LOW_TAP_POSITION -> ((TapChangerAdder<?, ?, ?, ?, ?, ?>) a).setLowTapPosition(value);
            case TC_TAP_POSITION -> ((TapChangerAdder<?, ?, ?, ?, ?, ?>) a).setTapPosition(value);
            case TC_SOLVED_TAP_POSITION -> ((TapChangerAdder<?, ?, ?, ?, ?, ?>) a).setSolvedTapPosition(value);
            case PTC_REGULATION_MODE -> ((PhaseTapChangerAdder) a)
                .setRegulationMode(enumValue(PhaseTapChanger.RegulationMode.class, value));
            case IC_NODE2 -> {
                if (a instanceof VoltageLevel.NodeBreakerView.InternalConnectionAdder adder) {
                    adder.setNode2(value);
                } else {
                    ((InternalConnectionsRemoval) a).node2 = value;
                }
            }
            case SW_NODE1 -> ((VoltageLevel.NodeBreakerView.SwitchAdder) a).setNode1(value);
            case SW_NODE2 -> ((VoltageLevel.NodeBreakerView.SwitchAdder) a).setNode2(value);
            case SW_KIND -> ((VoltageLevel.NodeBreakerView.SwitchAdder) a).setKind(enumValue(SwitchKind.class, value));
            case VL_TOPOLOGY_KIND -> ((VoltageLevelAdder) a).setTopologyKind(enumValue(TopologyKind.class, value));
            case GEN_ENERGY_SOURCE -> ((GeneratorAdder) a).setEnergySource(enumValue(EnergySource.class, value));
            case LOAD_TYPE -> ((LoadAdder) a).setLoadType(enumValue(LoadType.class, value));
            case SHUNT_SECTION_COUNT -> ((ShuntCompensatorAdder) a).setSectionCount(value);
            case BRANCH_NODE1 -> ((BranchAdder<?, ?>) a).setNode1(value);
            case BRANCH_NODE2 -> ((BranchAdder<?, ?>) a).setNode2(value);
            case LEG_SIDE -> ((NewLeg) a).select(value);
            case HVDC_CONVERTERS_MODE -> ((HvdcLineAdder) a).setConvertersMode(enumValue(HvdcLine.ConvertersMode.class, value));
            case SHUNT_SOLVED_SECTION_COUNT -> ((ShuntCompensatorAdder) a).setSolvedSectionCount(value);
            case SHUNT_MAX_SECTION_COUNT -> ((ShuntCompensatorLinearModelAdder) a).setMaximumSectionCount(value);
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
            case EXT_GEC_CODE -> ((GeneratorEntsoeCategoryAdder) a).withCode(value);
            case EXT_OS_STATUS -> ((OperatingStatusAdder<?>) a).withStatus(enumValue(OperatingStatus.Status.class, value));
            case EXT_LA_CONNECTION_TYPE -> ((LoadAsymmetricalAdder) a).withConnectionType(enumValue(LoadConnectionType.class, value));
            case EXT_TWO_WTF_CONNECTION_TYPE1 -> ((TwoWindingsTransformerFortescueAdder) a)
                .withConnectionType1(enumValue(WindingConnectionType.class, value));
            case EXT_TWO_WTF_CONNECTION_TYPE2 -> ((TwoWindingsTransformerFortescueAdder) a)
                .withConnectionType2(enumValue(WindingConnectionType.class, value));
            case EXT_THREE_WTF_LEG -> ((FortescueLegs) a).select(value);
            case EXT_LEG_F_CONNECTION_TYPE -> ((FortescueLegs) a).leg().withConnectionType(enumValue(WindingConnectionType.class, value));
            case EXT_TWO_WT_PAC_CLOCK -> ((TwoWindingsTransformerPhaseAngleClockAdder) a).withPhaseAngleClock(value);
            case EXT_THREE_WT_PAC_CLOCK_LEG2 -> ((ThreeWindingsTransformerPhaseAngleClockAdder) a).withPhaseAngleClockLeg2(value);
            case EXT_THREE_WT_PAC_CLOCK_LEG3 -> ((ThreeWindingsTransformerPhaseAngleClockAdder) a).withPhaseAngleClockLeg3(value);
            case EXT_BSP_BUSBAR_INDEX -> ((BusbarSectionPositionAdder) a).withBusbarIndex(value);
            case EXT_BSP_SECTION_INDEX -> ((BusbarSectionPositionAdder) a).withSectionIndex(value);
            case EXT_FEEDER_SLOT -> ((NewFeeder) a).slot = value;
            case EXT_FEEDER_ORDER -> ((NewFeeder) a).order = value;
            case EXT_FEEDER_DIRECTION -> ((NewFeeder) a).direction = enumValue(ConnectablePosition.Direction.class, value);
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
            case TERMINAL_CONNECTED -> ((ConnectableMove) a).connected = value;
            case AUTOMATION_SYSTEM_ENABLED -> ((OverloadManagementSystemAdder) a).setEnabled(value);
            case VARIANT_MAY_OVERWRITE -> ((VariantClone) a).mayOverwrite = value;
            case TRIPPING_OPEN_ACTION -> ((OverloadManagementSystemAdder.TrippingAdder<?>) a).setOpenAction(value);
            case AREA_BOUNDARY_AC -> {
                if (a instanceof NewAreaBoundary boundary) {
                    boundary.ac = value;
                } else {
                    ((AreaBoundaryAdder) a).setAc(value);
                }
            }
            case GEN_CONDENSER -> ((GeneratorAdder) a).setCondenser(value);
            case GENERATION_VOLTAGE_REGULATION_ON -> ((BoundaryLineAdder.GenerationAdder) a).setVoltageRegulationOn(value);
            case SW_OPEN -> {
                if (a instanceof VoltageLevel.NodeBreakerView.SwitchAdder adder) {
                    adder.setOpen(value);
                } else {
                    ((VoltageLevel.BusBreakerView.SwitchAdder) a).setOpen(value);
                }
            }
            case SW_RETAINED -> ((VoltageLevel.NodeBreakerView.SwitchAdder) a).setRetained(value);
            case TC_LOAD_TAP_CHANGING_CAPABILITIES -> ((TapChangerAdder<?, ?, ?, ?, ?, ?>) a).setLoadTapChangingCapabilities(value);
            case REGULATING -> ((PhaseTapChangerAdder) a).setRegulating(value);
            case PLANE_GREATER_OR_EQUAL -> ((NewPlane) a).greaterOrEqual = value;
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
            case EXT_SA_STANDBY -> ((StandbyAutomatonAdder) a).withStandbyStatus(value);
            case EXT_MFRR_PARTICIPATE -> ((ManualFrequencyRestorationReserveAdder<?>) a).withParticipate(value);
            case EXT_GF_GROUNDED -> ((GeneratorFortescueAdder) a).withGrounded(value);
            case EXT_LINE_F_OPEN_PHASE_A -> ((LineFortescueAdder) a).withOpenPhaseA(value);
            case EXT_LINE_F_OPEN_PHASE_B -> ((LineFortescueAdder) a).withOpenPhaseB(value);
            case EXT_LINE_F_OPEN_PHASE_C -> ((LineFortescueAdder) a).withOpenPhaseC(value);
            case EXT_TWO_WTF_FREE_FLUXES -> ((TwoWindingsTransformerFortescueAdder) a).withFreeFluxes(value);
            case EXT_LEG_F_FREE_FLUXES -> ((FortescueLegs) a).leg().withFreeFluxes(value);
            case EXT_SVC_UNIT_PARTICIPATE -> ((ControlUnitAdder) a).withParticipate(value);
            default -> throw unknownField("bool", property);
        }
    }

    static void setString(PendingAdd pending, int property, String value) {
        Object a = pending.adder();
        switch (property) {
            case ID -> {
                if (a instanceof NewSubnetwork subnetwork) {
                    subnetwork.id = value;
                } else {
                    ((IdentifiableAdder<?, ?>) a).setId(value);
                }
            }
            case NAME -> {
                if (a instanceof NewSubnetwork subnetwork) {
                    subnetwork.name = value;
                } else {
                    ((IdentifiableAdder<?, ?>) a).setName(value);
                }
            }
            case NETWORK_SOURCE_FORMAT -> ((NewSubnetwork) a).sourceFormat = value;
            case COUNTRY -> ((SubstationAdder) a).setCountry(Country.valueOf(value));
            case SUBSTATION_TSO -> ((SubstationAdder) a).setTso(value);
            case TERMINAL_BUS_BREAKER_BUS_ID -> {
                if (a instanceof NewLeg leg) {
                    leg.leg().setBus(value);
                } else {
                    ((InjectionAdder<?, ?>) a).setBus(value);
                }
            }
            case TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID -> {
                switch (a) {
                    case ConnectableMove move -> move.busId = value;
                    case NewLeg leg -> leg.leg().setConnectableBus(value);
                    default -> ((InjectionAdder<?, ?>) a).setConnectableBus(value);
                }
            }
            case TERMINAL_VOLTAGE_LEVEL_ID -> {
                if (a instanceof NewLeg leg) {
                    leg.leg().setVoltageLevel(value);
                } else {
                    ((ConnectableMove) a).voltageLevelId = value;
                }
            }
            case BRANCH_VOLTAGE_LEVEL1 -> ((BranchAdder<?, ?>) a).setVoltageLevel1(value);
            case BRANCH_VOLTAGE_LEVEL2 -> ((BranchAdder<?, ?>) a).setVoltageLevel2(value);
            case BRANCH_BUS1 -> ((BranchAdder<?, ?>) a).setBus1(value);
            case BRANCH_BUS2 -> ((BranchAdder<?, ?>) a).setBus2(value);
            case BRANCH_CONNECTABLE_BUS1 -> ((BranchAdder<?, ?>) a).setConnectableBus1(value);
            case BRANCH_CONNECTABLE_BUS2 -> ((BranchAdder<?, ?>) a).setConnectableBus2(value);
            case HVDC_CONVERTER_STATION_ID1 -> ((HvdcLineAdder) a).setConverterStationId1(value);
            case HVDC_CONVERTER_STATION_ID2 -> ((HvdcLineAdder) a).setConverterStationId2(value);
            case BOUNDARY_LINE1_ID -> ((TieLineAdder) a).setBoundaryLine1(value);
            case BL_PAIRING_KEY -> ((BoundaryLineAdder) a).setPairingKey(value);
            case SW_BUS1 -> ((VoltageLevel.BusBreakerView.SwitchAdder) a).setBus1(value);
            case SW_BUS2 -> ((VoltageLevel.BusBreakerView.SwitchAdder) a).setBus2(value);
            case AREA_TYPE -> ((AreaAdder) a).setAreaType(value);
            case VAL_ID -> ((VoltageAngleLimitAdder) a).setId(value);
            case OMS_MONITORED_ELEMENT_ID -> ((OverloadManagementSystemAdder) a).setMonitoredElementId(value);
            case VARIANT_SOURCE_ID -> ((VariantClone) a).sourceId = value;
            case VARIANT_TARGET_ID -> ((VariantClone) a).targetIds.add(value);
            case TRIPPING_KEY -> ((OverloadManagementSystemAdder.TrippingAdder<?>) a).setKey(value);
            case TRIPPING_NAME -> ((OverloadManagementSystemAdder.TrippingAdder<?>) a).setName(value);
            case TRIPPING_ELEMENT_ID -> {
                switch (a) {
                    case OverloadManagementSystemAdder.SwitchTrippingAdder adder -> adder.setSwitchToOperateId(value);
                    case OverloadManagementSystemAdder.BranchTrippingAdder adder -> adder.setBranchToOperateId(value);
                    default -> ((OverloadManagementSystemAdder.ThreeWindingsTransformerTrippingAdder) a)
                        .setThreeWindingsTransformerToOperateId(value);
                }
            }
            case BOUNDARY_LINE2_ID -> ((TieLineAdder) a).setBoundaryLine2(value);
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
            case EXT_DMI_MODEL_NAME -> ((DynamicModelInfoAdder<?>) a).setModelName(value);
            case EXT_FEEDER_NAME -> ((NewFeeder) a).name = value;
            case OBS_AREA_BUS_BREAKER_BUS_ID -> ((NewObservabilityArea) a).busBreakerBusIds.add(value);
            case OBS_AREA_BUS_ID -> ((NewObservabilityArea) a).busId = value;
            case EXT_SVC_ZONE_NAME -> ((ControlZoneAdder) a).withName(value);
            case EXT_SVC_PILOT_POINT_IDS -> ((NewPilotPoint) a).ids.add(value);
            case EXT_SVC_UNIT_ID -> ((ControlUnitAdder) a).withId(value);
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
                    case AreaBoundaryAdder adder -> adder.setTerminal((Terminal) value);
                    case NewAreaBoundary boundary -> boundary.terminal = (Terminal) value;
                    case ReferencePriorityAdder adder -> adder.setTerminal((Terminal) value);
                    default -> ((VoltageRegulationAdderOrBuilder<?>) a).withTerminal((Terminal) value);
                }
            }
            case BOUNDARY -> {
                if (a instanceof NewAreaBoundary boundary) {
                    boundary.boundary = (Boundary) value;
                } else {
                    ((AreaBoundaryAdder) a).setBoundary((Boundary) value);
                }
            }
            case REL_TERMINAL_FROM -> ((VoltageAngleLimitAdder) a).from((Terminal) value);
            case REL_REGULATION_TERMINAL -> ((PhaseTapChangerAdder) a).setRegulationTerminal((Terminal) value);
            case REL_TERMINAL_TO -> ((VoltageAngleLimitAdder) a).to((Terminal) value);
            case ADD_VOLTAGE_LEVEL -> ((AreaAdder) a).addVoltageLevel((VoltageLevel) value);
            case IDENTIFIABLE -> ((BoundaryElementCheck) a).identifiable = (Identifiable<?>) value;
            case ADD_REFERENCE_TERMINAL -> ((NewReferenceTerminals) a).terminals.add((Terminal) value);
            case EXT_MC_LINE1 -> {
                if (a instanceof MutualCouplingQuery query) {
                    query.line1 = (Line) value;
                } else {
                    ((MutualCouplingAdder) a).withLine1((Line) value);
                }
            }
            case EXT_MC_LINE2 -> {
                if (a instanceof MutualCouplingQuery query) {
                    query.line2 = (Line) value;
                } else {
                    ((MutualCouplingAdder) a).withLine2((Line) value);
                }
            }
            case MUTUAL_COUPLING -> ((MutualCouplingQuery) a).coupling = (MutualCoupling) value;
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
