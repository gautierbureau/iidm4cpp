package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Lines and HVDC lines through {@link PropertyDispatcher}: their values, the converter stations of
 * an HVDC line by side, and their connection.
 */
class LineAndHvdcDispatchTest {

    private Network network;
    private Line line;
    private HvdcLine hvdc;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("lines", "test");
        for (String id : new String[] {"VL1", "VL2"}) {
            VoltageLevel vl = network.newVoltageLevel().setId(id).setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
            vl.getBusBreakerView().newBus().setId(id + "_B").add();
            vl.newLccConverterStation().setId(id + "_CS").setBus(id + "_B").setLossFactor(0.01f).setPowerFactor(0.9f).add();
        }
        line = network.newLine().setId("L")
            .setVoltageLevel1("VL1").setBus1("VL1_B").setVoltageLevel2("VL2").setBus2("VL2_B")
            .setR(1).setX(10).setG1(0).setB1(0).setG2(0).setB2(0).add();
        hvdc = network.newHvdcLine().setId("HVDC").setR(1).setNominalV(320).setMaxP(500).setActivePowerSetpoint(100)
            .setConvertersMode(HvdcLine.ConvertersMode.SIDE_1_RECTIFIER_SIDE_2_INVERTER)
            .setConverterStationId1("VL1_CS").setConverterStationId2("VL2_CS").add();
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

    @Test void lineValues() {
        long l = reg(line);
        PropertyDispatcher.setDouble(l, LINE_R, 2);
        PropertyDispatcher.setDouble(l, LINE_X, 20);
        PropertyDispatcher.setDouble(l, LINE_G1, 1e-6);
        PropertyDispatcher.setDouble(l, LINE_B1, 1e-5);
        PropertyDispatcher.setDouble(l, LINE_G2, 2e-6);
        PropertyDispatcher.setDouble(l, LINE_B2, 2e-5);
        assertEquals(2, line.getR());
        assertEquals(20, line.getX());
        assertEquals(1e-6, line.getG1());
        assertEquals(2e-5, line.getB2());
        assertThrows(ValidationException.class, () -> PropertyDispatcher.setDouble(l, LINE_R, Double.NaN));
    }

    @Test void hvdcLineValues() {
        long h = reg(hvdc);
        PropertyDispatcher.setDouble(h, HVDC_R, 2);
        PropertyDispatcher.setDouble(h, HVDC_NOMINAL_V, 400);
        PropertyDispatcher.setDouble(h, HVDC_MAX_P, 600);
        assertEquals(2, hvdc.getR());
        assertEquals(400, hvdc.getNominalV());
        assertEquals(600, hvdc.getMaxP());
        assertThrows(ValidationException.class, () -> PropertyDispatcher.setDouble(h, HVDC_MAX_P, -1));
    }

    @Test void converterStations() {
        long h = reg(hvdc);
        assertSame(hvdc.getConverterStation1(), obj(PropertyDispatcher.getRelatedByIndex(h, HVDC_CONVERTER_STATION, TwoSides.ONE.ordinal())));
        assertSame(hvdc.getConverterStation2(), obj(PropertyDispatcher.getRelatedByIndex(h, HVDC_CONVERTER_STATION, TwoSides.TWO.ordinal())));
        assertThrows(IllegalArgumentException.class, () -> PropertyDispatcher.getRelatedByIndex(h, HVDC_CONVERTER_STATION, 2));

        // Both disconnected, then connected again: the line when this changed them
        assertSame(hvdc, obj(PropertyDispatcher.invoke(h, ACT_DISCONNECT)));
        assertFalse(hvdc.getConverterStation1().getTerminal().isConnected());
        assertFalse(hvdc.getConverterStation2().getTerminal().isConnected());
        assertEquals(0L, PropertyDispatcher.invoke(h, ACT_DISCONNECT));
        assertNotEquals(0L, PropertyDispatcher.invoke(h, ACT_CONNECT));
        assertTrue(hvdc.getConverterStation2().getTerminal().isConnected());
    }
}
