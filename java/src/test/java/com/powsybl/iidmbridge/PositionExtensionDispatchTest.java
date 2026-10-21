package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidm.network.extensions.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.nio.file.Path;
import java.util.Arrays;
import java.util.List;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * The position extensions through {@link PropertyDispatcher}, on test/data/positions_test.xiidm:
 * coordinates of substations and lines, busbar section positions, the feeders of connectable
 * positions; created through their adders, with nested coordinates and feeders.
 */
class PositionExtensionDispatchTest {

    private Network network;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = com.powsybl.iidm.serde.NetworkSerDe.read(Path.of("../test/data/positions_test.xiidm"));
    }

    @AfterEach
    void tearDown() {
        NetworkRegistry.releaseAll();
    }

    private static long reg(Object obj) {
        return NetworkRegistry.register(obj);
    }

    private static List<Double> latitudes(long[] coordinates) {
        return Arrays.stream(coordinates).mapToObj(c -> PropertyDispatcher.getDouble(c, COORD_LATITUDE)).toList();
    }

    @Test void coordinates() {
        long sp = PropertyDispatcher.getRelated(reg(network.getSubstation("S1")), SUBSTATION_POSITION);
        long coordinate = PropertyDispatcher.getRelated(sp, COORDINATE);
        assertEquals(1, PropertyDispatcher.getBool(coordinate, COORDINATE));
        assertEquals(48.85, PropertyDispatcher.getDouble(coordinate, COORD_LATITUDE));
        assertEquals(2.35, PropertyDispatcher.getDouble(coordinate, COORD_LONGITUDE));
        assertEquals(0L, PropertyDispatcher.getRelated(reg(network.getSubstation("S2")), SUBSTATION_POSITION));

        long lp = PropertyDispatcher.getRelated(reg(network.getLine("L12")), LINE_POSITION);
        assertEquals(List.of(48.85, 49.5, 50.85), latitudes(PropertyDispatcher.getChildren(lp, COORDINATE)));
        long blp = PropertyDispatcher.getRelated(reg(network.getBoundaryLine("BL")), LINE_POSITION);
        assertEquals(2, PropertyDispatcher.getChildren(blp, COORDINATE).length);

        // New ones: coordinates in order; a substation's, the last one
        Substation s2 = network.getSubstation("S2");
        long adder = PropertyDispatcher.invoke(reg(s2), SUBSTATION_POSITION);
        long c = PropertyDispatcher.invoke(adder, COORDINATE);
        PropertyDispatcher.setDouble(c, COORD_LATITUDE, 50.85);
        PropertyDispatcher.setDouble(c, COORD_LONGITUDE, 4.35);
        assertEquals(0L, PropertyDispatcher.invoke(c, ACT_ADD)); // nested
        PropertyDispatcher.invoke(adder, ACT_ADD);
        assertEquals(new Coordinate(50.85, 4.35), s2.getExtension(SubstationPosition.class).getCoordinate());

        BoundaryLine bl = network.getBoundaryLine("BL");
        bl.removeExtension(LinePosition.class);
        long lineAdder = PropertyDispatcher.invoke(reg(bl), LINE_POSITION);
        for (double latitude : new double[] {1, 2, 3}) {
            long nc = PropertyDispatcher.invoke(lineAdder, COORDINATE);
            PropertyDispatcher.setDouble(nc, COORD_LATITUDE, latitude);
            PropertyDispatcher.setDouble(nc, COORD_LONGITUDE, 0);
            PropertyDispatcher.invoke(nc, ACT_ADD);
        }
        long added = PropertyDispatcher.invoke(lineAdder, ACT_ADD);
        assertEquals(List.of(1.0, 2.0, 3.0), latitudes(PropertyDispatcher.getChildren(added, COORDINATE)));
    }

    @Test void busbarSectionPositions() {
        long bsp = PropertyDispatcher.getRelated(reg(network.getBusbarSection("BBS")), BUSBAR_SECTION_POSITION);
        assertEquals(1, PropertyDispatcher.getInt(bsp, EXT_BSP_BUSBAR_INDEX));
        assertEquals(2, PropertyDispatcher.getInt(bsp, EXT_BSP_SECTION_INDEX));
        PropertyDispatcher.setInt(bsp, EXT_BSP_SECTION_INDEX, 3);
        assertEquals(3, network.getBusbarSection("BBS").getExtension(BusbarSectionPosition.class).getSectionIndex());
    }

    @Test void feeders() {
        long load = reg(network.getLoad("LD"));
        long cp = PropertyDispatcher.getRelated(load, CONNECTABLE_POSITION);
        long feeder = PropertyDispatcher.getRelatedByIndex(cp, FEEDER, 0);
        assertEquals(1, PropertyDispatcher.getBool(feeder, FEEDER));
        assertEquals("LD feeder", PropertyDispatcher.getOptString(feeder, EXT_FEEDER_NAME));
        assertEquals(10, PropertyDispatcher.getOptInt(feeder, EXT_FEEDER_ORDER));
        assertEquals(ConnectablePosition.Direction.BOTTOM.ordinal(), PropertyDispatcher.getInt(feeder, EXT_FEEDER_DIRECTION));
        assertEquals(0L, PropertyDispatcher.getRelatedByIndex(cp, FEEDER, 1));

        long lineCp = PropertyDispatcher.getRelated(reg(network.getLine("L12")), CONNECTABLE_POSITION);
        long feeder1 = PropertyDispatcher.getRelatedByIndex(lineCp, FEEDER, 1);
        assertNull(PropertyDispatcher.getOptString(feeder1, EXT_FEEDER_NAME));
        assertEquals(PropertyDispatcher.NO_INT, PropertyDispatcher.getOptInt(feeder1, EXT_FEEDER_ORDER));
        assertEquals(3, PropertyDispatcher.getOptInt(PropertyDispatcher.getRelatedByIndex(lineCp, FEEDER, 2), EXT_FEEDER_ORDER));
        assertEquals(0L, PropertyDispatcher.getRelatedByIndex(lineCp, FEEDER, 0));

        // Changed: name, order, direction; the order removed
        PropertyDispatcher.setString(feeder1, EXT_FEEDER_NAME, "L12 side 1");
        PropertyDispatcher.setInt(feeder1, EXT_FEEDER_ORDER, 7);
        PropertyDispatcher.setInt(feeder1, EXT_FEEDER_DIRECTION, ConnectablePosition.Direction.UNDEFINED.ordinal());
        ConnectablePosition<?> position = (ConnectablePosition<?>) network.getLine("L12").getExtensionByName("position");
        assertEquals("L12 side 1", position.getFeeder1().getName().orElseThrow());
        assertEquals(7, position.getFeeder1().getOrder().orElseThrow());
        assertEquals(ConnectablePosition.Direction.UNDEFINED, position.getFeeder1().getDirection());
        PropertyDispatcher.invoke(feeder, ACT_REMOVE_ORDER);
        assertEquals(PropertyDispatcher.NO_INT, PropertyDispatcher.getOptInt(feeder, EXT_FEEDER_ORDER));
    }

    @Test void feederAdders() {
        Generator g = network.getGenerator("G");
        long adder = PropertyDispatcher.invoke(reg(g), CONNECTABLE_POSITION);
        long feeder = PropertyDispatcher.invoke(adder, FEEDER);
        PropertyDispatcher.setString(feeder, EXT_FEEDER_NAME, "G feeder");
        PropertyDispatcher.setInt(feeder, EXT_FEEDER_ORDER, 20);
        PropertyDispatcher.setInt(feeder, EXT_FEEDER_DIRECTION, ConnectablePosition.Direction.TOP.ordinal());
        assertEquals(0L, PropertyDispatcher.invoke(feeder, ACT_ADD)); // nested
        long cp = PropertyDispatcher.invoke(adder, ACT_ADD);
        long added = PropertyDispatcher.getRelatedByIndex(cp, FEEDER, 0);
        assertEquals("G feeder", PropertyDispatcher.getOptString(added, EXT_FEEDER_NAME));
        assertEquals(20, PropertyDispatcher.getOptInt(added, EXT_FEEDER_ORDER));

        // The feeder of an injection and a side's: rejected
        Line line = network.getLine("L12");
        line.removeExtension(ConnectablePosition.class);
        long lineAdder = PropertyDispatcher.invoke(reg(line), CONNECTABLE_POSITION);
        PropertyDispatcher.invoke(PropertyDispatcher.invoke(lineAdder, FEEDER), ACT_ADD);
        long side1 = PropertyDispatcher.invoke(lineAdder, FEEDER);
        PropertyDispatcher.setInt(side1, EXT_FEEDER_SLOT, 1);
        PropertyDispatcher.invoke(side1, ACT_ADD);
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(lineAdder, ACT_ADD))));
    }
}
