package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * The data extensions through {@link PropertyDispatcher}: each created on its host through its adder,
 * read, changed and removed. Short circuit (generator, battery, identifiable), ENTSO-E category and
 * startup of a generator, load detail, standby automaton, manual frequency restoration reserve,
 * operating status, dynamic model info.
 */
class DataExtensionDispatchTest {

    private Network network;
    private VoltageLevel vl;
    private Generator generator;
    private Battery battery;
    private Load load;
    private StaticVarCompensator svc;
    private Line line;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("data_extensions", "test");
        Substation s = network.newSubstation().setId("S").add();
        vl = s.newVoltageLevel().setId("VL").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl.getBusBreakerView().newBus().setId("B").add();
        VoltageLevel vl2 = s.newVoltageLevel().setId("VL2").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl2.getBusBreakerView().newBus().setId("B2").add();
        generator = vl.newGenerator().setId("G").setBus("B").setMinP(0).setMaxP(100).setTargetP(50).setLocalTargetQ(0).add();
        battery = vl.newBattery().setId("BAT").setBus("B").setMinP(-10).setMaxP(10).setTargetP(0).setLocalTargetQ(0).add();
        load = vl.newLoad().setId("LD").setBus("B").setP0(80).setQ0(10).add();
        svc = vl.newStaticVarCompensator().setId("SVC").setBus("B").setBmin(-0.01).setBmax(0.01).add();
        line = network.newLine().setId("L").setVoltageLevel1("VL").setBus1("B").setVoltageLevel2("VL2").setBus2("B2")
            .setR(1).setX(10).setG1(0).setB1(0).setG2(0).setB2(0).add();
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    private static Object obj(long handle) {
        return NetworkRegistry.lookup(handle);
    }

    /** The extension added to the host by the adder of its type, after the fields set by setFields. */
    private static long add(Object host, int type, java.util.function.LongConsumer setFields) {
        long adder = PropertyDispatcher.invoke(reg(host), type);
        setFields.accept(adder);
        long extension = PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(1, PropertyDispatcher.getBool(extension, type));
        assertSame(obj(extension), obj(PropertyDispatcher.getRelated(reg(host), type)));
        return extension;
    }

    private static void remove(Object host, int type) {
        PropertyDispatcher.setRelated(reg(host), type, 0L);
        assertEquals(0L, PropertyDispatcher.getRelated(reg(host), type));
    }

    @Test void shortCircuits() {
        long g = add(generator, GENERATOR_SHORT_CIRCUIT, a -> {
            PropertyDispatcher.setDouble(a, EXT_SC_DIRECT_SUBTRANS_X, 0.2);
            PropertyDispatcher.setDouble(a, EXT_SC_DIRECT_TRANS_X, 0.3);
            PropertyDispatcher.setDouble(a, EXT_SC_STEP_UP_TRANSFORMER_X, 0.1);
        });
        assertEquals(0.2, PropertyDispatcher.getDouble(g, EXT_SC_DIRECT_SUBTRANS_X));
        PropertyDispatcher.setDouble(g, EXT_SC_DIRECT_TRANS_X, 0.35);
        assertEquals(0.35, generator.getExtension(GeneratorShortCircuit.class).getDirectTransX());
        assertEquals(0, PropertyDispatcher.getBool(g, BATTERY_SHORT_CIRCUIT));

        long b = add(battery, BATTERY_SHORT_CIRCUIT, a -> PropertyDispatcher.setDouble(a, EXT_SC_DIRECT_TRANS_X, 0.4));
        assertEquals(0.4, PropertyDispatcher.getDouble(b, EXT_SC_DIRECT_TRANS_X));
        assertTrue(Double.isNaN(PropertyDispatcher.getDouble(b, EXT_SC_STEP_UP_TRANSFORMER_X)));

        long v = add(vl, IDENTIFIABLE_SHORT_CIRCUIT, a -> {
            PropertyDispatcher.setDouble(a, EXT_ISC_IP_MIN, 1000);
            PropertyDispatcher.setDouble(a, EXT_ISC_IP_MAX, 5000);
        });
        PropertyDispatcher.setDouble(v, EXT_ISC_IP_MAX, 6000);
        assertEquals(6000, PropertyDispatcher.getDouble(v, EXT_ISC_IP_MAX));
        assertEquals(1000, PropertyDispatcher.getDouble(v, EXT_ISC_IP_MIN));

        remove(generator, GENERATOR_SHORT_CIRCUIT);
        remove(battery, BATTERY_SHORT_CIRCUIT);
        remove(vl, IDENTIFIABLE_SHORT_CIRCUIT);
    }

    @Test void generatorData() {
        long category = add(generator, GENERATOR_ENTSOE_CATEGORY, a -> PropertyDispatcher.setInt(a, EXT_GEC_CODE, 4));
        assertEquals(4, PropertyDispatcher.getInt(category, EXT_GEC_CODE));
        PropertyDispatcher.setInt(category, EXT_GEC_CODE, 7);
        assertEquals(7, generator.getExtension(GeneratorEntsoeCategory.class).getCode());

        long startup = add(generator, GENERATOR_STARTUP, a -> {
            PropertyDispatcher.setDouble(a, EXT_GS_PLANNED_ACTIVE_POWER_SETPOINT, 40);
            PropertyDispatcher.setDouble(a, EXT_GS_STARTUP_COST, 1000);
            PropertyDispatcher.setDouble(a, EXT_GS_MARGINAL_COST, 30);
            PropertyDispatcher.setDouble(a, EXT_GS_PLANNED_OUTAGE_RATE, 0.1);
            PropertyDispatcher.setDouble(a, EXT_GS_FORCED_OUTAGE_RATE, 0.05);
        });
        assertEquals(1000, PropertyDispatcher.getDouble(startup, EXT_GS_STARTUP_COST));
        PropertyDispatcher.setDouble(startup, EXT_GS_MARGINAL_COST, 35);
        assertEquals(35, PropertyDispatcher.getDouble(startup, EXT_GS_MARGINAL_COST));
        assertEquals(0.05, PropertyDispatcher.getDouble(startup, EXT_GS_FORCED_OUTAGE_RATE));

        long reserve = add(generator, MANUAL_FREQUENCY_RESTORATION_RESERVE,
            a -> PropertyDispatcher.setBool(a, EXT_MFRR_PARTICIPATE, 1));
        assertEquals(1, PropertyDispatcher.getBool(reserve, EXT_MFRR_PARTICIPATE));
        PropertyDispatcher.setBool(reserve, EXT_MFRR_PARTICIPATE, 0);
        assertFalse(((ManualFrequencyRestorationReserve<?>) generator.getExtension(ManualFrequencyRestorationReserve.class)).isParticipate());
        remove(generator, MANUAL_FREQUENCY_RESTORATION_RESERVE);
    }

    @Test void loadAndCompensatorData() {
        long detail = add(load, LOAD_DETAIL, a -> {
            PropertyDispatcher.setDouble(a, EXT_LD_FIXED_ACTIVE_POWER, 30);
            PropertyDispatcher.setDouble(a, EXT_LD_FIXED_REACTIVE_POWER, 4);
            PropertyDispatcher.setDouble(a, EXT_LD_VARIABLE_ACTIVE_POWER, 50);
            PropertyDispatcher.setDouble(a, EXT_LD_VARIABLE_REACTIVE_POWER, 6);
        });
        assertEquals(50, PropertyDispatcher.getDouble(detail, EXT_LD_VARIABLE_ACTIVE_POWER));
        PropertyDispatcher.setDouble(detail, EXT_LD_FIXED_REACTIVE_POWER, 5);
        assertEquals(5, load.getExtension(LoadDetail.class).getFixedReactivePower());

        long automaton = add(svc, STANDBY_AUTOMATON, a -> {
            PropertyDispatcher.setBool(a, EXT_SA_STANDBY, 1);
            PropertyDispatcher.setDouble(a, EXT_SA_B0, 0.001);
            PropertyDispatcher.setDouble(a, EXT_SA_LOW_VOLTAGE_SETPOINT, 390);
            PropertyDispatcher.setDouble(a, EXT_SA_HIGH_VOLTAGE_SETPOINT, 410);
            PropertyDispatcher.setDouble(a, EXT_SA_LOW_VOLTAGE_THRESHOLD, 380);
            PropertyDispatcher.setDouble(a, EXT_SA_HIGH_VOLTAGE_THRESHOLD, 420);
        });
        assertEquals(1, PropertyDispatcher.getBool(automaton, EXT_SA_STANDBY));
        assertEquals(0.001, PropertyDispatcher.getDouble(automaton, EXT_SA_B0));
        PropertyDispatcher.setBool(automaton, EXT_SA_STANDBY, 0);
        PropertyDispatcher.setDouble(automaton, EXT_SA_HIGH_VOLTAGE_THRESHOLD, 425);
        StandbyAutomaton standby = svc.getExtension(StandbyAutomaton.class);
        assertFalse(standby.isStandby());
        assertEquals(425, standby.getHighVoltageThreshold());
        // In standby, a low threshold above the high one is rejected
        PropertyDispatcher.setBool(automaton, EXT_SA_STANDBY, 1);
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.setDouble(automaton, EXT_SA_LOW_VOLTAGE_THRESHOLD, 430))));
        remove(svc, STANDBY_AUTOMATON);
    }

    @Test void statusAndModel() {
        long status = add(line, OPERATING_STATUS,
            a -> PropertyDispatcher.setInt(a, EXT_OS_STATUS, OperatingStatus.Status.PLANNED_OUTAGE.ordinal()));
        assertEquals(OperatingStatus.Status.PLANNED_OUTAGE.ordinal(), PropertyDispatcher.getInt(status, EXT_OS_STATUS));
        PropertyDispatcher.setInt(status, EXT_OS_STATUS, OperatingStatus.Status.FORCED_OUTAGE.ordinal());
        assertEquals(OperatingStatus.Status.FORCED_OUTAGE,
            ((OperatingStatus<?>) line.getExtension(OperatingStatus.class)).getStatus());
        // Not on a load
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> add(load, OPERATING_STATUS, a -> { }))));

        long info = add(generator, DYNAMIC_MODEL_INFO, a -> PropertyDispatcher.setString(a, EXT_DMI_MODEL_NAME, "GeneratorFictitious"));
        assertEquals("GeneratorFictitious", PropertyDispatcher.getString(info, EXT_DMI_MODEL_NAME));
        PropertyDispatcher.setString(info, EXT_DMI_MODEL_NAME, "GeneratorSynchronous");
        assertEquals("GeneratorSynchronous", ((DynamicModelInfo<?>) generator.getExtension(DynamicModelInfo.class)).getModelName());
        remove(line, OPERATING_STATUS);
        remove(generator, DYNAMIC_MODEL_INFO);
    }
}
