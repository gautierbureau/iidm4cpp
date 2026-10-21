package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.graalvm.NetworkRegistry;
import org.junit.jupiter.api.*;

import java.util.Arrays;
import java.util.List;

import static com.powsybl.iidmbridge.PropertyCodes.*;
import static org.junit.jupiter.api.Assertions.*;

/**
 * The connected and synchronous components of the bus view, and the variant manager, through
 * {@link PropertyDispatcher}: components of the network and of a bus, their buses; variants cloned,
 * switched to, overwritten and removed.
 */
class ComponentAndVariantDispatchTest {

    private Network network;
    private Load load;

    /** B1 - L12 - B2, the main component; B3 with a load, another; B4, with nothing, none. */
    @BeforeEach
    void setUp() {
        NetworkRegistry.releaseAll();
        network = Network.create("components", "test");
        Substation s = network.newSubstation().setId("S").add();
        VoltageLevel vl1 = newVoltageLevel(s, "VL1");
        vl1.getBusBreakerView().newBus().setId("B1").add();
        VoltageLevel vl2 = newVoltageLevel(s, "VL2");
        vl2.getBusBreakerView().newBus().setId("B2").add();
        VoltageLevel vl3 = newVoltageLevel(s, "VL3");
        vl3.getBusBreakerView().newBus().setId("B3").add();
        vl3.getBusBreakerView().newBus().setId("B4").add();
        network.newLine().setId("L12").setVoltageLevel1("VL1").setBus1("B1").setVoltageLevel2("VL2").setBus2("B2")
            .setR(1).setX(10).setG1(0).setB1(0).setG2(0).setB2(0).add();
        load = vl3.newLoad().setId("LD3").setBus("B3").setP0(10).setQ0(1).add();
    }

    private static VoltageLevel newVoltageLevel(Substation s, String id) {
        return s.newVoltageLevel().setId(id).setNominalV(400).setTopologyKind(TopologyKind.BUS_BREAKER).add();
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

    private static void cloneVariant(long manager, String sourceId, boolean mayOverwrite, String... targetIds) {
        long clone = PropertyDispatcher.invoke(manager, VARIANT_CLONE);
        PropertyDispatcher.setString(clone, VARIANT_SOURCE_ID, sourceId);
        for (String targetId : targetIds) {
            PropertyDispatcher.setString(clone, VARIANT_TARGET_ID, targetId);
        }
        PropertyDispatcher.setBool(clone, VARIANT_MAY_OVERWRITE, mayOverwrite ? 1 : 0);
        assertEquals(0L, PropertyDispatcher.invoke(clone, ACT_ADD));
    }

    @Test void components() {
        long n = reg(network);
        long[] components = PropertyDispatcher.getChildren(n, REL_CONNECTED_COMPONENT);
        assertEquals(2, components.length);
        long main = components[0];
        assertEquals(1, PropertyDispatcher.getBool(main, COMPONENT));
        assertEquals(0, PropertyDispatcher.getInt(main, COMPONENT_NUM));
        assertEquals(2, PropertyDispatcher.getInt(main, COMPONENT_SIZE));
        assertEquals(List.of("VL1_0", "VL2_0"), ids(PropertyDispatcher.getChildren(main, BUS)));
        assertEquals(1, PropertyDispatcher.getInt(components[1], COMPONENT_NUM));
        assertEquals(List.of("VL3_0"), ids(PropertyDispatcher.getChildren(components[1], BUS)));
        assertEquals(2, PropertyDispatcher.getChildren(n, REL_SYNCHRONOUS_COMPONENT).length);

        // The one of a bus: of the bus view, of the bus-breaker view; none with nothing connected
        Bus b3 = network.getBusView().getBus("VL3_0");
        assertSame(obj(components[1]), obj(PropertyDispatcher.getRelated(reg(b3), REL_CONNECTED_COMPONENT)));
        assertSame(b3.getSynchronousComponent(), obj(PropertyDispatcher.getRelated(reg(b3), REL_SYNCHRONOUS_COMPONENT)));
        long b1 = reg(network.getBusBreakerView().getBus("B1"));
        assertSame(obj(main), obj(PropertyDispatcher.getRelated(b1, REL_CONNECTED_COMPONENT)));
        assertEquals(0L, PropertyDispatcher.getRelated(reg(network.getBusBreakerView().getBus("B4")), REL_CONNECTED_COMPONENT));
    }

    @Test void variants() {
        long manager = PropertyDispatcher.getRelated(reg(network), VARIANT_MANAGER);
        assertEquals(1, PropertyDispatcher.getBool(manager, VARIANT_MANAGER));
        assertArrayEquals(new String[] {"InitialState"}, PropertyDispatcher.getStringList(manager, VARIANT_IDS));
        assertEquals("InitialState", PropertyDispatcher.getString(manager, WORKING_VARIANT_ID));

        // Cloned into two, listed sorted
        cloneVariant(manager, "InitialState", false, "V2", "V1");
        assertArrayEquals(new String[] {"InitialState", "V1", "V2"}, PropertyDispatcher.getStringList(manager, VARIANT_IDS));

        // Changed in one, not in the others
        PropertyDispatcher.setString(manager, WORKING_VARIANT_ID, "V1");
        assertEquals("V1", network.getVariantManager().getWorkingVariantId());
        load.setP0(99);
        PropertyDispatcher.setString(manager, WORKING_VARIANT_ID, "InitialState");
        assertEquals(10, load.getP0());

        // Overwritten only when allowed
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> cloneVariant(manager, "InitialState", false, "V1"))));
        cloneVariant(manager, "InitialState", true, "V1");
        PropertyDispatcher.setString(manager, WORKING_VARIANT_ID, "V1");
        assertEquals(10, load.getP0());
        assertEquals(BridgeErrors.VALIDATION, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.setString(manager, WORKING_VARIANT_ID, "NONE"))));

        PropertyDispatcher.setString(manager, REMOVE_VARIANT, "V2");
        assertArrayEquals(new String[] {"InitialState", "V1"}, PropertyDispatcher.getStringList(manager, VARIANT_IDS));

        // Multi-thread access: this thread keeps its working variant
        PropertyDispatcher.setBool(manager, VARIANT_MULTI_THREAD_ACCESS, 1);
        assertEquals(1, PropertyDispatcher.getBool(manager, VARIANT_MULTI_THREAD_ACCESS));
        assertEquals("V1", PropertyDispatcher.getString(manager, WORKING_VARIANT_ID));
        PropertyDispatcher.setBool(manager, VARIANT_MULTI_THREAD_ACCESS, 0);
        assertFalse(network.getVariantManager().isVariantMultiThreadAccessAllowed());
    }

    /** A call made in steps takes its own fields only; a value of another type fails before the call. */
    @Test void callFields() {
        long manager = PropertyDispatcher.getRelated(reg(network), VARIANT_MANAGER);
        long clone = PropertyDispatcher.invoke(manager, VARIANT_CLONE);
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.setDouble(clone, LOAD_P0, 1))));
        PropertyDispatcher.setString(clone, VARIANT_SOURCE_ID, "InitialState");
        PropertyDispatcher.setString(clone, VARIANT_TARGET_ID, "V1");
        PropertyDispatcher.setInt(clone, VARIANT_TARGET_ID, 2);
        assertEquals(BridgeErrors.PROPERTY_NOT_FOUND, BridgeErrors.kindOf(assertThrows(Throwable.class,
            () -> PropertyDispatcher.invoke(clone, ACT_ADD))));
        // Not even V1
        assertArrayEquals(new String[] {"InitialState"}, PropertyDispatcher.getStringList(manager, VARIANT_IDS));
    }
}
