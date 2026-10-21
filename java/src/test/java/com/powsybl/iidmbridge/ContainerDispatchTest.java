package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Networks, substations and voltage levels through {@link PropertyDispatcher}: counts, validation
 * levels, countries, geographical tags, the network of an identifiable, bus views, topology.
 */
class ContainerDispatchTest {

    private Network network;
    private Substation substation;
    private VoltageLevel vl1;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("containers", "test");
        substation = network.newSubstation().setId("S").setCountry(Country.FR).setTso("RTE").add();
        substation.addGeographicalTag("north");
        vl1 = substation.newVoltageLevel().setId("VL1").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl1.getBusBreakerView().newBus().setId("B1").add();
        vl1.getBusBreakerView().newBus().setId("B1b").add();
        vl1.getBusBreakerView().newSwitch().setId("SW").setBus1("B1").setBus2("B1b").setOpen(false).add();
        VoltageLevel vl2 = substation.newVoltageLevel().setId("VL2").setNominalV(225).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl2.getBusBreakerView().newBus().setId("B2").add();
        network.newSubstation().setId("S2").setCountry(Country.BE).add();
        vl1.newLoad().setId("L").setBus("B1").setP0(10).setQ0(1).add();
        network.newLine().setId("LINE").setVoltageLevel1("VL1").setBus1("B1").setVoltageLevel2("VL2").setBus2("B2")
            .setR(1).setX(10).setG1(0).setB1(0).setG2(0).setB2(0).add();
        substation.newTwoWindingsTransformer().setId("T2").setVoltageLevel1("VL1").setBus1("B1b")
            .setVoltageLevel2("VL2").setBus2("B2").setR(1).setX(10).setG(0).setB(0).setRatedU1(400).setRatedU2(225).add();
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

    private static int count(Object container, int type) {
        return (int) PropertyDispatcher.evaluate(reg(container), CHILD_COUNT, new double[] {type});
    }

    @Test void counts() {
        assertEquals(2, count(network, SUBSTATION));
        assertEquals(2, count(network, VOLTAGE_LEVEL));
        assertEquals(1, count(network, LINE));
        assertEquals(1, count(network, SWITCH));
        assertEquals(3, count(network, BUS));
        assertEquals(1, count(vl1, LINE));
        assertEquals(1, count(vl1, TWO_WINDINGS_TRANSFORMER));
        assertEquals(3, count(vl1, CONNECTABLE));
        assertEquals(1, count(substation, TWO_WINDINGS_TRANSFORMER));
        assertEquals(0, count(substation, THREE_WINDINGS_TRANSFORMER));
        // Not a type of object a substation holds: what PropertyNotFoundException comes from
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, BridgeErrors.kindOf(assertThrows(Throwable.class, () -> count(substation, GENERATOR))));
    }

    @Test void networkValues() {
        long n = reg(network);
        assertEquals(ContainerType.NETWORK.ordinal(), PropertyDispatcher.getInt(n, CONTAINER_TYPE));
        assertEquals(ContainerType.VOLTAGE_LEVEL.ordinal(), PropertyDispatcher.getInt(reg(vl1), CONTAINER_TYPE));
        assertEquals(ValidationLevel.STEADY_STATE_HYPOTHESIS.ordinal(), PropertyDispatcher.getInt(n, NETWORK_VALIDATION_LEVEL));
        assertEquals(ValidationLevel.STEADY_STATE_HYPOTHESIS.ordinal(), PropertyDispatcher.evaluate(n, NETWORK_VALIDATION_LEVEL, new double[] {1}));
        PropertyDispatcher.setInt(n, NETWORK_MIN_VALIDATION_LEVEL, ValidationLevel.EQUIPMENT.ordinal());
        network.getLoad("L").setP0(Double.NaN);
        assertEquals(ValidationLevel.EQUIPMENT.ordinal(), PropertyDispatcher.evaluate(n, NETWORK_VALIDATION_LEVEL, new double[] {0}));
        assertArrayEquals(new String[] {"BE", "FR"}, PropertyDispatcher.getStringList(n, NETWORK_COUNTRIES));

        // The network of its objects; ids can change
        assertSame(network, obj(PropertyDispatcher.getRelated(reg(vl1), NETWORK)));
        assertSame(network, obj(PropertyDispatcher.getRelated(reg(vl1), REL_PARENT_NETWORK)));
        assertEquals(1, PropertyDispatcher.getBool(n, NETWORK));
        PropertyDispatcher.setString(reg(network.getLoad("L")), ID, "L_RENAMED");
        assertNotNull(network.getLoad("L_RENAMED"));
    }

    @Test void substations() {
        long s = reg(substation);
        assertArrayEquals(new String[] {"north"}, PropertyDispatcher.getStringList(s, SUBSTATION_GEOGRAPHICAL_TAGS));
        PropertyDispatcher.setString(s, ADD_GEOGRAPHICAL_TAG, "coast");
        assertArrayEquals(new String[] {"coast", "north"}, PropertyDispatcher.getStringList(s, SUBSTATION_GEOGRAPHICAL_TAGS));
        PropertyDispatcher.setString(s, COUNTRY, "DE");
        assertEquals(Country.DE, substation.getCountry().orElseThrow());
    }

    @Test void voltageLevels() {
        long vl = reg(vl1);
        PropertyDispatcher.setDouble(vl, VL_NOMINAL_V, 380);
        PropertyDispatcher.setDouble(vl, VL_LOW_VOLTAGE_LIMIT, 360);
        PropertyDispatcher.setDouble(vl, VL_HIGH_VOLTAGE_LIMIT, 420);
        assertEquals(380, vl1.getNominalV());
        assertEquals(420, vl1.getHighVoltageLimit());
        assertSame(network.getLoad("L"), obj(PropertyDispatcher.getRelatedByString(vl, CONNECTABLE, "L")));
        assertEquals(0L, PropertyDispatcher.getRelatedByString(vl, CONNECTABLE, "UNKNOWN"));
        assertSame(vl1, obj(PropertyDispatcher.getRelated(reg(network.getSwitch("SW")), REL_VOLTAGE_LEVEL)));
        assertTrue(PropertyDispatcher.getString(vl, VL_TOPOLOGY_TEXT).contains("Topology of VL1"));

        // Bus view: B1 and B1b merged (the switch is closed)
        long[] buses = PropertyDispatcher.getChildren(vl, REL_BV_BUS);
        assertEquals(1, buses.length);
        String busId = PropertyDispatcher.getString(buses[0], ID);
        assertEquals(1, PropertyDispatcher.isSameObject(buses[0], PropertyDispatcher.getRelatedByString(vl, REL_BV_BUS, busId)));
        assertEquals(1, PropertyDispatcher.isSameObject(buses[0], PropertyDispatcher.getRelatedByString(reg(network), REL_BV_BUS, busId)));
        assertEquals(2, PropertyDispatcher.getChildren(reg(network), REL_BV_BUS).length);
        assertSame(network.getBusBreakerView().getBus("B2"), obj(PropertyDispatcher.getRelatedByString(reg(network), REL_BBV_BUS, "B2")));

        // Converted to bus-breaker topology (not the other way)
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.setInt(vl, VL_TOPOLOGY_KIND, TopologyKind.NODE_BREAKER.ordinal()))));
        VoltageLevel vl3 = substation.newVoltageLevel().setId("VL3").setNominalV(63).setTopologyKind(TopologyKind.NODE_BREAKER).add();
        vl3.getNodeBreakerView().newBusbarSection().setId("BBS").setNode(0).add();
        vl3.getNodeBreakerView().newBreaker().setId("BR").setNode1(0).setNode2(1).setOpen(false).add();
        vl3.newLoad().setId("L3").setNode(1).setP0(1).setQ0(0).add();
        long merged = PropertyDispatcher.getRelatedByString(reg(vl3), REL_MERGED_BUS, "BBS");
        assertSame(vl3.getBusView().getMergedBus("BBS"), obj(merged));
        PropertyDispatcher.setInt(reg(vl3), VL_TOPOLOGY_KIND, TopologyKind.BUS_BREAKER.ordinal());
        assertEquals(TopologyKind.BUS_BREAKER, vl3.getTopologyKind());
    }
}
