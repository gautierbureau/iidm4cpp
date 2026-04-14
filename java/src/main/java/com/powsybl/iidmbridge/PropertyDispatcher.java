package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;

import java.util.stream.Stream;
import java.util.stream.StreamSupport;

import static com.powsybl.iidmbridge.PropertyCodes.*;

/**
 * Dispatches property get/set operations to the correct PowSyBl IIDM Java methods
 * based on integer property codes defined in {@link PropertyCodes}.
 *
 * Used by the GraalVM backend entry points and can also be reused from the JNI side.
 */
public final class PropertyDispatcher {

    private PropertyDispatcher() {}

    // ── Double getters ────────────────────────────────────────────────────

    public static double getDouble(long handle, int property) {
        Object obj = NetworkRegistry.lookup(handle);
        return switch (property) {
            case GEN_TARGET_P  -> ((Generator) obj).getTargetP();
            case GEN_TARGET_Q  -> ((Generator) obj).getTargetQ();
            case GEN_TARGET_V  -> ((Generator) obj).getTargetV();
            case GEN_MIN_P     -> ((Generator) obj).getMinP();
            case GEN_MAX_P     -> ((Generator) obj).getMaxP();
            case LOAD_P0       -> ((Load) obj).getP0();
            case LOAD_Q0       -> ((Load) obj).getQ0();
            case TERMINAL_P    -> ((Terminal) obj).getP();
            case TERMINAL_Q    -> ((Terminal) obj).getQ();
            case BUS_V         -> ((Bus) obj).getV();
            case BUS_ANGLE     -> ((Bus) obj).getAngle();
            case VL_NOMINAL_V  -> ((VoltageLevel) obj).getNominalV();
            case VL_LOW_VOLTAGE_LIMIT  -> ((VoltageLevel) obj).getLowVoltageLimit();
            case VL_HIGH_VOLTAGE_LIMIT -> ((VoltageLevel) obj).getHighVoltageLimit();
            case LINE_R  -> ((Line) obj).getR();
            case LINE_X  -> ((Line) obj).getX();
            case LINE_G1 -> ((Line) obj).getG1();
            case LINE_B1 -> ((Line) obj).getB1();
            case LINE_G2 -> ((Line) obj).getG2();
            case LINE_B2 -> ((Line) obj).getB2();
            case TWO_WT_R        -> ((TwoWindingsTransformer) obj).getR();
            case TWO_WT_X        -> ((TwoWindingsTransformer) obj).getX();
            case TWO_WT_G        -> ((TwoWindingsTransformer) obj).getG();
            case TWO_WT_B        -> ((TwoWindingsTransformer) obj).getB();
            case TWO_WT_RATED_U1 -> ((TwoWindingsTransformer) obj).getRatedU1();
            case TWO_WT_RATED_U2 -> ((TwoWindingsTransformer) obj).getRatedU2();
            case TWO_WT_RATED_S  -> ((TwoWindingsTransformer) obj).getRatedS();
            case HVDC_R                     -> ((HvdcLine) obj).getR();
            case HVDC_NOMINAL_V             -> ((HvdcLine) obj).getNominalV();
            case HVDC_ACTIVE_POWER_SETPOINT -> ((HvdcLine) obj).getActivePowerSetpoint();
            case HVDC_MAX_P                 -> ((HvdcLine) obj).getMaxP();
            case DL_P0 -> ((DanglingLine) obj).getP0();
            case DL_Q0 -> ((DanglingLine) obj).getQ0();
            case DL_R  -> ((DanglingLine) obj).getR();
            case DL_X  -> ((DanglingLine) obj).getX();
            case DL_G  -> ((DanglingLine) obj).getG();
            case DL_B  -> ((DanglingLine) obj).getB();
            case SHUNT_B_PER_SECTION -> ((ShuntCompensator) obj).getB();
            case SVC_B_MIN                   -> ((StaticVarCompensator) obj).getBmin();
            case SVC_B_MAX                   -> ((StaticVarCompensator) obj).getBmax();
            case SVC_VOLTAGE_SETPOINT        -> ((StaticVarCompensator) obj).getVoltageSetpoint();
            case SVC_REACTIVE_POWER_SETPOINT -> ((StaticVarCompensator) obj).getReactivePowerSetpoint();
            case VSC_VOLTAGE_SETPOINT        -> ((VscConverterStation) obj).getVoltageSetpoint();
            case VSC_REACTIVE_POWER_SETPOINT -> ((VscConverterStation) obj).getReactivePowerSetpoint();
            default -> throw new IllegalArgumentException("Unknown double property: " + property);
        };
    }

    // ── Double setters ────────────────────────────────────────────────────

    public static void setDouble(long handle, int property, double value) {
        Object obj = NetworkRegistry.lookup(handle);
        switch (property) {
            case GEN_TARGET_P  -> ((Generator) obj).setTargetP(value);
            case GEN_TARGET_Q  -> ((Generator) obj).setTargetQ(value);
            case GEN_TARGET_V  -> ((Generator) obj).setTargetV(value);
            case LOAD_P0       -> ((Load) obj).setP0(value);
            case LOAD_Q0       -> ((Load) obj).setQ0(value);
            case TERMINAL_P    -> ((Terminal) obj).setP(value);
            case TERMINAL_Q    -> ((Terminal) obj).setQ(value);
            case BUS_V         -> ((Bus) obj).setV(value);
            case BUS_ANGLE     -> ((Bus) obj).setAngle(value);
            case HVDC_ACTIVE_POWER_SETPOINT -> ((HvdcLine) obj).setActivePowerSetpoint(value);
            case DL_P0 -> ((DanglingLine) obj).setP0(value);
            case DL_Q0 -> ((DanglingLine) obj).setQ0(value);
            case SVC_VOLTAGE_SETPOINT        -> ((StaticVarCompensator) obj).setVoltageSetpoint(value);
            case SVC_REACTIVE_POWER_SETPOINT -> ((StaticVarCompensator) obj).setReactivePowerSetpoint(value);
            case VSC_VOLTAGE_SETPOINT        -> ((VscConverterStation) obj).setVoltageSetpoint(value);
            case VSC_REACTIVE_POWER_SETPOINT -> ((VscConverterStation) obj).setReactivePowerSetpoint(value);
            default -> throw new IllegalArgumentException("Unknown double property for set: " + property);
        }
    }

    // ── Int getters ───────────────────────────────────────────────────────

    public static int getInt(long handle, int property) {
        Object obj = NetworkRegistry.lookup(handle);
        return switch (property) {
            case GEN_ENERGY_SOURCE  -> ((Generator) obj).getEnergySource().ordinal();
            case LOAD_TYPE          -> ((Load) obj).getLoadType().ordinal();
            case VL_TOPOLOGY_KIND   -> ((VoltageLevel) obj).getTopologyKind().ordinal();
            case HVDC_CONVERTERS_MODE -> ((HvdcLine) obj).getConvertersMode().ordinal();
            case SVC_REGULATION_MODE  -> ((StaticVarCompensator) obj).getRegulationMode().ordinal();
            case SHUNT_SECTION_COUNT     -> ((ShuntCompensator) obj).getSectionCount();
            case SHUNT_MAX_SECTION_COUNT -> ((ShuntCompensator) obj).getMaximumSectionCount();
            default -> throw new IllegalArgumentException("Unknown int property: " + property);
        };
    }

    // ── Int setters ───────────────────────────────────────────────────────

    public static void setInt(long handle, int property, int value) {
        Object obj = NetworkRegistry.lookup(handle);
        switch (property) {
            case HVDC_CONVERTERS_MODE ->
                ((HvdcLine) obj).setConvertersMode(HvdcLine.ConvertersMode.values()[value]);
            case SVC_REGULATION_MODE ->
                ((StaticVarCompensator) obj).setRegulationMode(
                    StaticVarCompensator.RegulationMode.values()[value]);
            case SHUNT_SECTION_COUNT ->
                ((ShuntCompensator) obj).setSectionCount(value);
            default -> throw new IllegalArgumentException("Unknown int property for set: " + property);
        }
    }

    // ── Bool getters ──────────────────────────────────────────────────────

    public static int getBool(long handle, int property) {
        Object obj = NetworkRegistry.lookup(handle);
        boolean val = switch (property) {
            case GEN_VOLTAGE_REGULATOR_ON -> ((Generator) obj).isVoltageRegulatorOn();
            case TERMINAL_CONNECTED       -> ((Terminal) obj).isConnected();
            case VSC_VOLTAGE_REGULATOR_ON -> ((VscConverterStation) obj).isVoltageRegulatorOn();
            default -> throw new IllegalArgumentException("Unknown bool property: " + property);
        };
        return val ? 1 : 0;
    }

    // ── Bool setters ──────────────────────────────────────────────────────

    public static void setBool(long handle, int property, int value) {
        Object obj = NetworkRegistry.lookup(handle);
        boolean bval = value != 0;
        switch (property) {
            case GEN_VOLTAGE_REGULATOR_ON -> ((Generator) obj).setVoltageRegulatorOn(bval);
            case TERMINAL_CONNECTED -> {
                if (bval) ((Terminal) obj).connect();
                else      ((Terminal) obj).disconnect();
            }
            case VSC_VOLTAGE_REGULATOR_ON -> ((VscConverterStation) obj).setVoltageRegulatorOn(bval);
            default -> throw new IllegalArgumentException("Unknown bool property for set: " + property);
        }
    }

    // ── String getters ────────────────────────────────────────────────────

    public static String getString(long handle, int property) {
        Object obj = NetworkRegistry.lookup(handle);
        return switch (property) {
            case ID   -> ((Identifiable<?>) obj).getId();
            case NAME -> ((Identifiable<?>) obj).getNameOrId();
            case TERMINAL_BUS_ID -> {
                Terminal t = (Terminal) obj;
                Bus bus = t.getBusView().getConnectableBus();
                yield bus != null ? bus.getId() : "";
            }
            default -> throw new IllegalArgumentException("Unknown string property: " + property);
        };
    }

    // ── String setters ────────────────────────────────────────────────────

    public static void setString(long handle, int property, String value) {
        Object obj = NetworkRegistry.lookup(handle);
        switch (property) {
            case NAME    -> ((Identifiable<?>) obj).setName(value);
            case COUNTRY -> ((Substation) obj).setCountry(Country.valueOf(value));
            default -> throw new IllegalArgumentException("Unknown string property for set: " + property);
        }
    }

    // ── Children (navigation) ─────────────────────────────────────────────

    public static long[] getChildren(long parentHandle, int childType) {
        Object obj = NetworkRegistry.lookup(parentHandle);
        Stream<?> children = switch (childType) {
            case GENERATOR   -> ((Network) obj).getGeneratorStream();
            case LOAD        -> ((Network) obj).getLoadStream();
            case LINE        -> ((Network) obj).getLineStream();
            case SUBSTATION  -> ((Network) obj).getSubstationStream();
            case VOLTAGE_LEVEL -> ((Network) obj).getVoltageLevelStream();
            case TWO_WINDINGS_TRANSFORMER   -> ((Network) obj).getTwoWindingsTransformerStream();
            case THREE_WINDINGS_TRANSFORMER -> ((Network) obj).getThreeWindingsTransformerStream();
            case HVDC_LINE           -> ((Network) obj).getHvdcLineStream();
            case DANGLING_LINE       -> ((Network) obj).getDanglingLineStream();
            case SHUNT_COMPENSATOR   -> ((Network) obj).getShuntCompensatorStream();
            case STATIC_VAR_COMPENSATOR -> ((Network) obj).getStaticVarCompensatorStream();
            case VSC_CONVERTER_STATION  -> ((Network) obj).getVscConverterStationStream();
            case LCC_CONVERTER_STATION  -> ((Network) obj).getLccConverterStationStream();
            default -> throw new IllegalArgumentException("Unknown child type: " + childType);
        };
        return children.mapToLong(NetworkRegistry::register).toArray();
    }

    // ── Related object (navigation) ───────────────────────────────────────

    public static long getRelated(long handle, int relation) {
        Object obj = NetworkRegistry.lookup(handle);
        Object related = switch (relation) {
            case REL_TERMINAL  -> {
                if (obj instanceof Injection<?> inj) yield inj.getTerminal();
                throw new IllegalArgumentException("Object does not have a single terminal: " + obj);
            }
            case REL_TERMINAL1 -> {
                if (obj instanceof Branch<?> br) yield br.getTerminal1();
                throw new IllegalArgumentException("Object does not have terminal1: " + obj);
            }
            case REL_TERMINAL2 -> {
                if (obj instanceof Branch<?> br) yield br.getTerminal2();
                throw new IllegalArgumentException("Object does not have terminal2: " + obj);
            }
            case REL_BUS -> {
                Terminal t = (Terminal) obj;
                yield t.getBusView().getConnectableBus();
            }
            case REL_VOLTAGE_LEVEL -> ((Terminal) obj).getVoltageLevel();
            default -> throw new IllegalArgumentException("Unknown relation: " + relation);
        };
        if (related == null) return 0L; // INVALID_HANDLE
        return NetworkRegistry.register(related);
    }

    // ── Find by ID ────────────────────────────────────────────────────────

    public static long findById(long networkHandle, int objectType, String id) {
        Object netObj = NetworkRegistry.lookup(networkHandle);
        Network network = (Network) netObj;
        Object found = switch (objectType) {
            case GENERATOR                  -> network.getGenerator(id);
            case LOAD                       -> network.getLoad(id);
            case LINE                       -> network.getLine(id);
            case TWO_WINDINGS_TRANSFORMER   -> network.getTwoWindingsTransformer(id);
            case THREE_WINDINGS_TRANSFORMER -> network.getThreeWindingsTransformer(id);
            case HVDC_LINE                  -> network.getHvdcLine(id);
            case DANGLING_LINE              -> network.getDanglingLine(id);
            case SHUNT_COMPENSATOR          -> network.getShuntCompensator(id);
            case STATIC_VAR_COMPENSATOR     -> network.getStaticVarCompensator(id);
            case VSC_CONVERTER_STATION      -> network.getVscConverterStation(id);
            case LCC_CONVERTER_STATION      -> network.getLccConverterStation(id);
            default -> throw new IllegalArgumentException("Unknown object type: " + objectType);
        };
        if (found == null) return 0L; // INVALID_HANDLE
        return NetworkRegistry.register(found);
    }

    // ── Extensions ────────────────────────────────────────────────────────

    public static long getExtensionHandle(long handle, String name) {
        Identifiable<?> obj = (Identifiable<?>) NetworkRegistry.lookup(handle);
        Extension<?> ext = obj.getExtensionByName(name);
        if (ext == null) return 0L;
        return NetworkRegistry.register(ext);
    }

    public static String getExtensionNamesJoined(long handle) {
        Identifiable<?> obj = (Identifiable<?>) NetworkRegistry.lookup(handle);
        return obj.getExtensions().stream()
            .map(Extension::getName)
            .collect(java.util.stream.Collectors.joining(";"));
    }

    public static String getExtensionAttribute(long extensionHandle, String key) {
        Object ext = NetworkRegistry.lookup(extensionHandle);
        String capKey = Character.toUpperCase(key.charAt(0)) + key.substring(1);
        for (String prefix : new String[]{"get", "is"}) {
            try {
                java.lang.reflect.Method m = ext.getClass().getMethod(prefix + capKey);
                Object result = m.invoke(ext);
                return result != null ? result.toString() : "";
            } catch (NoSuchMethodException ignored) {
                // try next prefix
            } catch (Exception e) {
                throw new RuntimeException("Error invoking " + prefix + capKey + ": " + e.getMessage(), e);
            }
        }
        throw new IllegalArgumentException("Unknown attribute '" + key + "' for extension: " + ext.getClass().getName());
    }
}
