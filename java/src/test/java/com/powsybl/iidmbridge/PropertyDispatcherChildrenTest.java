package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Regression tests for PropertyDispatcher.getChildren().
 *
 * Previously REACTIVE_CURVE_POINT cast {@code obj} to {@link Generator}
 * unconditionally, so calling {@code vsc.getReactiveCapabilityCurve().getPoints()}
 * (which dispatches REACTIVE_CURVE_POINT with a VscConverterStation handle)
 * threw {@code ClassCastException: VscConverterStationImpl cannot be cast to Generator}.
 */
class PropertyDispatcherChildrenTest {

    private Network network;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("test", "test");
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    @Test void reactiveCurvePoints_onVscConverterStation() {
        Substation s = network.newSubstation().setId("S").setCountry(Country.FR).add();
        VoltageLevel vl = s.newVoltageLevel().setId("VL").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b = vl.getBusBreakerView().newBus().setId("B").add();
        VscConverterStation vsc = vl.newVscConverterStation().setId("VSC")
                .setBus(b.getId()).setConnectableBus(b.getId())
                .setLossFactor(0.01f)
                .setVoltageRegulatorOn(true)
                .setVoltageSetpoint(400)
                .setReactivePowerSetpoint(0)
                .add();
        vsc.newReactiveCapabilityCurve()
                .beginPoint().setP(-100).setMinQ(-50).setMaxQ(50).endPoint()
                .beginPoint().setP(100).setMinQ(-60).setMaxQ(60).endPoint()
                .add();

        long h = NetworkRegistry.register(vsc);
        long[] points = assertDoesNotThrow(
                () -> PropertyDispatcher.getChildren(h, REACTIVE_CURVE_POINT),
                "REACTIVE_CURVE_POINT should support VscConverterStation, not throw ClassCastException");
        assertEquals(2, points.length);
    }

    @Test void reactiveCurvePoints_onGenerator() {
        Substation s = network.newSubstation().setId("SG").setCountry(Country.FR).add();
        VoltageLevel vl = s.newVoltageLevel().setId("VLG").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b = vl.getBusBreakerView().newBus().setId("BG").add();
        Generator gen = vl.newGenerator().setId("G")
                .setBus(b.getId()).setConnectableBus(b.getId())
                .setTargetP(100).setTargetQ(0).setTargetV(400)
                .setMinP(0).setMaxP(200).setVoltageRegulatorOn(true)
                .add();
        gen.newReactiveCapabilityCurve()
                .beginPoint().setP(0).setMinQ(-10).setMaxQ(10).endPoint()
                .beginPoint().setP(50).setMinQ(-20).setMaxQ(20).endPoint()
                .beginPoint().setP(100).setMinQ(-30).setMaxQ(30).endPoint()
                .add();

        long h = NetworkRegistry.register(gen);
        long[] points = PropertyDispatcher.getChildren(h, REACTIVE_CURVE_POINT);
        assertEquals(3, points.length);
    }

    @Test void reactiveCurvePoints_onBattery() {
        Substation s = network.newSubstation().setId("SB").setCountry(Country.FR).add();
        VoltageLevel vl = s.newVoltageLevel().setId("VLB").setNominalV(400)
                .setTopologyKind(TopologyKind.BUS_BREAKER).add();
        Bus b = vl.getBusBreakerView().newBus().setId("BB").add();
        Battery bat = vl.newBattery().setId("BAT")
                .setBus(b.getId()).setConnectableBus(b.getId())
                .setTargetP(10).setTargetQ(0).setMinP(-50).setMaxP(50)
                .add();
        bat.newReactiveCapabilityCurve()
                .beginPoint().setP(-50).setMinQ(-25).setMaxQ(25).endPoint()
                .beginPoint().setP(50).setMinQ(-25).setMaxQ(25).endPoint()
                .add();

        long h = NetworkRegistry.register(bat);
        long[] points = PropertyDispatcher.getChildren(h, REACTIVE_CURVE_POINT);
        assertEquals(2, points.length);
    }
}
