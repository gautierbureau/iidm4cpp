package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.util.Arrays;
import java.util.List;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Areas and voltage angle limits through {@link PropertyDispatcher}: an area's voltage levels and
 * boundaries, interchanges, creation through the adders, removal.
 */
class AreaDispatchTest {

    private Network network;
    private VoltageLevel vl1;
    private VoltageLevel vl2;
    private VoltageLevel vl3;
    private Line line;
    private BoundaryLine boundaryLine;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("areas", "test");
        Substation s = network.newSubstation().setId("S").add();
        vl1 = newVoltageLevel(s, "VL1");
        vl2 = newVoltageLevel(s, "VL2");
        vl3 = newVoltageLevel(s, "VL3");
        line = network.newLine().setId("L23").setVoltageLevel1("VL2").setBus1("B_VL2").setVoltageLevel2("VL3").setBus2("B_VL3")
            .setR(1).setX(10).setG1(0).setB1(0).setG2(0).setB2(0).add();
        line.getTerminal1().setP(99.5).setQ(9);
        boundaryLine = vl2.newBoundaryLine().setId("BL").setBus("B_VL2").setP0(50).setQ0(5)
            .setR(1).setX(10).setG(0).setB(0).add();
    }

    private static VoltageLevel newVoltageLevel(Substation s, String id) {
        VoltageLevel vl = s.newVoltageLevel().setId(id).setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl.getBusBreakerView().newBus().setId("B_" + id).add().setV(400).setAngle(0);
        return vl;
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

    private static List<String> ids(long[] handles) {
        return Arrays.stream(handles).mapToObj(h -> PropertyDispatcher.getString(h, ID)).toList();
    }

    /** CA through the adder: VL2 then VL1 (listed sorted), a line terminal and a boundary. */
    private long newArea() {
        long adder = PropertyDispatcher.invoke(reg(network), AREA);
        PropertyDispatcher.setString(adder, ID, "CA");
        PropertyDispatcher.setString(adder, AREA_TYPE, "ControlArea");
        PropertyDispatcher.setDouble(adder, AREA_INTERCHANGE_TARGET, -100);
        PropertyDispatcher.setRelated(adder, ADD_VOLTAGE_LEVEL, reg(vl2));
        PropertyDispatcher.setRelated(adder, ADD_VOLTAGE_LEVEL, reg(vl1));
        long terminalBoundary = PropertyDispatcher.invoke(adder, AREA_BOUNDARY);
        PropertyDispatcher.setRelated(terminalBoundary, REL_TERMINAL, reg(line.getTerminal1()));
        PropertyDispatcher.setBool(terminalBoundary, AREA_BOUNDARY_AC, 1);
        assertEquals(0L, PropertyDispatcher.invoke(terminalBoundary, ACT_ADD));
        long boundary = PropertyDispatcher.invoke(adder, AREA_BOUNDARY);
        PropertyDispatcher.setRelated(boundary, BOUNDARY, reg(boundaryLine.getBoundary()));
        PropertyDispatcher.setBool(boundary, AREA_BOUNDARY_AC, 0);
        PropertyDispatcher.invoke(boundary, ACT_ADD);
        return PropertyDispatcher.invoke(adder, ACT_ADD);
    }

    @Test void areas() {
        long area = newArea();
        assertEquals(1, PropertyDispatcher.getBool(area, AREA));
        assertEquals("ControlArea", PropertyDispatcher.getString(area, AREA_TYPE));
        assertEquals(-100, PropertyDispatcher.getDouble(area, AREA_INTERCHANGE_TARGET));
        assertEquals(List.of("VL1", "VL2"), ids(PropertyDispatcher.getChildren(area, VOLTAGE_LEVEL)));
        assertArrayEquals(new String[] {"ControlArea"}, PropertyDispatcher.getStringList(reg(network), AREA_TYPE));
        assertSame(obj(area), obj(PropertyDispatcher.getRelatedByString(reg(vl1), AREA, "ControlArea")));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(reg(vl1), AREA, "BiddingZone"));
        assertSame(obj(area), obj(PropertyDispatcher.findById(reg(network), AREA, "CA")));
        assertEquals(1, PropertyDispatcher.getChildren(reg(vl2), AREA).length);

        // Its boundaries: a terminal (AC), a boundary (DC)
        long[] boundaries = PropertyDispatcher.getChildren(area, AREA_BOUNDARY);
        assertEquals(2, boundaries.length);
        long first = boundaries[0];
        assertEquals(1, PropertyDispatcher.getBool(first, AREA_BOUNDARY));
        assertEquals(1, PropertyDispatcher.getBool(first, AREA_BOUNDARY_AC));
        assertSame(line.getTerminal1(), obj(PropertyDispatcher.getRelated(first, REL_TERMINAL)));
        assertEquals(0L, PropertyDispatcher.getRelated(first, BOUNDARY));
        assertSame(obj(area), obj(PropertyDispatcher.getRelated(first, AREA)));
        assertEquals(99.5, PropertyDispatcher.getDouble(first, AREA_BOUNDARY_P));
        assertSame(boundaryLine.getBoundary(), obj(PropertyDispatcher.getRelated(boundaries[1], BOUNDARY)));
        assertEquals(99.5, PropertyDispatcher.getDouble(area, AREA_AC_INTERCHANGE));
        PropertyDispatcher.setKeyedString(first, PROPERTY, "kind", "line");
        assertEquals("line", PropertyDispatcher.getKeyedString(first, PROPERTY, "kind"));

        // Changes
        PropertyDispatcher.setDouble(area, AREA_INTERCHANGE_TARGET, Double.NaN);
        assertTrue(Double.isNaN(PropertyDispatcher.getDouble(area, AREA_INTERCHANGE_TARGET)));
        PropertyDispatcher.setRelated(area, REMOVE_VOLTAGE_LEVEL, reg(vl1));
        PropertyDispatcher.setRelated(reg(vl3), ADD_AREA, area);
        assertEquals(List.of("VL2", "VL3"), ids(PropertyDispatcher.getChildren(area, VOLTAGE_LEVEL)));
        PropertyDispatcher.setRelated(reg(vl3), REMOVE_AREA, area);
        PropertyDispatcher.setRelated(area, ADD_VOLTAGE_LEVEL, reg(vl1));
        assertEquals(List.of("VL1", "VL2"), ids(PropertyDispatcher.getChildren(area, VOLTAGE_LEVEL)));
        PropertyDispatcher.setRelated(area, REMOVE_AREA_BOUNDARY, reg(line.getTerminal1()));
        PropertyDispatcher.setRelated(area, REMOVE_AREA_BOUNDARY, reg(boundaryLine.getBoundary()));
        assertEquals(0, PropertyDispatcher.getChildren(area, AREA_BOUNDARY).length);

        // A boundary of the area itself: its adder returns the area
        long adder = PropertyDispatcher.invoke(area, AREA_BOUNDARY);
        PropertyDispatcher.setRelated(adder, REL_TERMINAL, reg(line.getTerminal2()));
        PropertyDispatcher.setBool(adder, AREA_BOUNDARY_AC, 1);
        assertSame(obj(area), obj(PropertyDispatcher.invoke(adder, ACT_ADD)));
        assertEquals(1, PropertyDispatcher.getChildren(area, AREA_BOUNDARY).length);

        PropertyDispatcher.invoke(area, ACT_REMOVE);
        assertNull(network.getArea("CA"));
    }

    @Test void voltageAngleLimits() {
        long adder = PropertyDispatcher.invoke(reg(network), VOLTAGE_ANGLE_LIMIT);
        PropertyDispatcher.setString(adder, VAL_ID, "VAL");
        PropertyDispatcher.setRelated(adder, REL_TERMINAL_FROM, reg(line.getTerminal1()));
        PropertyDispatcher.setRelated(adder, REL_TERMINAL_TO, reg(line.getTerminal2()));
        PropertyDispatcher.setDouble(adder, VAL_HIGH_LIMIT, 15);
        PropertyDispatcher.setKeyedString(adder, PROPERTY, "source", "operator");
        long limit = PropertyDispatcher.invoke(adder, ACT_ADD);

        assertEquals(1, PropertyDispatcher.getBool(limit, VOLTAGE_ANGLE_LIMIT));
        assertEquals("VAL", PropertyDispatcher.getString(limit, VAL_ID));
        assertEquals(LimitType.VOLTAGE_ANGLE.ordinal(), PropertyDispatcher.getInt(limit, LL_LIMIT_TYPE));
        assertEquals(15, PropertyDispatcher.getDouble(limit, VAL_HIGH_LIMIT));
        assertTrue(Double.isNaN(PropertyDispatcher.getDouble(limit, VAL_LOW_LIMIT)));
        assertSame(line.getTerminal1(), obj(PropertyDispatcher.getRelated(limit, REL_TERMINAL_FROM)));
        assertSame(line.getTerminal2(), obj(PropertyDispatcher.getRelated(limit, REL_TERMINAL_TO)));
        assertEquals("operator", PropertyDispatcher.getKeyedString(limit, PROPERTY, "source"));
        assertEquals(1, PropertyDispatcher.getChildren(reg(network), VOLTAGE_ANGLE_LIMIT).length);
        assertSame(obj(limit), obj(PropertyDispatcher.findById(reg(network), VOLTAGE_ANGLE_LIMIT, "VAL")));

        PropertyDispatcher.invoke(limit, ACT_REMOVE);
        assertNull(network.getVoltageAngleLimit("VAL"));
    }
}
