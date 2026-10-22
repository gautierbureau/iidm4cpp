package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.LoadingLimits.TemporaryLimit;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidm.network.regulation.RegulationMode;
import com.powsybl.iidm.network.regulation.VoltageRegulation;
import com.powsybl.iidm.network.regulation.VoltageRegulationHolder;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Collection;
import java.util.Comparator;
import java.util.Objects;
import java.util.OptionalInt;
import java.util.function.Predicate;

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

    /**
     * The hash of the property codes: FNV-1a of each "NAME=value\n", in the order of
     * PropertyCodes.def. The C++ library compares it with its own when a backend opens, as a jar
     * or native image built from other codes would read other properties without any error.
     */
    public static long codesHash() {
        long hash = 0xcbf29ce484222325L;
        for (String code : PropertyCodes.CODES) {
            for (byte b : (code + "\n").getBytes(StandardCharsets.US_ASCII)) {
                hash ^= b & 0xff;
                hash *= 0x100000001b3L;
            }
        }
        return hash;
    }

    // ── Double getters ────────────────────────────────────────────────────

    public static double getDouble(long handle, int property) {
        return getDouble(NetworkRegistry.lookup(handle), property);
    }

    public static double getDouble(Object obj, int property) {
        PropertyTable.DoubleProperty table = PropertyTable.doubleProperty(property);
        if (table != null && table.get() != null) return table.get().applyAsDouble(obj);
        throw new BridgeErrors.PropertyNotFoundException("Unknown double property: " + property);
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
        PropertyTable.DoubleProperty table = PropertyTable.doubleProperty(property);
        if (table != null && table.set() != null) {
            table.set().accept(obj, value);
            return;
        }
        throw new BridgeErrors.PropertyNotFoundException("Unknown double property for set: " + property);
    }

    // ── Int getters ───────────────────────────────────────────────────────

    public static int getInt(long handle, int property) {
        return getInt(NetworkRegistry.lookup(handle), property);
    }

    public static int getInt(Object obj, int property) {
        PropertyTable.IntProperty table = PropertyTable.intProperty(property);
        if (table != null && table.get() != null) return table.get().applyAsInt(obj);
        return switch (property) {
            case TERMINAL_SIDE -> {
                ThreeSides side = ((Terminal) obj).getSide();
                yield side == null ? -1 : side.ordinal();
            }
            case VR_MODE -> {
                RegulationMode mode = ((VoltageRegulation) obj).getMode();
                yield mode == null ? -1 : mode.ordinal();
            }
            case VL_NBV_MAXIMUM_NODE_INDEX -> ((VoltageLevel) obj).getNodeBreakerView().getMaximumNodeIndex();
            case SW_NODE1 -> ((Switch) obj).getVoltageLevel().getNodeBreakerView().getNode1(((Switch) obj).getId());
            case SW_NODE2 -> ((Switch) obj).getVoltageLevel().getNodeBreakerView().getNode2(((Switch) obj).getId());
            case TRIPPING_SIDE -> obj instanceof OverloadManagementSystem.BranchTripping tripping
                ? tripping.getSideToOperate().ordinal()
                : ((OverloadManagementSystem.ThreeWindingsTransformerTripping) obj).getSideToOperate().ordinal();
            case OVERLOAD_DURATION -> overloadChecked(obj) instanceof ThreeWindingsTransformer twt
                ? twt.getOverloadDuration()
                : ((Branch<?>) obj).getOverloadDuration();
            // Terminal node number (node-breaker topology)
            case TERMINAL_NODE -> ((Terminal) obj).getNodeBreakerView().getNode();
            // State estimation extensions
            case OBS_QUALITY_REDUNDANT -> ((ObservabilityQuality<?>) obj).isRedundant().map(r -> r ? 1 : 0).orElse(-1);
            case MEAS_SIDE -> {
                ThreeSides side = ((Measurement) obj).getSide();
                yield side == null ? -1 : side.ordinal();
            }
            case DMEAS_TAP_CHANGER -> {
                DiscreteMeasurement.TapChanger tapChanger = ((DiscreteMeasurement) obj).getTapChanger();
                yield tapChanger == null ? -1 : tapChanger.ordinal();
            }
            case OBS_AREA_NUMBER    -> ((ObservabilityAreaRef) obj).area().getAreaNumber();
            case OBS_AREA_STATUS    -> ((ObservabilityAreaRef) obj).area().getStatus().ordinal();
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
        PropertyTable.IntProperty table = PropertyTable.intProperty(property);
        if (table != null && table.set() != null) {
            table.set().accept(obj, value);
            return;
        }
        switch (property) {
            case NETWORK_MIN_VALIDATION_LEVEL ->
                ((Network) obj).setMinimumAcceptableValidationLevel(Creation.enumValue(ValidationLevel.class, value));
            case VR_MODE -> ((VoltageRegulation) obj).setMode(value < 0 ? null : Creation.enumValue(RegulationMode.class, value));
            case HVDC_CONVERTERS_MODE ->
                ((HvdcLine) obj).setConvertersMode(HvdcLine.ConvertersMode.values()[value]);
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
            case EXT_TWO_WTF_CONNECTION_TYPE1 -> ((TwoWindingsTransformerFortescue) obj)
                .setConnectionType1(Creation.enumValue(WindingConnectionType.class, value));
            case EXT_TWO_WTF_CONNECTION_TYPE2 -> ((TwoWindingsTransformerFortescue) obj)
                .setConnectionType2(Creation.enumValue(WindingConnectionType.class, value));
            case EXT_FEEDER_DIRECTION -> ((ConnectablePosition.Feeder) obj)
                .setDirection(Creation.enumValue(ConnectablePosition.Direction.class, value));
            default -> throw new BridgeErrors.PropertyNotFoundException("Unknown int property for set: " + property);
        }
    }

    // ── Bool getters ──────────────────────────────────────────────────────

    public static int getBool(long handle, int property) {
        return getBool(NetworkRegistry.lookup(handle), property) ? 1 : 0;
    }

    public static boolean getBool(Object obj, int property) {
        PropertyTable.BoolProperty table = PropertyTable.boolProperty(property);
        if (table != null && table.get() != null) return table.get().test(obj);
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
            case VOLTAGE_ANGLE_LIMIT        -> obj instanceof VoltageAngleLimit;
            case OVERLOAD_MANAGEMENT_SYSTEM -> obj instanceof OverloadManagementSystem;
            case COMPONENT                  -> obj instanceof Component;
            case VARIANT_MANAGER            -> obj instanceof VariantManager;
            case TRIPPING                   -> obj instanceof OverloadManagementSystem.Tripping;
            case SWITCH_TRIPPING            -> obj instanceof OverloadManagementSystem.SwitchTripping;
            case BRANCH_TRIPPING            -> obj instanceof OverloadManagementSystem.BranchTripping;
            case THREE_WINDINGS_TRANSFORMER_TRIPPING -> obj instanceof OverloadManagementSystem.ThreeWindingsTransformerTripping;
            case GROUND                     -> obj instanceof Ground;
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
            case MEASUREMENT                -> obj instanceof Measurement;
            case DISCRETE_MEASUREMENT       -> obj instanceof DiscreteMeasurement;
            case AREA_CHARACTERISTICS       -> obj instanceof ObservabilityAreaRef;
            case REGULATING                 -> obj instanceof TapChanger<?, ?, ?, ?> tapChanger
                                               ? tapChanger.isRegulating()
                                               : ((VoltageRegulationHolder<?>) obj).isRegulating();
            // State estimation extensions
            case REFERENCE_PRIORITY         -> obj instanceof ReferencePriority;
            case COORDINATE                 -> obj instanceof Coordinate;
            case FEEDER                     -> obj instanceof ConnectablePosition.Feeder;
            case LEG_FORTESCUE              -> obj instanceof LegFortescue;
            case CONTROL_ZONE               -> obj instanceof ControlZone;
            case PILOT_POINT                -> obj instanceof PilotPoint;
            case CONTROL_UNIT               -> obj instanceof ControlUnit;
            case MUTUAL_COUPLING            -> obj instanceof MutualCoupling;
            default -> {
                // An extension type code: whether the object is one
                if (Extensions.isType(property)) yield Extensions.isInstance(obj, property);
                throw new BridgeErrors.PropertyNotFoundException("Unknown bool property: " + property);
            }
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
        PropertyTable.BoolProperty table = PropertyTable.boolProperty(property);
        if (table != null && table.set() != null) {
            table.set().accept(obj, bval);
            return;
        }
        throw new BridgeErrors.PropertyNotFoundException("Unknown bool property for set: " + property);
    }

    // ── String getters ────────────────────────────────────────────────────

    public static String getString(long handle, int property) {
        return getString(NetworkRegistry.lookup(handle), property);
    }

    /** A value, or fails: a property that may have none is read with {@link #getOptString}. */
    public static String getString(Object obj, int property) {
        PropertyTable.StringProperty table = PropertyTable.stringProperty(property);
        String value = table != null && table.get() != null ? table.get().apply(obj) : switch (property) {
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
            case VL_TOPOLOGY_TEXT -> {
                java.io.ByteArrayOutputStream text = new java.io.ByteArrayOutputStream();
                ((VoltageLevel) obj).printTopology(new java.io.PrintStream(text, true, java.nio.charset.StandardCharsets.UTF_8), null);
                yield text.toString(java.nio.charset.StandardCharsets.UTF_8);
            }
            case TRIPPING_ELEMENT_ID -> switch (obj) {
                case OverloadManagementSystem.SwitchTripping tripping -> tripping.getSwitchToOperateId();
                case OverloadManagementSystem.BranchTripping tripping -> tripping.getBranchToOperateId();
                default -> ((OverloadManagementSystem.ThreeWindingsTransformerTripping) obj).getThreeWindingsTransformerToOperateId();
            };
            case BL_PAIRING_KEY -> pairingKey(obj);
            case SELECTED_OLG_ID -> LimitsDispatch.holder(obj).getSelectedOperationalLimitsGroupId().orElse(null);
            case COUNTRY -> ((Substation) obj).getCountry().map(Country::name).orElse("");
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
        PropertyTable.StringProperty table = PropertyTable.stringProperty(property);
        if (table != null && table.set() != null) {
            table.set().accept(obj, value);
            return;
        }
        switch (property) {
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
            case COUNTRY -> ((Substation) obj).setCountry(Country.valueOf(value));
            case NETWORK_CASE_DATE -> ((Network) obj).setCaseDate(java.time.ZonedDateTime.parse(value));
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
            case EXT_FEEDER_ORDER           -> ((ConnectablePosition.Feeder) obj).getOrder()
                                               .map(OptionalInt::of).orElse(OptionalInt.empty());
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
            case EXT_FEEDER_NAME -> ((ConnectablePosition.Feeder) obj).getName().orElse(null);
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
        if (listCode == EXT_SVC_PILOT_POINT_IDS) {
            return ((PilotPoint) obj).getBusbarSectionsOrBusesIds().toArray(String[]::new);
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
            case EXT_RP_PRIORITY -> switch (obj) {
                case Injection<?> injection -> {
                    if (args.length != 0) throw wrongArgumentCount(code, args);
                    yield ReferencePriority.get(injection);
                }
                case Branch<?> branch -> {
                    if (args.length != 1) throw wrongArgumentCount(code, args);
                    yield ReferencePriority.get(branch, Creation.enumValue(TwoSides.class, (int) args[0]));
                }
                default -> {
                    if (args.length != 1) throw wrongArgumentCount(code, args);
                    yield ReferencePriority.get((ThreeWindingsTransformer) obj, Creation.enumValue(ThreeSides.class, (int) args[0]));
                }
            };
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
            case TERMINAL -> switch (obj) {
                case Bus bus -> bus.getConnectedTerminalStream();
                case ReferenceTerminals referenceTerminals -> referenceTerminals.getReferenceTerminals().stream();
                default -> ((Connectable<?>) obj).getTerminals().stream();
            };
            case REFERENCE_PRIORITY -> obj instanceof Network network
                ? ReferencePriorities.get(network).stream()
                : ((ReferencePriorities<?>) obj).getReferencePriorities().stream();
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
            case COORDINATE -> ((LinePosition<?>) obj).getCoordinates().stream();
            case CONTROL_ZONE -> ((SecondaryVoltageControl) obj).getControlZones().stream();
            case CONTROL_UNIT -> ((ControlZone) obj).getControlUnits().stream();
            case MUTUAL_COUPLING -> ((LineCouplings) obj).getMutualCouplings().stream();
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
            case AREA_CHARACTERISTICS -> observabilityAreas((ObservabilityArea) obj);
            case OBS_AREA_TERMINALS   -> observabilityAreaTerminals(obj);
            case OBS_AREA_BUSES       -> observabilityAreaBuses(obj);
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
                if (obj instanceof ReferencePriority priority) yield priority.getTerminal();
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
            case COORDINATE -> ((SubstationPosition) obj).getCoordinate();
            case PILOT_POINT -> ((ControlZone) obj).getPilotPoint();
            case EXT_MC_LINE1 -> ((MutualCoupling) obj).getLine1();
            case EXT_MC_LINE2 -> ((MutualCoupling) obj).getLine2();
            case EXT_MC_LINE1_SEGMENT -> ((MutualCoupling) obj).getLine1Segment();
            case EXT_MC_LINE2_SEGMENT -> ((MutualCoupling) obj).getLine2Segment();
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
            default -> {
                // An extension type code: the host's extension of that type
                if (Extensions.isType(relation)) yield Extensions.get(obj, relation);
                throw new BridgeErrors.PropertyNotFoundException("Unknown relation: " + relation);
            }
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
            case ADD_REFERENCE_TERMINAL -> ((ReferenceTerminals) obj).addReferenceTerminal((Terminal) value);
            case ADD_MUTUAL_COUPLING -> ((LineCouplings) obj).add((MutualCoupling) value);
            case REMOVE_AREA_BOUNDARY -> {
                if (value instanceof Terminal terminal) ((Area) obj).removeAreaBoundary(terminal);
                else ((Area) obj).removeAreaBoundary((Boundary) value);
            }
            // Only clearing (INVALID_HANDLE): new ones are created with invoke
            case VOLTAGE_REGULATION, REL_SELECTED_OLG, CURRENT_LIMITS, ACTIVE_POWER_LIMITS,
                 APPARENT_POWER_LIMITS -> Creation.clear(obj, relation, value);
            default -> {
                // An extension type code: removes the host's extension of that type (clearing only too)
                if (!Extensions.isType(relation)) {
                    throw new BridgeErrors.PropertyNotFoundException("Unknown relation for set: " + relation);
                }
                Creation.clear(obj, relation, value);
            }
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
            case ACT_REMOVE_ORDER -> {
                ((ConnectablePosition.Feeder) obj).removeOrder();
                yield null;
            }
            case ACT_DELETE_REFERENCE_PRIORITIES -> {
                if (obj instanceof Network network) ReferencePriorities.delete(network);
                else ((ReferencePriorities<?>) obj).deleteReferencePriorities();
                yield null;
            }
            case ACT_RESET_REFERENCE_TERMINALS -> {
                if (obj instanceof Network network) ReferenceTerminals.reset(network);
                else ((ReferenceTerminals) obj).reset();
                yield null;
            }
            case ACT_ADD_REFERENCE_TERMINAL -> {
                ReferenceTerminals.addTerminal((Terminal) obj);
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

    /**
     * Connects (disconnects) the same through the switches the predicate accepts, on a side (TwoSides
     * ordinal for an HVDC line or a tie line, ThreeSides for a connectable; null: every side).
     */
    static boolean connect(Object obj, boolean connect, Predicate<Switch> switches, Integer side) {
        return switch (obj) {
            case Terminal terminal -> connect ? terminal.connect(switches) : terminal.disconnect(switches);
            case HvdcLine line -> connect ? line.connectConverterStations(switches, side(TwoSides.class, side))
                : line.disconnectConverterStations(switches, side(TwoSides.class, side));
            case TieLine line -> connect ? line.connectBoundaryLines(switches, side(TwoSides.class, side))
                : line.disconnectBoundaryLines(switches, side(TwoSides.class, side));
            default -> connect ? ((Connectable<?>) obj).connect(switches, side(ThreeSides.class, side))
                : ((Connectable<?>) obj).disconnect(switches, side(ThreeSides.class, side));
        };
    }

    private static <E extends Enum<E>> E side(Class<E> type, Integer ordinal) {
        return ordinal == null ? null : Creation.enumValue(type, ordinal);
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
            case OBS_AREA_NODES -> observabilityAreaNodes(obj);
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
            case CONTROL_ZONE -> ((SecondaryVoltageControl) obj).getControlZone(key).orElse(null);
            case CONTROL_UNIT -> ((ControlZone) obj).getControlUnit(key).orElse(null);
            case REL_OBS_AREA_BY_BUS, REL_OBS_AREA_BY_BBV_BUS
                -> observabilityAreaByBus((ObservabilityArea) obj, relation, key);
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
            case FEEDER -> switch (index) {
                case 0 -> ((ConnectablePosition<?>) obj).getFeeder();
                case 1 -> ((ConnectablePosition<?>) obj).getFeeder1();
                case 2 -> ((ConnectablePosition<?>) obj).getFeeder2();
                case 3 -> ((ConnectablePosition<?>) obj).getFeeder3();
                default -> throw new IllegalArgumentException("Invalid feeder slot: " + index);
            };
            case LEG_FORTESCUE -> switch (Creation.enumValue(ThreeSides.class, index)) {
                case ONE -> ((ThreeWindingsTransformerFortescue) obj).getLeg1();
                case TWO -> ((ThreeWindingsTransformerFortescue) obj).getLeg2();
                case THREE -> ((ThreeWindingsTransformerFortescue) obj).getLeg3();
            };
            case RATIO_TAP_CHANGER_STEP -> ((RatioTapChanger) obj).getStep(index);
            case PHASE_TAP_CHANGER_STEP -> ((PhaseTapChanger) obj).getStep(index);
            case REL_OBS_AREA_BY_NODE -> observabilityAreaByNode((ObservabilityArea) obj, index);
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

    /** The ordinals of the regulation modes that satisfy a test. */
    private static int[] modes(java.util.function.Predicate<RegulationMode> test) {
        return Arrays.stream(RegulationMode.values()).filter(test).mapToInt(Enum::ordinal).toArray();
    }

    /**
     * An area of an observability area extension, with the voltage level it belongs to: the voltage
     * level is needed to resolve the area's nodes and bus-breaker bus ids into objects.
     */
    record ObservabilityAreaRef(VoltageLevel voltageLevel, ObservabilityArea.AreaCharacteristics area) {
    }

    private static Stream<?> observabilityAreas(ObservabilityArea ext) {
        VoltageLevel vl = ext.getExtendable();
        return ext.getObservabilityAreas().stream()
            // powsybl returns them in no particular order
            .sorted(Comparator.comparingInt(ObservabilityArea.AreaCharacteristics::getAreaNumber))
            .map(area -> new ObservabilityAreaRef(vl, area));
    }

    private static Object observabilityAreaByNode(ObservabilityArea ext, int node) {
        return observabilityArea(ext.getExtendable(), ext.getNodeBreakerView().getObservabilityArea(node));
    }

    private static Object observabilityAreaByBus(ObservabilityArea ext, int relation, String busId) {
        ObservabilityArea.BusView view = relation == REL_OBS_AREA_BY_BBV_BUS ? ext.getBusBreakerView() : ext.getBusView();
        return observabilityArea(ext.getExtendable(), view.getObservabilityArea(busId, false));
    }

    private static Object observabilityArea(VoltageLevel vl, ObservabilityArea.AreaCharacteristics area) {
        return area == null ? null : new ObservabilityAreaRef(vl, area);
    }

    // The node-breaker data of an area of a bus-breaker voltage level (and vice versa) throws when
    // used, so the topology kind of the voltage level decides which data is read.

    private static boolean isNodeBreaker(ObservabilityAreaRef ref) {
        return ref.voltageLevel().getTopologyKind() == TopologyKind.NODE_BREAKER;
    }

    /** Sorted node numbers of a node-breaker area, empty for a bus-breaker area. */
    private static int[] observabilityAreaNodes(Object obj) {
        ObservabilityAreaRef ref = (ObservabilityAreaRef) obj;
        if (!isNodeBreaker(ref)) {
            return new int[0];
        }
        return ref.area().getNodeBreakerData().getNodes().stream().mapToInt(Integer::intValue).sorted().toArray();
    }

    private static Stream<?> observabilityAreaTerminals(Object obj) {
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
    private static Stream<?> observabilityAreaBuses(Object obj) {
        ObservabilityAreaRef ref = (ObservabilityAreaRef) obj;
        if (isNodeBreaker(ref)) {
            return Stream.empty();
        }
        VoltageLevel.BusBreakerView view = ref.voltageLevel().getBusBreakerView();
        return ref.area().getBusBreakerData().getBusIds().stream().sorted()
            .map(view::getBus).filter(Objects::nonNull);
    }
}
