package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Shunt compensators through {@link PropertyDispatcher}: their models (linear, non-linear and its
 * sections), section counts, susceptance and conductance, and solved values.
 */
class ShuntCompensatorDispatchTest {

    private Network network;
    private ShuntCompensator linear;
    private ShuntCompensator nonLinear;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("shunts", "test");
        VoltageLevel vl = network.newSubstation().setId("S").add()
            .newVoltageLevel().setId("VL").setNominalV(225).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl.getBusBreakerView().newBus().setId("B").add();
        linear = vl.newShuntCompensator().setId("LINEAR").setBus("B").setSectionCount(2)
            .newLinearModel().setBPerSection(12.0).setGPerSection(0.5).setMaximumSectionCount(3).add()
            .add();
        nonLinear = vl.newShuntCompensator().setId("NON_LINEAR").setBus("B").setSectionCount(1)
            .newNonLinearModel()
                .beginSection().setB(1e-3).setG(0).endSection()
                .beginSection().setB(2.5e-3).setG(1e-5).endSection()
                .add()
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

    private static int kindOfFailure(Runnable call) {
        Throwable t = assertThrows(Throwable.class, call::run);
        return BridgeErrors.kindOf(t);
    }

    @Test void linearModel() {
        long sc = reg(linear);
        assertEquals(ShuntCompensatorModelType.LINEAR.ordinal(), PropertyDispatcher.getInt(sc, SHUNT_MODEL_TYPE));
        long model = PropertyDispatcher.getRelated(sc, SHUNT_COMPENSATOR_MODEL);
        assertSame(linear.getModel(), obj(model));
        assertEquals(1, PropertyDispatcher.isSameObject(model, PropertyDispatcher.getRelated(sc, SHUNT_COMPENSATOR_LINEAR_MODEL)));
        assertEquals(1, PropertyDispatcher.getBool(model, SHUNT_COMPENSATOR_MODEL));
        assertEquals(1, PropertyDispatcher.getBool(model, SHUNT_COMPENSATOR_LINEAR_MODEL));
        assertEquals(0, PropertyDispatcher.getBool(model, SHUNT_COMPENSATOR_NON_LINEAR_MODEL));
        // getModel(Class) of the other type
        assertEquals(BridgeErrors.VALIDATION,
            kindOfFailure(() -> PropertyDispatcher.getRelated(sc, SHUNT_COMPENSATOR_NON_LINEAR_MODEL)));

        // Per section values: those of the model (not the totals at the section count)
        assertEquals(12.0, PropertyDispatcher.getDouble(model, SHUNT_B_PER_SECTION));
        assertEquals(0.5, PropertyDispatcher.getDouble(model, SHUNT_G_PER_SECTION));
        assertEquals(24.0, PropertyDispatcher.getDouble(sc, SHUNT_B));
        assertEquals(1.0, PropertyDispatcher.getDouble(sc, SHUNT_G));
        assertEquals(36.0, PropertyDispatcher.evaluate(sc, SHUNT_B, new double[] {3}));
        assertEquals(0.0, PropertyDispatcher.evaluate(sc, SHUNT_G, new double[] {0}));
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.evaluate(sc, SHUNT_B, new double[] {4})));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.evaluate(sc, SHUNT_B, new double[] {})));
        // Codes of the model, not of the shunt
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getDouble(sc, SHUNT_B_PER_SECTION)));

        PropertyDispatcher.setDouble(model, SHUNT_B_PER_SECTION, 10.0);
        PropertyDispatcher.setDouble(model, SHUNT_G_PER_SECTION, 0.25);
        PropertyDispatcher.setInt(model, SHUNT_MAX_SECTION_COUNT, 5);
        assertEquals(10.0, linear.getModel(ShuntCompensatorLinearModel.class).getBPerSection());
        assertEquals(0.25, linear.getModel(ShuntCompensatorLinearModel.class).getGPerSection());
        assertEquals(5, PropertyDispatcher.getInt(sc, SHUNT_MAX_SECTION_COUNT));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.setInt(sc, SHUNT_MAX_SECTION_COUNT, 6)));

        // A properties holder
        PropertyDispatcher.setKeyedString(model, PROPERTY, "maker", "abb");
        assertEquals("abb", linear.getModel().getProperty("maker"));
        assertArrayEquals(new String[] {"maker"}, PropertyDispatcher.getStringList(model, PROPERTY_NAMES));
    }

    @Test void nonLinearModel() {
        long sc = reg(nonLinear);
        assertEquals(ShuntCompensatorModelType.NON_LINEAR.ordinal(), PropertyDispatcher.getInt(sc, SHUNT_MODEL_TYPE));
        assertEquals(2, PropertyDispatcher.getInt(sc, SHUNT_MAX_SECTION_COUNT));
        long model = PropertyDispatcher.getRelated(sc, SHUNT_COMPENSATOR_NON_LINEAR_MODEL);
        assertEquals(1, PropertyDispatcher.isSameObject(model, PropertyDispatcher.getRelated(sc, SHUNT_COMPENSATOR_MODEL)));
        assertEquals(BridgeErrors.VALIDATION,
            kindOfFailure(() -> PropertyDispatcher.getRelated(sc, SHUNT_COMPENSATOR_LINEAR_MODEL)));
        // A linear model's code
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.setInt(model, SHUNT_MAX_SECTION_COUNT, 3)));

        long[] sections = PropertyDispatcher.getChildren(model, SHUNT_SECTION);
        assertEquals(2, sections.length);
        assertEquals(1, PropertyDispatcher.getBool(sections[0], SHUNT_SECTION));
        assertEquals(1e-3, PropertyDispatcher.getDouble(sections[0], SHUNT_SECTION_B));
        assertEquals(1e-5, PropertyDispatcher.getDouble(sections[1], SHUNT_SECTION_G));
        assertEquals(1, PropertyDispatcher.isSameObject(sections[1], PropertyDispatcher.getChildren(model, SHUNT_SECTION)[1]));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getChildren(sc, SHUNT_SECTION)));

        // B and G: of the section at the section count; of none at 0
        assertEquals(1e-3, PropertyDispatcher.getDouble(sc, SHUNT_B));
        assertEquals(2.5e-3, PropertyDispatcher.evaluate(sc, SHUNT_B, new double[] {2}));
        assertEquals(0.0, PropertyDispatcher.evaluate(sc, SHUNT_G, new double[] {0}));
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.evaluate(sc, SHUNT_G, new double[] {3})));

        PropertyDispatcher.setDouble(sections[0], SHUNT_SECTION_B, 2e-3);
        PropertyDispatcher.setDouble(sections[0], SHUNT_SECTION_G, 1e-6);
        assertEquals(2e-3, nonLinear.getB());
        assertEquals(1e-6, nonLinear.getG());
        PropertyDispatcher.setKeyedString(sections[0], PROPERTY, "stage", "first");
        assertEquals("first", PropertyDispatcher.getKeyedString(sections[0], PROPERTY, "stage"));
    }

    @Test void sectionCounts() {
        long sc = reg(linear);
        assertEquals(2, PropertyDispatcher.getInt(sc, SHUNT_SECTION_COUNT));
        assertEquals(PropertyDispatcher.NO_INT, PropertyDispatcher.getOptInt(sc, SHUNT_SOLVED_SECTION_COUNT));
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.setInt(sc, SHUNT_SECTION_COUNT, 4)));

        // Solved section count: applied, unset
        PropertyDispatcher.setInt(sc, SHUNT_SOLVED_SECTION_COUNT, 3);
        PropertyDispatcher.invoke(sc, ACT_SET_SECTION_COUNT_TO_SOLVED_SECTION_COUNT);
        assertEquals(3, linear.getSectionCount());
        PropertyDispatcher.setInt(sc, SHUNT_SECTION_COUNT, 1);
        assertEquals(0L, PropertyDispatcher.invoke(sc, ACT_APPLY_SOLVED_VALUES));
        assertEquals(3, linear.getSectionCount());
        PropertyDispatcher.invoke(sc, ACT_UNSET_SOLVED_SECTION_COUNT);
        assertEquals(PropertyDispatcher.NO_INT, PropertyDispatcher.getOptInt(sc, SHUNT_SOLVED_SECTION_COUNT));
        PropertyDispatcher.invoke(sc, ACT_APPLY_SOLVED_VALUES); // none: no change
        assertEquals(3, linear.getSectionCount());

        // Unset (a network that accepts it): no value, getInt and getB fail
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.invoke(sc, ACT_UNSET_SECTION_COUNT)));
        network.setMinimumAcceptableValidationLevel(ValidationLevel.EQUIPMENT);
        PropertyDispatcher.invoke(sc, ACT_UNSET_SECTION_COUNT);
        assertEquals(PropertyDispatcher.NO_INT, PropertyDispatcher.getOptInt(sc, SHUNT_SECTION_COUNT));
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.getInt(sc, SHUNT_SECTION_COUNT)));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getDouble(sc, SHUNT_B)));
        assertEquals(24.0, PropertyDispatcher.evaluate(sc, SHUNT_B, new double[] {2}));
    }

    @Test void unsetSolvedValues() {
        linear.setSolvedSectionCount(1);
        linear.getTerminal().setP(0).setQ(-20);
        PropertyDispatcher.invoke(reg(linear), ACT_UNSET_SOLVED_VALUES);
        assertTrue(linear.findSolvedSectionCount().isEmpty());
        assertTrue(Double.isNaN(linear.getTerminal().getQ()));

        nonLinear.getTerminal().setQ(-5);
        PropertyDispatcher.invoke(reg(nonLinear.getTerminal()), ACT_UNSET_SOLVED_VALUES);
        assertTrue(Double.isNaN(nonLinear.getTerminal().getQ()));
        Bus bus = network.getBusBreakerView().getBus("B");
        bus.setV(226);
        PropertyDispatcher.invoke(reg(bus), ACT_UNSET_SOLVED_VALUES);
        assertTrue(Double.isNaN(bus.getV()));

        long model = reg(linear.getModel());
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.invoke(model, ACT_UNSET_SOLVED_VALUES)));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.invoke(model, ACT_APPLY_SOLVED_VALUES)));
    }

    // Generators, loads... apply their solved values too
    @Test void applySolvedValuesOfOtherObjects() {
        VoltageLevel vl = network.getVoltageLevel("VL");
        Load load = vl.newLoad().setId("L").setBus("B").setP0(10).setQ0(1).add();
        load.getTerminal().setP(12).setQ(3);
        PropertyDispatcher.invoke(reg(load), ACT_APPLY_SOLVED_VALUES);
        assertEquals(12, load.getP0());
        assertEquals(3, load.getQ0());
    }
}
