package com.powsybl.iidmbridge;

import com.powsybl.commons.extensions.Extendable;
import com.powsybl.commons.extensions.ExtensionAdder;
import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.ActivePowerControl;
import com.powsybl.iidm.network.extensions.ActivePowerControlAdder;
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
            case REL_TERMINAL -> new TerminalChange((VoltageRegulation) owner);
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
            case VoltageRegulationBuilder builder -> builder.build();
            case VoltageRegulationAdder<?> adder -> {
                adder.add();
                yield null;
            }
            case TerminalChange change -> {
                change.apply();
                yield null;
            }
            default -> throw new BridgeErrors.PropertyNotFoundException("Not an adder: " + pending.adder());
        };
    }

    /** Removes an identifiable from its network, or a tap changer from its transformer. */
    static void remove(Object obj) {
        switch (obj) {
            case Connectable<?> connectable -> connectable.remove();
            case TapChanger<?, ?, ?, ?> tapChanger -> tapChanger.remove();
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
            default -> throw unknownField("int", property);
        }
    }

    static void setBool(PendingAdd pending, int property, boolean value) {
        Object a = pending.adder();
        switch (property) {
            case FICTITIOUS -> ((IdentifiableAdder<?, ?>) a).setFictitious(value);
            case ENSURE_ID_UNICITY -> ((IdentifiableAdder<?, ?>) a).setEnsureIdUnicity(value);
            case EQUIVALENT -> ((ConnectableAdder<?, ?>) a).setEquivalent(value);
            case GEN_CONDENSER -> ((GeneratorAdder) a).setCondenser(value);
            case VR_REGULATING -> ((VoltageRegulationAdderOrBuilder<?>) a).withRegulating(value);
            case EXT_APC_PARTICIPATE -> ((ActivePowerControlAdder<?>) a).withParticipate(value);
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
            default -> throw unknownField("string", property);
        }
    }

    /** Properties of the adders that are properties holders (limits, reactive limits...). */
    static void setKeyedString(PendingAdd pending, int property, String key, String value) {
        Object a = pending.adder();
        switch (property) {
            case PROPERTY -> ((BasePropertiesHolder) a).setProperty(key, Objects.requireNonNull(value, "property value"));
            default -> throw unknownField("keyed string", property);
        }
    }

    static void setRelated(PendingAdd pending, int relation, Object value) {
        Object a = pending.adder();
        switch (relation) {
            case REL_TERMINAL -> {
                if (a instanceof TerminalChange change) {
                    change.terminal = (Terminal) value;
                } else {
                    ((VoltageRegulationAdderOrBuilder<?>) a).withTerminal((Terminal) value);
                }
            }
            default -> throw unknownField("related object", relation);
        }
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
