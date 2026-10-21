package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Injections through {@link PropertyDispatcher}: load models (and their adders), the setters of
 * loads, batteries and LCC converter stations, the targets from the solved values.
 */
class InjectionDispatchTest {

    private Network network;
    private VoltageLevel vl;
    private Load zipLoad;
    private Load expLoad;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("injections", "test");
        vl = network.newVoltageLevel().setId("VL").setNominalV(225).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl.getBusBreakerView().newBus().setId("B").add();
        zipLoad = vl.newLoad().setId("ZIP").setBus("B").setP0(10).setQ0(2)
            .newZipModel().setC0p(0.5).setC1p(0.3).setC2p(0.2).setC0q(0.4).setC1q(0.4).setC2q(0.2).add()
            .add();
        expLoad = vl.newLoad().setId("EXP").setBus("B").setP0(5).setQ0(1)
            .newExponentialModel().setNp(1.2).setNq(2.1).add()
            .add();
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

    @Test void loadModels() {
        long zip = PropertyDispatcher.getRelated(reg(zipLoad), LOAD_MODEL);
        assertSame(zipLoad.getModel().orElseThrow(), obj(zip));
        assertEquals(1, PropertyDispatcher.getBool(zip, LOAD_MODEL));
        assertEquals(1, PropertyDispatcher.getBool(zip, ZIP_LOAD_MODEL));
        assertEquals(0, PropertyDispatcher.getBool(zip, EXPONENTIAL_LOAD_MODEL));
        assertEquals(LoadModelType.ZIP.ordinal(), PropertyDispatcher.getInt(zip, LOAD_MODEL_TYPE));
        assertEquals(0.3, PropertyDispatcher.getDouble(zip, ZIP_C1P));
        PropertyDispatcher.setDouble(zip, ZIP_C0P, 0.6);
        PropertyDispatcher.setDouble(zip, ZIP_C2P, 0.1);
        assertEquals(0.6, ((ZipLoadModel) zipLoad.getModel().orElseThrow()).getC0p());

        long exp = PropertyDispatcher.getRelated(reg(expLoad), LOAD_MODEL);
        assertEquals(LoadModelType.EXPONENTIAL.ordinal(), PropertyDispatcher.getInt(exp, LOAD_MODEL_TYPE));
        assertEquals(2.1, PropertyDispatcher.getDouble(exp, EXP_NQ));
        PropertyDispatcher.setDouble(exp, EXP_NP, 1.5);
        assertEquals(1.5, ((ExponentialLoadModel) expLoad.getModel().orElseThrow()).getNp());
        PropertyDispatcher.setKeyedString(exp, PROPERTY, "source", "measured");
        assertEquals("measured", expLoad.getModel().orElseThrow().getProperty("source"));

        Load plain = vl.newLoad().setId("PLAIN").setBus("B").setP0(1).setQ0(0).add();
        assertEquals(0L, PropertyDispatcher.getRelated(reg(plain), LOAD_MODEL));
    }

    // The model of a new load: a nested adder of the load adder
    @Test void loadModelAdders() {
        long adder = PropertyDispatcher.invoke(reg(vl), LOAD);
        PropertyDispatcher.setString(adder, ID, "NEW");
        PropertyDispatcher.setString(adder, TERMINAL_BUS_BREAKER_BUS_ID, "B");
        PropertyDispatcher.setDouble(adder, LOAD_P0, 3);
        PropertyDispatcher.setDouble(adder, LOAD_Q0, 1);
        long model = PropertyDispatcher.invoke(adder, EXPONENTIAL_LOAD_MODEL);
        PropertyDispatcher.setDouble(model, EXP_NP, 0.8);
        PropertyDispatcher.setDouble(model, EXP_NQ, 1.6);
        PropertyDispatcher.setKeyedString(model, PROPERTY, "k", "v");
        assertEquals(0L, PropertyDispatcher.invoke(model, ACT_ADD));
        Load load = (Load) obj(PropertyDispatcher.invoke(adder, ACT_ADD));
        ExponentialLoadModel created = (ExponentialLoadModel) load.getModel().orElseThrow();
        assertEquals(1.6, created.getNq());
        assertEquals("v", created.getProperty("k"));
    }

    @Test void setters() {
        long load = reg(zipLoad);
        PropertyDispatcher.setInt(load, LOAD_TYPE, LoadType.AUXILIARY.ordinal());
        assertEquals(LoadType.AUXILIARY, zipLoad.getLoadType());

        Battery battery = vl.newBattery().setId("BAT").setBus("B").setMinP(-10).setMaxP(10).setTargetP(0).setLocalTargetQ(0).add();
        PropertyDispatcher.setDouble(reg(battery), BAT_MIN_P, -20);
        PropertyDispatcher.setDouble(reg(battery), BAT_MAX_P, 30);
        assertEquals(-20, battery.getMinP());
        assertEquals(30, battery.getMaxP());

        LccConverterStation lcc = vl.newLccConverterStation().setId("LCC").setBus("B").setLossFactor(0.01f).setPowerFactor(0.8f).add();
        PropertyDispatcher.setDouble(reg(lcc), LCC_POWER_FACTOR, 0.9);
        assertEquals(0.9f, lcc.getPowerFactor());
    }

    // The targets of loads, generators and batteries from their solved flows
    @Test void targetsFromSolvedValues() {
        zipLoad.getTerminal().setP(12).setQ(3);
        PropertyDispatcher.invoke(reg(zipLoad), ACT_SET_TARGET_P_TO_P);
        assertEquals(12, zipLoad.getP0());
        PropertyDispatcher.invoke(reg(zipLoad), ACT_SET_TARGET_Q_TO_Q);
        assertEquals(3, zipLoad.getQ0());

        Generator generator = vl.newGenerator().setId("G").setBus("B").setMinP(0).setMaxP(100).setTargetP(10)
            .setLocalTargetQ(0).add();
        generator.getTerminal().setP(-40);
        PropertyDispatcher.invoke(reg(generator), ACT_SET_TARGET_P_TO_P);
        assertEquals(40, generator.getTargetP());
        Battery battery = vl.newBattery().setId("BAT").setBus("B").setMinP(-50).setMaxP(50).setTargetP(0).setLocalTargetQ(0).add();
        battery.getTerminal().setP(-15).setQ(-2);
        PropertyDispatcher.invoke(reg(battery), ACT_APPLY_SOLVED_VALUES);
        assertEquals(15, battery.getTargetP());
    }
}
