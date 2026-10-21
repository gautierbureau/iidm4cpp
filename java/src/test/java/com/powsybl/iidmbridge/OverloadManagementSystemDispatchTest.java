package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Overload management systems through {@link PropertyDispatcher}: their values and trippings,
 * creation through the adder with nested tripping adders, removal.
 */
class OverloadManagementSystemDispatchTest {

    private Network network;
    private Substation substation;

    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("automation", "test");
        substation = network.newSubstation().setId("S").add();
        VoltageLevel vl1 = substation.newVoltageLevel().setId("VL1").setNominalV(400).setTopologyKind(TopologyKind.NODE_BREAKER).add();
        vl1.getNodeBreakerView().newBusbarSection().setId("BBS").setNode(0).add();
        vl1.getNodeBreakerView().newBreaker().setId("BR").setNode1(0).setNode2(1).setOpen(false).add();
        VoltageLevel vl2 = substation.newVoltageLevel().setId("VL2").setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
        vl2.getBusBreakerView().newBus().setId("B2").add();
        network.newLine().setId("L12").setVoltageLevel1("VL1").setNode1(1).setVoltageLevel2("VL2").setBus2("B2")
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

    /** OMS: L12 side ONE monitored; trippings: the breaker BR, then side TWO of the line itself. */
    private long newSystem() {
        long adder = PropertyDispatcher.invoke(reg(substation), OVERLOAD_MANAGEMENT_SYSTEM);
        PropertyDispatcher.setString(adder, ID, "OMS");
        PropertyDispatcher.setBool(adder, AUTOMATION_SYSTEM_ENABLED, 1);
        PropertyDispatcher.setString(adder, OMS_MONITORED_ELEMENT_ID, "L12");
        PropertyDispatcher.setInt(adder, OMS_MONITORED_SIDE, ThreeSides.ONE.ordinal());
        long switchTripping = PropertyDispatcher.invoke(adder, SWITCH_TRIPPING);
        PropertyDispatcher.setString(switchTripping, TRIPPING_KEY, "SW");
        PropertyDispatcher.setString(switchTripping, TRIPPING_NAME, "Open breaker");
        PropertyDispatcher.setDouble(switchTripping, TRIPPING_CURRENT_LIMIT, 1000);
        PropertyDispatcher.setBool(switchTripping, TRIPPING_OPEN_ACTION, 1);
        PropertyDispatcher.setString(switchTripping, TRIPPING_ELEMENT_ID, "BR");
        PropertyDispatcher.setKeyedString(switchTripping, PROPERTY, "origin", "test");
        assertEquals(0L, PropertyDispatcher.invoke(switchTripping, ACT_ADD)); // nested
        long branchTripping = PropertyDispatcher.invoke(adder, BRANCH_TRIPPING);
        PropertyDispatcher.setString(branchTripping, TRIPPING_KEY, "BRANCH");
        PropertyDispatcher.setDouble(branchTripping, TRIPPING_CURRENT_LIMIT, 1200);
        PropertyDispatcher.setBool(branchTripping, TRIPPING_OPEN_ACTION, 1);
        PropertyDispatcher.setString(branchTripping, TRIPPING_ELEMENT_ID, "L12");
        PropertyDispatcher.setInt(branchTripping, TRIPPING_SIDE, TwoSides.TWO.ordinal());
        PropertyDispatcher.invoke(branchTripping, ACT_ADD);
        return PropertyDispatcher.invoke(adder, ACT_ADD);
    }

    @Test void systems() {
        long system = newSystem();
        assertEquals(1, PropertyDispatcher.getBool(system, OVERLOAD_MANAGEMENT_SYSTEM));
        assertEquals(1, PropertyDispatcher.getBool(system, AUTOMATION_SYSTEM_ENABLED));
        assertEquals("L12", PropertyDispatcher.getString(system, OMS_MONITORED_ELEMENT_ID));
        assertEquals(ThreeSides.ONE.ordinal(), PropertyDispatcher.getInt(system, OMS_MONITORED_SIDE));
        assertSame(substation, obj(PropertyDispatcher.getRelated(system, REL_SUBSTATION)));
        assertEquals(1, PropertyDispatcher.getChildren(reg(substation), OVERLOAD_MANAGEMENT_SYSTEM).length);
        assertEquals(1, PropertyDispatcher.getChildren(reg(network), OVERLOAD_MANAGEMENT_SYSTEM).length);
        assertSame(obj(system), obj(PropertyDispatcher.findById(reg(network), OVERLOAD_MANAGEMENT_SYSTEM, "OMS")));
        PropertyDispatcher.setBool(system, AUTOMATION_SYSTEM_ENABLED, 0);
        assertFalse(((OverloadManagementSystem) obj(system)).isEnabled());

        // Its trippings, in order
        long[] trippings = PropertyDispatcher.getChildren(system, TRIPPING);
        assertEquals(2, trippings.length);
        long sw = trippings[0];
        assertEquals(1, PropertyDispatcher.getBool(sw, SWITCH_TRIPPING));
        assertEquals(OverloadManagementSystem.Tripping.Type.SWITCH_TRIPPING.ordinal(), PropertyDispatcher.getInt(sw, TRIPPING_TYPE));
        assertEquals("SW", PropertyDispatcher.getString(sw, TRIPPING_KEY));
        assertEquals("Open breaker", PropertyDispatcher.getString(sw, TRIPPING_NAME));
        assertEquals(1000, PropertyDispatcher.getDouble(sw, TRIPPING_CURRENT_LIMIT));
        assertEquals(1, PropertyDispatcher.getBool(sw, TRIPPING_OPEN_ACTION));
        assertEquals("BR", PropertyDispatcher.getString(sw, TRIPPING_ELEMENT_ID));
        assertEquals("test", PropertyDispatcher.getKeyedString(sw, PROPERTY, "origin"));
        long branch = trippings[1];
        assertEquals(1, PropertyDispatcher.getBool(branch, BRANCH_TRIPPING));
        assertEquals("BRANCH", PropertyDispatcher.getString(branch, TRIPPING_NAME)); // no name: the key
        assertEquals(TwoSides.TWO.ordinal(), PropertyDispatcher.getInt(branch, TRIPPING_SIDE));
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND,
            BridgeErrors.kindOf(assertThrows(Throwable.class, () -> PropertyDispatcher.getInt(sw, TRIPPING_SIDE))));

        // Changes
        PropertyDispatcher.setString(branch, TRIPPING_NAME, "Trip the line");
        PropertyDispatcher.setInt(branch, TRIPPING_SIDE, TwoSides.ONE.ordinal());
        PropertyDispatcher.setDouble(sw, TRIPPING_CURRENT_LIMIT, 900);
        PropertyDispatcher.setBool(sw, TRIPPING_OPEN_ACTION, 0);
        OverloadManagementSystem oms = (OverloadManagementSystem) obj(system);
        OverloadManagementSystem.BranchTripping branchTripping = (OverloadManagementSystem.BranchTripping) oms.getTrippings().get(1);
        assertEquals("Trip the line", branchTripping.getNameOrKey());
        assertEquals(TwoSides.ONE, branchTripping.getSideToOperate());
        assertFalse(oms.getTrippings().get(0).isOpenAction());

        PropertyDispatcher.invoke(system, ACT_REMOVE);
        assertNull(network.getOverloadManagementSystem("OMS"));
    }

    @Test void addTripping() {
        long system = newSystem();
        OverloadManagementSystem.Tripping tripping = ((OverloadManagementSystem) obj(system)).getTrippings().get(0);
        PropertyDispatcher.setRelated(system, ADD_TRIPPING, reg(tripping));
        assertEquals(3, PropertyDispatcher.getChildren(system, TRIPPING).length);
    }
}
