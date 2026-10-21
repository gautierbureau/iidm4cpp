package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;
import org.junit.jupiter.api.function.Executable;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Reactive limits as objects of their own (min-max limits, capability curve and its points,
 * capability shape and its planes), and evaluate for the methods that take arguments.
 */
class ReactiveLimitsDispatchTest {

    private Generator gen;
    private Battery bat;
    private VscConverterStation vsc;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        Network network = Network.create("test", "test");
        VoltageLevel vl = network.newSubstation().setId("S").add()
            .newVoltageLevel().setId("VL").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl.getBusBreakerView().newBus().setId("B").add();
        gen = vl.newGenerator().setId("G").setBus("B").setMinP(0).setMaxP(200).setTargetP(100)
            .setLocalTargetQ(0).add();
        gen.newReactiveCapabilityCurve()
            .beginPoint().setP(0).setMinQ(-10).setMaxQ(10).endPoint()
            .beginPoint().setP(100).setMinQ(-30).setMaxQ(30).endPoint()
            .add();
        bat = vl.newBattery().setId("BAT").setBus("B").setMinP(-50).setMaxP(50).setTargetP(0).setLocalTargetQ(0).add();
        bat.newMinMaxReactiveLimits().setMinQ(-20).setMaxQ(25).add();
        vsc = vl.newVscConverterStation().setId("VSC").setBus("B").setLossFactor(0.01f).setLocalTargetQ(0).add();
        vsc.newReactiveCapabilityShape()
            .addPlane(0, 0, 1, true, -100)   // Q >= -100
            .addPlane(0, 0, 1, false, 100)   // Q <= 100
            .addPlane(0, 0.5, 1, false, 150) // Q + 0.5 P <= 150
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

    private static int kindOfFailure(Executable call) {
        return BridgeErrors.kindOf(assertThrows(Throwable.class, call));
    }

    @Test void limitsOfEveryHolder() {
        assertSame(gen.getReactiveLimits(), obj(PropertyDispatcher.getRelated(reg(gen), REACTIVE_LIMITS)));
        assertSame(bat.getReactiveLimits(), obj(PropertyDispatcher.getRelated(reg(bat), REACTIVE_LIMITS)));
        assertSame(vsc.getReactiveLimits(), obj(PropertyDispatcher.getRelated(reg(vsc), REACTIVE_LIMITS)));
        assertEquals(ReactiveLimitsKind.CURVE.ordinal(), PropertyDispatcher.getInt(gen.getReactiveLimits(), RL_KIND));
        assertEquals(ReactiveLimitsKind.MIN_MAX.ordinal(), PropertyDispatcher.getInt(bat.getReactiveLimits(), RL_KIND));
        assertEquals(ReactiveLimitsKind.SHAPE.ordinal(), PropertyDispatcher.getInt(vsc.getReactiveLimits(), RL_KIND));
    }

    @Test void limitsOfAKind() {
        long g = reg(gen);
        assertNotEquals(0L, PropertyDispatcher.getRelated(g, REACTIVE_CAPABILITY_CURVE));
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.getRelated(g, MIN_MAX_REACTIVE_LIMITS)));
        Object limits = bat.getReactiveLimits();
        assertTrue(PropertyDispatcher.getBool(limits, REACTIVE_LIMITS));
        assertTrue(PropertyDispatcher.getBool(limits, MIN_MAX_REACTIVE_LIMITS));
        assertFalse(PropertyDispatcher.getBool(limits, REACTIVE_CAPABILITY_CURVE));
        assertFalse(PropertyDispatcher.getBool(gen, REACTIVE_LIMITS)); // a generator is not reactive limits
    }

    @Test void minMax() {
        long limits = PropertyDispatcher.getRelated(reg(bat), MIN_MAX_REACTIVE_LIMITS);
        assertEquals(-20, PropertyDispatcher.getDouble(limits, RL_MIN_Q));
        assertEquals(25, PropertyDispatcher.getDouble(limits, RL_MAX_Q));
        assertEquals(-20, PropertyDispatcher.evaluate(limits, RL_MIN_Q, new double[] {1000})); // whatever p
        // A curve has no limits without p
        long curve = PropertyDispatcher.getRelated(reg(gen), REACTIVE_LIMITS);
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.getDouble(curve, RL_MIN_Q)));
    }

    @Test void curve() {
        ReactiveCapabilityCurve c = gen.getReactiveLimits(ReactiveCapabilityCurve.class);
        long curve = reg(c);
        assertEquals(2, PropertyDispatcher.getInt(curve, CURVE_POINT_COUNT));
        assertEquals(0, PropertyDispatcher.getDouble(curve, CURVE_MIN_P));
        assertEquals(100, PropertyDispatcher.getDouble(curve, CURVE_MAX_P));
        long[] points = PropertyDispatcher.getChildren(curve, REACTIVE_CURVE_POINT);
        assertEquals(2, points.length);
        assertEquals(100, PropertyDispatcher.getDouble(points[1], POINT_P));
        assertEquals(-30, PropertyDispatcher.getDouble(points[1], POINT_MIN_Q));
        assertEquals(30, PropertyDispatcher.getDouble(points[1], POINT_MAX_Q));

        // Interpolated, and beyond the curve with or without the slope
        assertEquals(-20, PropertyDispatcher.evaluate(curve, RL_MIN_Q, new double[] {50}), 1e-9);
        assertEquals(c.getMaxQ(150, false), PropertyDispatcher.evaluate(curve, RL_MAX_Q, new double[] {150, 0}));
        assertEquals(c.getMaxQ(150, true), PropertyDispatcher.evaluate(curve, RL_MAX_Q, new double[] {150, 1}));
        assertNotEquals(PropertyDispatcher.evaluate(curve, RL_MAX_Q, new double[] {150, 0}),
                        PropertyDispatcher.evaluate(curve, RL_MAX_Q, new double[] {150, 1}));
    }

    @Test void shape() {
        ReactiveCapabilityShape s = vsc.getReactiveLimits(ReactiveCapabilityShape.class);
        long shape = reg(s);
        long[] planes = PropertyDispatcher.getChildren(shape, REACTIVE_CAPABILITY_SHAPE_PLANE);
        assertEquals(3, planes.length);
        assertEquals(0.5, PropertyDispatcher.getDouble(planes[2], PLANE_BETA));
        assertEquals(150, PropertyDispatcher.getDouble(planes[2], PLANE_GAMMA));
        assertEquals(1, PropertyDispatcher.getBool(planes[2], PLANE_LESS_OR_EQUAL));
        assertEquals(1, PropertyDispatcher.getBool(planes[0], PLANE_GREATER_OR_EQUAL));

        assertEquals(s.getMaxQ(200, 400), PropertyDispatcher.evaluate(shape, RL_MAX_Q, new double[] {200, 400}));
        assertEquals(s.getMinQ(0, 400), PropertyDispatcher.evaluate(shape, RL_MIN_Q, new double[] {0, 400}));
        assertEquals(s.isInside(0, 0, 400) ? 1 : 0, PropertyDispatcher.evaluate(shape, SHAPE_IS_INSIDE, new double[] {0, 0, 400}));
        assertEquals(s.isInside(200, 90, 400) ? 1 : 0, PropertyDispatcher.evaluate(shape, SHAPE_IS_INSIDE, new double[] {200, 90, 400}));
    }

    @Test void evaluateFailures() {
        long limits = PropertyDispatcher.getRelated(reg(bat), REACTIVE_LIMITS);
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.evaluate(limits, RL_MIN_Q, new double[] {})));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.evaluate(limits, SHAPE_IS_INSIDE, new double[] {0, 0, 0})));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.evaluate(limits, GEN_TARGET_P, new double[] {0})));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.evaluate(reg(gen), RL_MIN_Q, new double[] {0})));
    }

    @Test void newLimits() {
        // Min-max limits replace the generator's curve
        long g = reg(gen);
        long minMax = PropertyDispatcher.invoke(g, MIN_MAX_REACTIVE_LIMITS);
        PropertyDispatcher.setDouble(minMax, RL_MIN_Q, -40);
        PropertyDispatcher.setDouble(minMax, RL_MAX_Q, 45);
        long added = PropertyDispatcher.invoke(minMax, ACT_ADD);
        assertSame(gen.getReactiveLimits(), obj(added));
        assertEquals(45, gen.getReactiveLimits(MinMaxReactiveLimits.class).getMaxQ());

        // A curve, its points one by one, with properties
        long curve = PropertyDispatcher.invoke(reg(bat), REACTIVE_CAPABILITY_CURVE);
        PropertyDispatcher.setKeyedString(curve, PROPERTY, "source", "datasheet");
        for (double p : new double[] {-50, 50}) {
            long point = PropertyDispatcher.invoke(curve, REACTIVE_CURVE_POINT);
            PropertyDispatcher.setDouble(point, POINT_P, p);
            PropertyDispatcher.setDouble(point, POINT_MIN_Q, -10);
            PropertyDispatcher.setDouble(point, POINT_MAX_Q, p < 0 ? 20 : 60);
            PropertyDispatcher.setKeyedString(point, PROPERTY, "p", Double.toString(p));
            assertEquals(0L, PropertyDispatcher.invoke(point, ACT_ADD)); // nested
        }
        PropertyDispatcher.invoke(curve, ACT_ADD);
        ReactiveCapabilityCurve c = bat.getReactiveLimits(ReactiveCapabilityCurve.class);
        assertEquals(60, c.getMaxQ(50));
        assertEquals("datasheet", c.getProperty("source"));
        assertEquals("-50.0", c.getPoints().iterator().next().getProperty("p"));
        // One point only: rejected
        long onePoint = PropertyDispatcher.invoke(reg(bat), REACTIVE_CAPABILITY_CURVE);
        long point = PropertyDispatcher.invoke(onePoint, REACTIVE_CURVE_POINT);
        PropertyDispatcher.setDouble(point, POINT_P, 0);
        PropertyDispatcher.setDouble(point, POINT_MIN_Q, -1);
        PropertyDispatcher.setDouble(point, POINT_MAX_Q, 1);
        PropertyDispatcher.invoke(point, ACT_ADD);
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.invoke(onePoint, ACT_ADD)));

        // A shape, its planes one by one
        long shape = PropertyDispatcher.invoke(reg(gen), REACTIVE_CAPABILITY_SHAPE);
        addPlane(shape, 0, 0, 1, true, -50);
        addPlane(shape, 0, 0, 1, false, 60);
        PropertyDispatcher.invoke(shape, ACT_ADD);
        ReactiveCapabilityShape created = gen.getReactiveLimits(ReactiveCapabilityShape.class);
        assertEquals(2, created.getPlanes().size());
        assertEquals(60, created.getMaxQ(0, 400), 1e-9);
        // Without a plane: rejected
        long empty = PropertyDispatcher.invoke(reg(gen), REACTIVE_CAPABILITY_SHAPE);
        assertEquals(BridgeErrors.VALIDATION, kindOfFailure(() -> PropertyDispatcher.invoke(empty, ACT_ADD)));
        // Not a reactive limits holder
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, kindOfFailure(() -> PropertyDispatcher.invoke(reg(gen.getNetwork()), MIN_MAX_REACTIVE_LIMITS)));
    }

    @Test void planeInequality() {
        ReactiveCapabilityShapePlane plane = vsc.getReactiveLimits(ReactiveCapabilityShape.class).getPlanes().iterator().next();
        long p = reg(plane);
        PropertyDispatcher.setDouble(p, PLANE_LESS_OR_EQUAL, 120);
        assertTrue(plane.isLessOrEqual());
        assertEquals(120, plane.getGamma());
        PropertyDispatcher.setDouble(p, PLANE_GREATER_OR_EQUAL, -120);
        assertTrue(plane.isGreaterOrEqual());
        assertEquals(-120, plane.getGamma());
    }

    private static void addPlane(long shapeAdder, double alpha, double beta, double delta, boolean greaterOrEqual, double gamma) {
        long plane = PropertyDispatcher.invoke(shapeAdder, REACTIVE_CAPABILITY_SHAPE_PLANE);
        PropertyDispatcher.setDouble(plane, PLANE_ALPHA, alpha);
        PropertyDispatcher.setDouble(plane, PLANE_BETA, beta);
        PropertyDispatcher.setDouble(plane, PLANE_DELTA, delta);
        PropertyDispatcher.setBool(plane, PLANE_GREATER_OR_EQUAL, greaterOrEqual ? 1 : 0);
        PropertyDispatcher.setDouble(plane, PLANE_GAMMA, gamma);
        assertEquals(0L, PropertyDispatcher.invoke(plane, ACT_ADD)); // nested
    }

    @Test void properties() {
        long limits = PropertyDispatcher.getRelated(reg(bat), REACTIVE_LIMITS);
        PropertyDispatcher.setKeyedString(limits, PROPERTY, "source", "datasheet");
        assertEquals("datasheet", bat.getReactiveLimits().getProperty("source"));
        long point = PropertyDispatcher.getChildren(reg(gen.getReactiveLimits()), REACTIVE_CURVE_POINT)[0];
        PropertyDispatcher.setKeyedString(point, PROPERTY, "k", "v");
        assertArrayEquals(new String[] {"k"}, PropertyDispatcher.getStringList(point, PROPERTY_NAMES));
    }
}
