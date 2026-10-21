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
 * The secondary voltage control and the line couplings of a network through {@link PropertyDispatcher},
 * on test/data/network_extensions_test.xiidm: their objects, their changes, their adders and the
 * queries of the line couplings.
 */
class NetworkExtensionDispatchTest {

    private Network network;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = com.powsybl.iidm.serde.NetworkSerDe.read(Path.of("../test/data/network_extensions_test.xiidm"));
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

    private static List<String> units(long zone) {
        return Arrays.stream(PropertyDispatcher.getChildren(zone, CONTROL_UNIT))
            .mapToObj(u -> PropertyDispatcher.getString(u, EXT_SVC_UNIT_ID) + "=" + (PropertyDispatcher.getBool(u, EXT_SVC_UNIT_PARTICIPATE) == 1))
            .toList();
    }

    @Test void secondaryVoltageControl() {
        long svc = PropertyDispatcher.getRelated(reg(network), SECONDARY_VOLTAGE_CONTROL);
        long[] zones = PropertyDispatcher.getChildren(svc, CONTROL_ZONE);
        assertEquals(2, zones.length);
        assertEquals(1, PropertyDispatcher.getBool(zones[0], CONTROL_ZONE));
        assertEquals("z1", PropertyDispatcher.getString(zones[0], EXT_SVC_ZONE_NAME));
        long pilotPoint = PropertyDispatcher.getRelated(zones[0], PILOT_POINT);
        // In order, not sorted
        assertArrayEquals(new String[] {"B2", "B1"}, PropertyDispatcher.getStringList(pilotPoint, EXT_SVC_PILOT_POINT_IDS));
        assertEquals(405.0, PropertyDispatcher.getDouble(pilotPoint, EXT_SVC_TARGET_V));
        assertEquals(List.of("G1=true", "G2=false"), units(zones[0]));

        // By name, by id; changes
        long z2 = PropertyDispatcher.getRelatedByString(svc, CONTROL_ZONE, "z2");
        assertSame(obj(zones[1]), obj(z2));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(svc, CONTROL_ZONE, "z3"));
        long g2 = PropertyDispatcher.getRelatedByString(zones[0], CONTROL_UNIT, "G2");
        PropertyDispatcher.setBool(g2, EXT_SVC_UNIT_PARTICIPATE, 1);
        assertEquals(List.of("G1=true", "G2=true"), units(zones[0]));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(zones[0], CONTROL_UNIT, "G3"));
        PropertyDispatcher.setDouble(pilotPoint, EXT_SVC_TARGET_V, 410);
        assertEquals(410.0, ((PilotPoint) obj(pilotPoint)).getTargetV());
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.setDouble(pilotPoint, EXT_SVC_TARGET_V, Double.NaN))));

        // A new one: a zone with its pilot point and control units
        network.removeExtension(SecondaryVoltageControl.class);
        long adder = PropertyDispatcher.invoke(reg(network), SECONDARY_VOLTAGE_CONTROL);
        long zone = PropertyDispatcher.invoke(adder, CONTROL_ZONE);
        PropertyDispatcher.setString(zone, EXT_SVC_ZONE_NAME, "z");
        long newPilotPoint = PropertyDispatcher.invoke(zone, PILOT_POINT);
        PropertyDispatcher.setString(newPilotPoint, EXT_SVC_PILOT_POINT_IDS, "B3");
        PropertyDispatcher.setString(newPilotPoint, EXT_SVC_PILOT_POINT_IDS, "B1");
        PropertyDispatcher.setDouble(newPilotPoint, EXT_SVC_TARGET_V, 400);
        assertEquals(0L, PropertyDispatcher.invoke(newPilotPoint, ACT_ADD)); // nested
        long unit = PropertyDispatcher.invoke(zone, CONTROL_UNIT);
        PropertyDispatcher.setString(unit, EXT_SVC_UNIT_ID, "G3");
        PropertyDispatcher.setBool(unit, EXT_SVC_UNIT_PARTICIPATE, 0);
        assertEquals(0L, PropertyDispatcher.invoke(unit, ACT_ADD));
        assertEquals(0L, PropertyDispatcher.invoke(zone, ACT_ADD));
        long added = PropertyDispatcher.invoke(adder, ACT_ADD);
        ControlZone created = ((SecondaryVoltageControl) obj(added)).getControlZone("z").orElseThrow();
        assertEquals(List.of("B3", "B1"), created.getPilotPoint().getBusbarSectionsOrBusesIds());
        assertFalse(created.getControlUnit("G3").orElseThrow().isParticipate());

        // powsybl's checks: a zone without a control unit, a pilot point without a bus
        network.removeExtension(SecondaryVoltageControl.class);
        long empty = PropertyDispatcher.invoke(PropertyDispatcher.invoke(reg(network), SECONDARY_VOLTAGE_CONTROL), CONTROL_ZONE);
        PropertyDispatcher.setString(empty, EXT_SVC_ZONE_NAME, "empty");
        long noBus = PropertyDispatcher.invoke(empty, PILOT_POINT);
        PropertyDispatcher.setDouble(noBus, EXT_SVC_TARGET_V, 400);
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(noBus, ACT_ADD))));
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(empty, ACT_ADD))));
    }

    @Test void lineCouplings() {
        long lc = PropertyDispatcher.getRelated(reg(network), LINE_COUPLINGS);
        long[] couplings = PropertyDispatcher.getChildren(lc, MUTUAL_COUPLING);
        assertEquals(2, couplings.length);
        assertEquals(1, PropertyDispatcher.getBool(couplings[1], MUTUAL_COUPLING));
        assertSame(network.getLine("L23"), obj(PropertyDispatcher.getRelated(couplings[1], EXT_MC_LINE1)));
        assertSame(network.getLine("L12"), obj(PropertyDispatcher.getRelated(couplings[1], EXT_MC_LINE2)));
        assertEquals(0.2, PropertyDispatcher.getDouble(couplings[1], EXT_MC_R));
        assertEquals(0.7, PropertyDispatcher.getDouble(couplings[1], EXT_MC_X));
        long segment = PropertyDispatcher.getRelated(couplings[1], EXT_MC_LINE1_SEGMENT);
        assertEquals(0.1, PropertyDispatcher.getDouble(segment, EXT_MC_SEGMENT_START));
        assertEquals(0.6, PropertyDispatcher.getDouble(segment, EXT_MC_SEGMENT_END));
        assertEquals(LineSegment.FULL_LINE, obj(PropertyDispatcher.getRelated(couplings[0], EXT_MC_LINE2_SEGMENT)));

        // Changes: values, a segment at once (an invalid one is rejected)
        PropertyDispatcher.setDouble(couplings[0], EXT_MC_X, 0.55);
        assertEquals(0.55, ((MutualCoupling) obj(couplings[0])).getX());
        long change = PropertyDispatcher.invoke(couplings[0], EXT_MC_LINE2_SEGMENT);
        PropertyDispatcher.setDouble(change, EXT_MC_SEGMENT_START, 0.5);
        PropertyDispatcher.setDouble(change, EXT_MC_SEGMENT_END, 0.75);
        assertEquals(0L, PropertyDispatcher.invoke(change, ACT_ADD));
        assertEquals(new LineSegment(0.5, 0.75), ((MutualCoupling) obj(couplings[0])).getLine2Segment());
        long invalid = PropertyDispatcher.invoke(couplings[0], EXT_MC_LINE1_SEGMENT);
        PropertyDispatcher.setDouble(invalid, EXT_MC_SEGMENT_START, 0.8);
        PropertyDispatcher.setDouble(invalid, EXT_MC_SEGMENT_END, 0.2);
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(invalid, ACT_ADD))));
        // Only a coupling or a new one has segments
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(lc, EXT_MC_LINE1_SEGMENT))));

        // Queries: found in either order; removed by coupling or by lines
        long l12 = reg(network.getLine("L12"));
        long l13 = reg(network.getLine("L13"));
        long l23 = reg(network.getLine("L23"));
        assertSame(obj(couplings[0]), obj(query(lc, FIND_MUTUAL_COUPLING, l13, l12)));
        assertEquals(0L, query(lc, FIND_MUTUAL_COUPLING, l13, l23));
        long removal = PropertyDispatcher.invoke(lc, REMOVE_MUTUAL_COUPLING);
        PropertyDispatcher.setRelated(removal, MUTUAL_COUPLING, couplings[0]);
        assertSame(obj(lc), obj(PropertyDispatcher.invoke(removal, ACT_ADD)));
        assertEquals(0L, query(lc, REMOVE_MUTUAL_COUPLING, l12, l13)); // already removed
        assertSame(obj(lc), obj(query(lc, REMOVE_MUTUAL_COUPLING, l12, l23)));
        assertEquals(0, PropertyDispatcher.getChildren(lc, MUTUAL_COUPLING).length);

        // An existing one added back (twice: rejected); a new one with its segments
        PropertyDispatcher.setRelated(lc, ADD_MUTUAL_COUPLING, couplings[0]);
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.setRelated(lc, ADD_MUTUAL_COUPLING, couplings[0]))));
        long adder = PropertyDispatcher.invoke(lc, MUTUAL_COUPLING);
        PropertyDispatcher.setRelated(adder, EXT_MC_LINE1, l13);
        PropertyDispatcher.setRelated(adder, EXT_MC_LINE2, l23);
        PropertyDispatcher.setDouble(adder, EXT_MC_R, 0.3);
        PropertyDispatcher.setDouble(adder, EXT_MC_X, 0.9);
        long segment1 = PropertyDispatcher.invoke(adder, EXT_MC_LINE1_SEGMENT);
        PropertyDispatcher.setDouble(segment1, EXT_MC_SEGMENT_START, 0);
        PropertyDispatcher.setDouble(segment1, EXT_MC_SEGMENT_END, 0.4);
        assertEquals(0L, PropertyDispatcher.invoke(segment1, ACT_ADD));
        MutualCoupling added = (MutualCoupling) obj(PropertyDispatcher.invoke(adder, ACT_ADD));
        assertEquals(new LineSegment(0, 0.4), added.getLine1Segment());
        assertEquals(LineSegment.FULL_LINE, added.getLine2Segment());
        assertEquals(0.9, added.getX());
        assertEquals(2, PropertyDispatcher.getChildren(lc, MUTUAL_COUPLING).length);

        // The same lines again: rejected
        long duplicate = PropertyDispatcher.invoke(lc, MUTUAL_COUPLING);
        PropertyDispatcher.setRelated(duplicate, EXT_MC_LINE1, l23);
        PropertyDispatcher.setRelated(duplicate, EXT_MC_LINE2, l13);
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(duplicate, ACT_ADD))));

        // Removed: powsybl's LineCouplingsImpl.setExtendable(null) then fails (it subscribes to a null
        // network), the extension removed all the same. A new, empty one.
        assertThrows(NullPointerException.class, () -> network.removeExtension(LineCouplings.class));
        assertNull(network.getExtension(LineCouplings.class));
        long empty = PropertyDispatcher.invoke(PropertyDispatcher.invoke(reg(network), LINE_COUPLINGS), ACT_ADD);
        assertEquals(0, PropertyDispatcher.getChildren(empty, MUTUAL_COUPLING).length);
    }

    private static long query(long lineCouplings, int code, long line1, long line2) {
        long query = PropertyDispatcher.invoke(lineCouplings, code);
        PropertyDispatcher.setRelated(query, EXT_MC_LINE1, line1);
        PropertyDispatcher.setRelated(query, EXT_MC_LINE2, line2);
        return PropertyDispatcher.invoke(query, ACT_ADD);
    }
}
